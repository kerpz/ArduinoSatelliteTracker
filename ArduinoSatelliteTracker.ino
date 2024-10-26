/*
 Author:
 - Philip Bordado (kerpz@yahoo.com)

 Hardware:
 - Wemos D1 mini (Compatible board)
 - MPU9250 / MPU6050 (not used)
 - L298 Dual H Bridge (for motor control)

 Software:
 - Arduino 1.8.19 (Stable)
 - Board 3.0.2
 - Adafruit SSD1306 2.5.3
 - SparkFun SGP4 1.0.3 <Hopperpop>
 - NTPClient 3.2.1 <Fabrice Weinberg>
 - ArduinoJson 6.21.5 <Benoit Blanchon>
 - Websockets 2.4.0 <Markus Sattler>
*/

#include "eeprom.h"
#include "network.h"
#include "webserver.h"
// #include "websocket.h"
#include "app.h"

void setup()
{
  delay(10);

  Serial.begin(115200);

  Serial.println();
  Serial.println(APPNAME);

  loadConfig();

  networkSetup();
  webserverSetup();

  appSetup();

  delay(100);
}

void loop()
{
  networkLoop();
  webserverLoop();

  appLoop();
}
