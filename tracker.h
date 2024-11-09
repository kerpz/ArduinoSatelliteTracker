#ifndef TRACKER_H
#define TRACKER_H

#include "eeprom.h"
#include "app.h"

extern float sat_latitude;
extern float sat_longitude;
extern float sat_altitude;
extern float sat_azimuth;
extern float sat_elevation;
extern float sat_distance;

extern String catalog_number;

extern uint16_t get_error;
extern int http_code;

void getTLE(String catalog_number);
void trackerSetup();
void trackerLoop();

#endif