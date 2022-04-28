#include <EEPROM.h>

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

  char ok[3];
  EEPROM.get(83, ok);
  EEPROM.end();
  
  if (String(ok) != String("OK")) {
    ssid[0] = 0;
    password[0] = 0;
    mpu9250_enable = 0;
    display_enable = 0;
    beep_enable = 0;
    latitude = 0.0;
    longitude = 0.0;
    altitude = 0.0;
    declination = 0.0;
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

  char ok[3] = "OK";
  EEPROM.put(83, ok);
  EEPROM.commit();
  EEPROM.end();
}
