#include <Arduino.h>
// eeprom
#include <EEPROM.h>

#include "eeprom.h"

// globals
char ssid[32] = "KERPZ-AP2";     // limit to 32 bytes
char password[32] = "hackmelol"; // limit to 32 bytes

// peripheral devices
uint8_t beep_enable = 0;
uint8_t analog_enable = 0;
uint8_t display_enable = 0;
uint8_t ads1115_enable = 0;
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

// uint8_t post_enable = 0;
// char api_key[] = "NIJCG7UI28O9CAYD";                                          // limit to 32
// char api_url[] = "https://192.168.2.1:8001/cgi-bin/custom-full.cgi?a=solard"; // limit to 256
// uint16_t http_timeout = 0;

void loadConfig()
{
  EEPROM.begin(512);

  char ok[3];
  EEPROM.get(264, ok);

  if (String(ok) == String("OK"))
  {
    Serial.print("Loading Storage (512b) ... ");
    // Wifi part
    EEPROM.get(0, ssid);      // 32
    EEPROM.get(32, password); // 32

    EEPROM.get(64, beep_enable);    // 1
    EEPROM.get(65, analog_enable);  // 1
    EEPROM.get(66, display_enable); // 1
    EEPROM.get(67, ads1115_enable); // 1
    EEPROM.get(68, mpu9250_enable); // 1
    EEPROM.get(69, motor_enable);   // 1
    EEPROM.get(70, tracker_enable); // 1

    EEPROM.get(71, latitude);    // 4
    EEPROM.get(75, longitude);   // 4
    EEPROM.get(79, altitude);    // 4
    EEPROM.get(83, declination); // 4

    EEPROM.get(87, ms_per_deg); // 4

    EEPROM.get(91, tle_name);   // 32
    EEPROM.get(123, tle_line1); // 70
    EEPROM.get(193, tle_line2); // 70

    EEPROM.get(263, auto_control); // 1
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
  EEPROM.begin(512);

  Serial.print("Saving Storage (512b) ... ");
  // Wifi part
  EEPROM.put(0, ssid);      // 32
  EEPROM.put(32, password); // 32

  EEPROM.put(64, beep_enable);    // 1
  EEPROM.put(65, analog_enable);  // 1
  EEPROM.put(66, display_enable); // 1
  EEPROM.put(67, ads1115_enable); // 1
  EEPROM.put(68, mpu9250_enable); // 1
  EEPROM.put(69, motor_enable);   // 1
  EEPROM.put(70, tracker_enable); // 1

  EEPROM.put(71, latitude);    // 4
  EEPROM.put(75, longitude);   // 4
  EEPROM.put(79, altitude);    // 4
  EEPROM.put(83, declination); // 4

  EEPROM.put(87, ms_per_deg); // 4

  EEPROM.put(91, tle_name);   // 32
  EEPROM.put(123, tle_line1); // 70
  EEPROM.put(193, tle_line2); // 70

  EEPROM.put(263, auto_control); // 1

  char ok[3] = "OK";
  EEPROM.put(264, ok);
  EEPROM.commit();
  Serial.println("Done");

  EEPROM.end();
}
