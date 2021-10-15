// Define your board name below for conditional compilation- currently supports M5Stack Core 2 and Heltec WIFI

#define M5_BOARD
//#define HELTEC_BOARD


#ifdef M5_BOARD
#include <M5Core2.h>
#else
#include "Heltec.h"
#endif
 
#include "BLEDevice.h"
#include "BluetoothSerial.h"
#include "RingBuffer.h"

#define C_SERVICE "ffc0"
#define C_CHAR1   "ffc1"
#define C_CHAR2   "ffc2"

BluetoothSerial BTApp;
 
BLEScan *pScan;
BLEScanResults pResults;
BLEAdvertisedDevice device;

BLEClient *pClient_sp;
BLERemoteService *pService_sp;
BLERemoteCharacteristic *pReceiver_sp;
BLERemoteCharacteristic *pSender_sp;

bool sp_triggered;
bool connected_sp;
int i;
int spos, apos;
byte sbuf[5000];
byte abuf[5000];

RingBuffer app;

int scrpos;

void printval(int a)
{
  if (a < 16) {
    Serial.print("0");
  }
  Serial.print(a, HEX);
  if (scrpos == 19)
    Serial.print(" < ");
  else if (scrpos == 21) 
    Serial.print(" > ");
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

void notifyCB_sp(BLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  int i;
  byte b;

  for (i = 0; i < length; i++) {
    b = pData[i];
    app.add(b);
  }
  app.commit();
}


void setup() {
  Serial.println("Started");
  
#ifdef M5_BOARD
  M5.begin();
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(4);
  M5.Lcd.println("   Logger");
  M5.Lcd.println("-------------");
  M5.Lcd.setTextSize(3);
#else
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "   Logger");
  Heltec.display->display();
#endif

  spos = 0;
  apos = 0;
  sp_triggered = false;
  scrpos = 0;

  // Create server to act as Spark
  BLEDevice::init("Spark 40 BLE");
  
  pClient_sp = BLEDevice::createClient();
  pScan      = BLEDevice::getScan();

  // Connect to Spark
  connected_sp = false;
  
  while (!connected_sp) {
    pResults = pScan->start(4);
    BLEUUID SpServiceUuid(C_SERVICE);

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
          pReceiver_sp->registerForNotify(notifyCB_sp);
        }
      }
    }
  }

  // This is what I was missing - obvious?
  
  const uint8_t notifyOn[] = {0x1, 0x0};
  BLERemoteDescriptor* p2902 = pReceiver_sp->getDescriptor(BLEUUID((uint16_t)0x2902));
  if(p2902 != nullptr)
  {
    p2902->writeValue((uint8_t*)notifyOn, 2, true);
  }
  
  if (!BTApp.begin ("Spark 40 Audio")) {
    Serial.println("Spark bluetooth init fail");
    while (true);
  }   

  Serial.println("Available for app to connect...");
}


void loop() {
  uint8_t b;
  int i;

#ifdef M5_BOARD
  M5.update();
#endif
  

  while (!app.is_empty()) {
    apos = 0;
    while (!app.is_empty() && apos < 0x6a) {
      app.get(&b);
      abuf[apos++] = b;
    }
    BTApp.write(abuf, apos);

    printhdr("Write to app:       ");
    for (i=0; i < apos; i++) {
      printval(abuf[i]);
    }
    apos = 0;
  }

  if (BTApp.available()) {
    b = BTApp.read();
    sbuf[spos++] = b;
    if (b == 0xf7) {
      pSender_sp->writeValue(sbuf, spos);
      
      printhdr("Write to spark:     ");
      for (i=0; i<spos; i++) {
        printval(sbuf[i]);
      }
      spos = 0 ;
    }
  }

}
