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
#include "ntp.h"
#include "display.h"
#include "tracker.h"
#include "webserver.h"
// #include "websocket.h"
#include "app.h"

void execEvery(int ms)
{
  static unsigned long msTick = millis();
  static uint8_t sTick;

  if (millis() - msTick >= ms)
  { // run every N ms
    msTick = millis();

    ntpLoop();
    // Serial.println(epochTime);

    if (display_enable)
      displayLoop();
    if (mpu9250_enable)
      mpu9250Loop();
    // if (motor_enable) motorLoop();
    if (tracker_enable)
      trackerLoop();

    if (sTick >= 59)
    {
      sTick = 0;
      run_time++;
    }
    else
    {
      sTick++;
    }
  }
}

void setup()
{
  delay(10);

  Serial.begin(115200);

  Serial.println();
  Serial.println(APPNAME);

  loadConfig();

  networkSetup();
  ntpSetup();
  webserverSetup();
  // websocketSetup();

  // if (beep_enable)
  //   displaySetup();
  // if (analog_enable)
  //   analogSetup();
  if (display_enable)
    displaySetup();
  // if (ads1115_enable) ads1115Setup();
  if (mpu9250_enable)
    mpu9250Setup();
  if (tracker_enable)
    trackerSetup();

  appSetup();

  delay(100);
}

void loop()
{
  execEvery(1000);

  // if (analog_enable) analogLoop();
  // if (ads1115_enable) ads1115Loop();
  // if (mpu9250_enable) mpu9250Loop();
  // if (motor_enable) motorLoop();

  networkLoop();
  webserverLoop();
  // websocketLoop();

  appLoop();
}
