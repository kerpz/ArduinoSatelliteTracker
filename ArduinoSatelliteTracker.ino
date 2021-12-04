/*
 Author:
 - Philip Bordado (kerpz@yahoo.com)

 Hardware:
 - Wemos D1 mini (Compatible board)
 
 Software:
 - Arduino 1.8.12 (Stable)
 - Board 2.7.1, payload bug empty
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include <WiFiClientSecureBearSSL.h>
// openssl s_client -connect api.thingspeak.com:443 | openssl x509 -fingerprint -noout
//const uint8_t fingerprint[20] = {0x27, 0x18, 0x92, 0xDD, 0xA4, 0x26, 0xC3, 0x07, 0x09, 0xB9, 0x7A, 0xE6, 0xC5, 0x21, 0xB9, 0x5B, 0x48, 0xF7, 0x16, 0xE1};

//#include <SoftwareSerial.h>

//#include <Adafruit_ADS1015.h>
//Adafruit_ADS1115 ads(0x48);

#include <EEPROM.h>

#define APPNAME "SatelliteTracker v1.0"

#define APssid "SatelliteTracker-AP"
#define APpassword  "12345678"

char ssid[32] = "";
char password[32] = "";

// mpu6050 part
float x;
float y;
float z;
float temperature;

// motor part
uint8_t motor_mode;

//
uint8_t display_enable;
uint8_t beep_enable;
uint8_t mpu6050_enable;
uint8_t motor_enable = 1;

uint16_t wifi_error = 0;
uint16_t mpu6050_error = 0;

uint16_t run_time = 0;

// timing
uint8_t second = 0;
//uint8_t minute;
//uint8_t hour;

// Web server
ESP8266WebServer webServer(80);
// DNS server
DNSServer dnsServer;

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

void loadConfig() {
  EEPROM.begin(512);

  // Wifi part
  EEPROM.get(0, ssid); // 32
  EEPROM.get(32, password); // 32

  // mpu6050 part
  EEPROM.get(64, mpu6050_enable); // 1
  // Display part
  EEPROM.get(65, display_enable); // 1
  // BEEP part
  EEPROM.get(66, beep_enable); // 1

  char ok[3];
  EEPROM.get(67, ok);
  EEPROM.end();
  
  if (String(ok) != String("OK")) {
    ssid[0] = 0;
    password[0] = 0;
    display_enable = 0;
    beep_enable = 0;
  }
}

void saveConfig() {
  EEPROM.begin(512);

  // Wifi part
  EEPROM.put(0, ssid); // 32
  EEPROM.put(32, password); // 32

  // mpu6050 part
  EEPROM.put(64, mpu6050_enable); // 1
  // Display part
  EEPROM.put(65, display_enable); // 1
  // BEEP part
  EEPROM.put(66, beep_enable); // 1

  char ok[3] = "OK";
  EEPROM.put(67, ok);
  EEPROM.commit();
  EEPROM.end();
}

void execEvery(int ms) {
  static unsigned long msTick = millis();

  if (millis() - msTick >= ms) { // run every N ms
    msTick = millis();

    if (display_enable) displayLoop();
    //if (mpu6050_enable) mpu6050Loop();
    if (motor_enable) motorLoop();
    
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
  Serial.println(APssid);

  loadConfig();

  wifiSetup();

  if (display_enable) displaySetup();
  if (mpu6050_enable) mpu6050Setup();
  if (motor_enable) motorSetup();

  //ads.begin();

  dnsServer.start(53, "*", apIP);

  webserverSetup();

  delay(100);
}

void loop() {
  execEvery(5);

  if (mpu6050_enable) mpu6050Loop();
  // dns server
  dnsServer.processNextRequest();
  // web server
  webServer.handleClient();
}
