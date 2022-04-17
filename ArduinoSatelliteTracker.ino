/*
 Author:
 - Philip Bordado (kerpz@yahoo.com)

 Hardware:
 - Wemos D1 mini (Compatible board)
 
 Software:
 - Arduino 1.8.19 (Stable)
 - Board 3.0.2
 https://github.com/shubhampaul/Real_Time_Planet_Tracking_System
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <WiFiClientSecureBearSSL.h>
// openssl s_client -connect api.thingspeak.com:443 | openssl x509 -fingerprint -noout
//const uint8_t fingerprint[20] = {0x27, 0x18, 0x92, 0xDD, 0xA4, 0x26, 0xC3, 0x07, 0x09, 0xB9, 0x7A, 0xE6, 0xC5, 0x21, 0xB9, 0x5B, 0x48, 0xF7, 0x16, 0xE1};

//#include <SoftwareSerial.h>

//#include <Adafruit_ADS1015.h>
//Adafruit_ADS1115 ads(0x48);

#include <EEPROM.h>

#include <Sgp4.h>

#define APPNAME "SatelliteTracker v1.0"

char ssid[32] = "";
char password[32] = "";

// mpu6050 part
float yaw;
float pitch;
float roll;
float temperature;

float az_min = 0.0;
float az_max = 360.0;
float el_min = 0.0;
float el_max = 180.0;

// motor part
uint8_t motor_mode;

//
uint8_t display_enable;
uint8_t beep_enable;
uint8_t mpu9250_enable;
uint8_t mpu9250_found = 0;
uint8_t ak8963_found = 0;
uint8_t motor_enable = 1;

float m_bias_x = 0.0;
float m_bias_y = 0.0;
float m_bias_z = 0.0;

float latitude = 14.6112342;
float longitude = 121.1303641;
float altitude = 19;
float declination = -2.57;  // Cainta // dd = d + m/60 + s/3600

uint16_t wifi_error = 0;
uint16_t mpu_error = 0;

uint16_t run_time = 0;

// timing
unsigned long epochTime;
uint8_t second = 0;
//uint8_t minute;
//uint8_t hour;


void loadConfig() {
  EEPROM.begin(512);

  // Wifi part
  EEPROM.get(0, ssid); // 32
  EEPROM.get(32, password); // 32

  // mpu6050 part
  EEPROM.get(64, mpu9250_enable); // 1
  // Display part
  EEPROM.get(65, display_enable); // 1
  // BEEP part
  EEPROM.get(66, beep_enable); // 1

  EEPROM.get(67, latitude); // 4
  EEPROM.get(71, longitude); // 4
  EEPROM.get(75, altitude); // 4
  EEPROM.get(79, declination); // 4

  //EEPROM.get(67, m_bias_x); // 4
  //EEPROM.get(71, m_bias_y); // 4
  //EEPROM.get(75, m_bias_z); // 4

  char ok[3];
  EEPROM.get(83, ok);
  EEPROM.end();
  
  if (String(ok) != String("OK")) {
    ssid[0] = 0;
    password[0] = 0;
    mpu9250_enable = 0;
    display_enable = 0;
    beep_enable = 0;
    m_bias_x = 0.0;
    m_bias_y = 0.0;
    m_bias_z = 0.0;
    declination = -2.57;
  }
}

void saveConfig() {
  EEPROM.begin(512);

  // Wifi part
  EEPROM.put(0, ssid); // 32
  EEPROM.put(32, password); // 32

  // mpu6050 part
  EEPROM.put(64, mpu9250_enable); // 1
  // Display part
  EEPROM.put(65, display_enable); // 1
  // BEEP part
  EEPROM.put(66, beep_enable); // 1

  EEPROM.put(67, latitude); // 4
  EEPROM.put(71, longitude); // 4
  EEPROM.put(75, altitude); // 4
  EEPROM.put(79, declination); // 4

  //EEPROM.put(67, m_bias_x); // 4
  //EEPROM.put(71, m_bias_y); // 4
  //EEPROM.put(75, m_bias_z); // 4

  char ok[3] = "OK";
  EEPROM.put(83, ok);
  EEPROM.commit();
  EEPROM.end();
}

void execEvery(int ms) {
  static unsigned long msTick = millis();

  if (millis() - msTick >= ms) { // run every N ms
    msTick = millis();

    ntpLoop();
    //Serial.println(epochTime);
    trackerLoop();

    if (display_enable) displayLoop();
    //if (mpu6050_enable) mpu6050Loop();
    //if (motor_enable) motorLoop();
    
    if (second >= 59) {

      second = 0;
      run_time++;
    }
    else {
      second++;
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

  //ads.begin();

  delay(100);
}

void loop() {
  execEvery(1000);

  if (mpu9250_enable && mpu9250_found) mpu9250Loop();
  if (motor_enable) motorLoop();

  networkLoop();
  webserverLoop();
}
