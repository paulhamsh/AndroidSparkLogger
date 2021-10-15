# AndroidSparkLogger
Logs interactions between App on Android and Spark amp

Ensure serial monitor is on!

Works with M5Stack Core and Heltec Wifi - change the #define to select the board

Output like this:

```
Found Spark - trying to connect....
Spark connected
Available for app to connect...

Write to spark:     01 FE 00 00 53 FE 17 00 00 00 00 00 00 00 00 00 
                    F0 01 01 00 02_2F F7_
Write to app:       01 FE 00 00 41 FF 1D 00 00 00 00 00 00 00 00 00 
                    F0 01 01 77 03_2F 11_4E 01 04 03 2E F7 
Write to spark:     01 FE 00 00 53 FE 17 00 00 00 00 00 00 00 00 00 
                    F0 01 02 00 02_2F F7_
Write to app:       01 FE 00 00 41 FF 1D 00 00 00 00 00 00 00 00 00 
                    F0 01 02 77 03_2F 11_4E 01 04 03 2E F7 
Write to spark:     01 FE 00 00 53 FE 17 00 00 00 00 00 00 00 00 00 
                    F0 01 03 00 02_11 F7_
Write to app:       01 FE 00 00 41 FF 23 00 00 00 00 00 00 00 00 00 
                    F0 01 03 5D 03_11 02_08 28 53 70 61 72 6B 00 20 
                    34 30 F7 
Write to spark:     01 FE 00 00 53 FE 17 00 00 00 00 00 00 00 00 00 
                    F0 01 04 00 02_23 F7_
Write to app:       01 FE 00 00 41 FF 29 00 00 00 00 00 00 00 00 00 
                    F0 01 04 32 03_23 02_0D 2D 53 30 34 30 43 00 32 
                    30 35 41 36 34 36 01 77 F7 

```

