#ifndef APP_H
#define APP_H

#include "eeprom.h"

#define APPNAME "Satellite v1.0"

#define APssid "Satellite-AP"
#define APpassword "12345678"

// motor part
extern uint8_t motor_mode_az;
extern float motor_az;
extern uint8_t motor_mode_el;
extern float motor_el;

// extern float cal_msPerDeg;
// extern uint32_t debug_ms;
// extern float debug_deg;

extern uint16_t run_time;

// timing
extern int timezone;
extern unsigned long epoch;
extern uint8_t second;
extern uint8_t minute;
extern uint8_t hour;
extern uint8_t day;
extern uint8_t month;
extern uint16_t year;
extern char datetime[32];

// for manual control / calibration
void stopAz();
void stopEl();
void forwardAz();
void reverseAz();
void forwardEl();
void reverseEl();

void setAz(int deg);
void setEl(int deg);

void appSetup();
void appLoop();

#endif