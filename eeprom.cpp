#include <Arduino.h>
// eeprom
#include <EEPROM.h>

#include "eeprom.h"

// globals
char ap_ssid[] = "KERPZ-AP-ST"; // limit to 32 bytes
char ap_key[] = "12345678";     // limit to 16 bytes
uint8_t sta_enable = 1;
char sta_ssid[] = "KERPZ-AP2"; // limit to 32 bytes
char sta_key[] = "12345678";   // limit to 16 bytes

// peripheral devices
uint8_t beep_enable = 0;
uint8_t analog_enable = 1;
uint8_t display_enable = 0;
uint8_t ads1115_enable = 0;

uint8_t post_enable = 0;
char api_url[] = "https://192.168.2.1:8001/cgi-bin/custom-full.cgi?a=iot"; // limit to 256
char api_key[] = "NIJCG7UI28O9CAYF";                                       // limit to 32
uint16_t http_timeout = 0;

uint8_t mpu9250_enable = 0;
uint8_t motor_enable = 1;
uint8_t tracker_enable = 1;

float ms_per_deg = 277.778;

// tracker location
float latitude = 14.6112342;
float longitude = 121.1303641;
float altitude = 19.0;
float declination = -2.57; // Cainta // dd = d + m/60 + s/3600

char tle_name[32] = "ISS (ZARYA)";
char tle_line1[70] = "1 25544U 98067A   24051.89611201  .00017355  00000+0  30940-3 0  9999";
char tle_line2[70] = "2 25544  51.6402 176.6243 0001894 294.5134 162.9214 15.50183938440364";

uint8_t auto_control = 0;

void loadConfig()
{
  EEPROM.begin(600);

  char ok[3];
  EEPROM.get(588, ok);

  if (String(ok) == String("OK"))
  {
    Serial.print("Loading Storage (600b) ... ");
    EEPROM.get(0, ap_ssid);     // 32
    EEPROM.get(32, ap_key);     // 16
    EEPROM.get(48, sta_enable); // 1
    EEPROM.get(49, sta_ssid);   // 32
    EEPROM.get(81, sta_key);    // 16

    EEPROM.get(97, beep_enable);     // 1
    EEPROM.get(98, analog_enable);   // 1
    EEPROM.get(99, display_enable);  // 1
    EEPROM.get(100, ads1115_enable); // 1

    EEPROM.get(101, post_enable);  // 1
    EEPROM.get(102, api_url);      // 256
    EEPROM.get(358, api_key);      // 32
    EEPROM.get(390, http_timeout); // 2

    EEPROM.get(392, mpu9250_enable); // 1
    EEPROM.get(393, motor_enable);   // 1
    EEPROM.get(394, tracker_enable); // 1

    EEPROM.get(395, latitude);    // 4
    EEPROM.get(399, longitude);   // 4
    EEPROM.get(403, altitude);    // 4
    EEPROM.get(407, declination); // 4

    EEPROM.get(411, ms_per_deg); // 4

    EEPROM.get(415, tle_name);  // 32
    EEPROM.get(447, tle_line1); // 70
    EEPROM.get(517, tle_line2); // 70

    EEPROM.get(587, auto_control); // 1
    Serial.println("Done");
  }
  else
  {
    saveConfig();
  }

  EEPROM.end();
}

void saveConfig()
{
  EEPROM.begin(600);

  Serial.print("Saving Storage (600b) ... ");
  EEPROM.put(0, ap_ssid);     // 32
  EEPROM.put(32, ap_key);     // 16
  EEPROM.put(48, sta_enable); // 1
  EEPROM.put(49, sta_ssid);   // 32
  EEPROM.put(81, sta_key);    // 16

  EEPROM.put(97, beep_enable);     // 1
  EEPROM.put(98, analog_enable);   // 1
  EEPROM.put(99, display_enable);  // 1
  EEPROM.put(100, ads1115_enable); // 1

  EEPROM.put(101, post_enable);  // 1
  EEPROM.put(102, api_url);      // 256
  EEPROM.put(358, api_key);      // 32
  EEPROM.put(390, http_timeout); // 2

  EEPROM.put(392, mpu9250_enable); // 1
  EEPROM.put(393, motor_enable);   // 1
  EEPROM.put(394, tracker_enable); // 1

  EEPROM.put(395, latitude);    // 4
  EEPROM.put(399, longitude);   // 4
  EEPROM.put(403, altitude);    // 4
  EEPROM.put(407, declination); // 4

  EEPROM.put(411, ms_per_deg); // 4

  EEPROM.put(415, tle_name);  // 32
  EEPROM.put(447, tle_line1); // 70
  EEPROM.put(517, tle_line2); // 70

  EEPROM.put(587, auto_control); // 1

  char ok[3] = "OK";
  EEPROM.put(588, ok);
  EEPROM.commit();
  Serial.println("Done");

  EEPROM.end();
}
