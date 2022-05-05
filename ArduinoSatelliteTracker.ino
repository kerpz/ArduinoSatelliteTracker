/*
 Author:
 - Philip Bordado (kerpz@yahoo.com)

 Hardware:
 - Wemos D1 mini (Compatible board)
 - MPU 9250
 
 Software:
 - Arduino 1.8.19 (Stable)
 - Board 3.0.2
 - Adafruit SSD1306 2.5.3
 - NTPClient 3.2.0
 - SparkFun SGP4 1.0.3
 - ArduinoJson 6.19.4
*/


#define APPNAME "SatelliteTracker v1.0"

// eeprom
#include <EEPROM.h>
// network
#include <ESP8266WiFi.h>
#include <DNSServer.h>
// webserver
#include <ESP8266WebServer.h>
#include "ArduinoJson.h"
// ntp
#include <NTPClient.h>
#include <WiFiUdp.h>
// tracker
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
// openssl s_client -connect api.thingspeak.com:443 | openssl x509 -fingerprint -noout
//const uint8_t fingerprint[20] = {0x27, 0x18, 0x92, 0xDD, 0xA4, 0x26, 0xC3, 0x07, 0x09, 0xB9, 0x7A, 0xE6, 0xC5, 0x21, 0xB9, 0x5B, 0x48, 0xF7, 0x16, 0xE1};
#include <Sgp4.h>

char ssid[32] = "";
char password[32] = "";

// mpu6050 part
float yaw;
float pitch;
float roll;
float temperature;

//float az_min = 0.0;
//float az_max = 360.0;
//float el_min = 0.0;
//float el_max = 180.0;

// motor part
uint8_t motor_mode;

//
uint8_t display_enable;
uint8_t beep_enable;
uint8_t mpu9250_enable;
uint8_t mpu9250_found = 0;
uint8_t ak8963_found = 0;
uint8_t motor_enable = 1;

//float m_bias_x = 0.0;
//float m_bias_y = 0.0;
//float m_bias_z = 0.0;

float latitude = 14.6112342;
float longitude = 121.1303641;
float altitude = 19.0;
float declination = -2.57;  // Cainta // dd = d + m/60 + s/3600

char sat_name[] = "ISS (ZARYA)";
char tle_line1[] = "1 25544U 98067A   22109.21444691  .00006496  00000+0  12115-3 0  9992";  //Line one from the TLE data
char tle_line2[] = "2 25544  51.6426 268.6615 0004851  39.2849 131.6326 15.50131630336035";  //Line two from the TLE data

float sat_latitude;
float sat_longitude;
float sat_altitude;
float sat_azimuth;
float sat_elevation;
float sat_distance;

uint16_t wifi_error = 0;
uint16_t mpu_error = 0;

uint16_t run_time = 0;

// timing
int timezone = 0;
unsigned long epoch;
uint8_t second = 0;
uint8_t minute;
uint8_t hour;
uint8_t day;
uint8_t month;
uint16_t year;


void execEvery(int ms) {
  static unsigned long msTick = millis();
  static uint8_t sTick;

  if (millis() - msTick >= ms) { // run every N ms
    msTick = millis();

    ntpLoop();
    //Serial.println(epochTime);
    trackerLoop();

    if (display_enable) displayLoop();
    //if (mpu6050_enable) mpu6050Loop();
    //if (motor_enable) motorLoop();
    
    if (sTick >= 59) {
      sTick = 0;
      run_time++;
    }
    else {
      sTick++;
    }
  }
}

void setup() {
  delay(10);

  Serial.begin(115200);

  Serial.println();
  Serial.println(APPNAME);

  loadConfig();

  networkSetup();
  ntpSetup();
  webserverSetup();
  trackerSetup();

  if (display_enable) displaySetup();
  if (mpu9250_enable) mpu9250Setup();
  if (motor_enable) motorSetup();

  delay(100);
}

void loop() {
  execEvery(1000);

  if (mpu9250_enable && mpu9250_found) mpu9250Loop();
  if (motor_enable) motorLoop();

  networkLoop();
  webserverLoop();
}
