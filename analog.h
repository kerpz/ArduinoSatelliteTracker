#ifndef ANALOG_H
#define ANALOG_H

extern float a_voltage;
extern float a_current;
extern float a_power;

void analogSetup();
void analogLoop();

#endif