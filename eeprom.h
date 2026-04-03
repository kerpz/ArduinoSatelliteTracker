#ifndef EEPROM_H
#define EEPROM_H

extern char ap_ssid[32];
extern char ap_key[16];
extern uint8_t sta_enable;
extern char sta_ssid[32];
extern char sta_key[16];

// peripheral devices
extern uint8_t beep_enable;
extern uint8_t analog_enable;
extern uint8_t display_enable;
extern uint8_t ads1115_enable;

extern uint8_t post_enable;
extern char api_key[32];
extern char api_url[256];
extern uint16_t http_timeout;

extern uint8_t mpu9250_enable;
extern uint8_t motor_enable;
extern uint8_t tracker_enable;

extern float ms_per_deg;

extern float latitude;
extern float longitude;
extern float altitude;
extern float declination;

extern char tle_name[32];
extern char tle_line1[70];
extern char tle_line2[70];

extern uint8_t auto_control;

void loadConfig();
void saveConfig();

#endif