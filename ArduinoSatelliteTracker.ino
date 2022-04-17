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
*/

#define APPNAME "SatelliteTracker v1.0"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

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
