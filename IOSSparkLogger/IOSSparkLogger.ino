#include <M5Core2.h> 
#include "NimBLEDevice.h"

#define C_SERVICE "ffc0"
#define C_CHAR1   "ffc1"
#define C_CHAR2   "ffc2"

#define S_SERVICE "ffc0"
#define S_CHAR1   "ffc1"
#define S_CHAR2   "ffc2"


NimBLEServer *pServer;
NimBLEService *pService;
NimBLECharacteristic *pCharacteristic_receive;
NimBLECharacteristic *pCharacteristic_send;

NimBLEAdvertising *pAdvertising;
  
NimBLEScan *pScan;
NimBLEScanResults pResults;
NimBLEAdvertisedDevice device;

NimBLEClient *pClient_sp;
NimBLERemoteService *pService_sp;
NimBLERemoteCharacteristic *pReceiver_sp;
NimBLERemoteCharacteristic *pSender_sp;


int scrpos;

void printval(int a)
{
  if (a < 16) {
    Serial.print("0");
  }
  Serial.print(a, HEX);
  if (scrpos == 19) {
    Serial.print(" << ");
    scrpos++;
  }
  else if (scrpos == 22) {
    Serial.print(" >> ");
    scrpos++;
  }
  else
    Serial.print(" ");

  if (scrpos % 16 == 15) {
    Serial.println();
    Serial.print("                    ");
  }
  scrpos++;
}

void printhdr(char *s)
{
  Serial.println();
  Serial.print(s);
  scrpos = 0;
}




bool triggered_to_amp, triggered_to_app;
int  pass_size_amp,    pass_size_app;
byte pass_amp[1000],   pass_app[1000];

void notifyCB_sp(BLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  int i;
  byte b;

  for (i = 0; i < length; i++) {
    b = pData[i];
    pass_app[i] = b;
  }
  pass_size_app = i;
  triggered_to_app = true;

}




class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic){

    };

    void onWrite(NimBLECharacteristic* pCharacteristic) {
        int j, l;
        const char *p;
        byte b;
        l = pCharacteristic->getValue().length();
        p = pCharacteristic->getValue().c_str();
        for (j=0; j < l; j++) {
          b = p[j];
          pass_amp[j] = b;
        }
        pass_size_amp = j;
        triggered_to_amp = true;
    };
   

    void onNotify(NimBLECharacteristic* pCharacteristic) {
    };



    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
    };

    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
        String str = "Client ID: ";
        str += desc->conn_handle;
        str += " Address: ";
        str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
        if(subValue == 0) {
            str += " Unsubscribed to ";
        }else if(subValue == 1) {
            str += " Subscribed to notfications for ";
        } else if(subValue == 2) {
            str += " Subscribed to indications for ";
        } else if(subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID()).c_str();

        if (subValue == 1 && strcmp(std::string(pCharacteristic->getUUID()).c_str(), "0xffc2") == 0) {
          Serial.println("App active");
        }
    };
};



static CharacteristicCallbacks chrCallbacks_s, chrCallbacks_r;

bool connected_sp;
int i;

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(4);
  M5.Lcd.println("   Logger");
  M5.Lcd.println("-------------");
  Serial.println("Started");
  M5.Lcd.setTextSize(3);

  triggered_to_app = false;
  triggered_to_amp = false;

  scrpos = 0;

  // Create server to act as Spark
  NimBLEDevice::init("Spark 40 BLE");
  pClient_sp = NimBLEDevice::createClient();
  pScan      = NimBLEDevice::getScan();
    
  // Set up server
  pServer = NimBLEDevice::createServer();
  pService = pServer->createService(S_SERVICE);
  pCharacteristic_receive = pService->createCharacteristic(S_CHAR1, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  pCharacteristic_send = pService->createCharacteristic(S_CHAR2, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);  
  pCharacteristic_receive->setCallbacks(&chrCallbacks_r);
  pCharacteristic_send->setCallbacks(&chrCallbacks_s);

  pService->start();
  pServer->start();

  pAdvertising = NimBLEDevice::getAdvertising(); // create advertising instance
  pAdvertising->addServiceUUID(pService->getUUID()); // tell advertising the UUID of our service
  pAdvertising->setScanResponse(true);  

  Serial.println("Service set up");

  
  // Connect to Spark
  connected_sp = false;
  
  while (!connected_sp) {
    pResults = pScan->start(4);
    NimBLEUUID SpServiceUuid(C_SERVICE);


    Serial.println("------------------------------");
    for(i = 0; i < pResults.getCount()  && (!connected_sp); i++) {
      device = pResults.getDevice(i);

      if (device.isAdvertisingService(SpServiceUuid)) {
        Serial.println("Found Spark - trying to connect....");
        if(pClient_sp->connect(&device)) {
          connected_sp = true;
          Serial.println("Spark connected");
        }
      }

    }

    // Set up client
    if (connected_sp) {
      pService_sp = pClient_sp->getService(SpServiceUuid);
      if (pService_sp != nullptr) {
        pSender_sp   = pService_sp->getCharacteristic(C_CHAR1);
        pReceiver_sp = pService_sp->getCharacteristic(C_CHAR2);
        if (pReceiver_sp && pReceiver_sp->canNotify()) {
          if (!pReceiver_sp->subscribe(true, notifyCB_sp, true)) {
            connected_sp = false;
            Serial.println("Spark disconnected");
            //pClient_sp->disconnect();
            NimBLEDevice::deleteClient(pClient_sp);
          }
        }
      }
    }

  }

  Serial.println("Available for app to connect...");
  
  // start advertising
  pAdvertising->start(); 

}


void loop() {
  // put your main code here, to run repeatedly:
  M5.update();

  if (triggered_to_amp) {
    triggered_to_amp = false;
    if (connected_sp) {
      pSender_sp->writeValue(pass_amp, pass_size_amp, false);

      printhdr("Write to spark:     ");
      for (i=0; i<pass_size_amp; i++) {
        printval(pass_amp[i]);
      }
    }
  }

  if (triggered_to_app) {
    triggered_to_app = false;
    pCharacteristic_send->setValue(pass_app, pass_size_app);
    pCharacteristic_send->notify(true);

    printhdr("Write to app:       ");
    for (i=0; i<pass_size_app; i++) {
      printval(pass_app[i]);
    }
  }
}
