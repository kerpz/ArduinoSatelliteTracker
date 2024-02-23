#ifndef MPU6050_H
#define MPU6050_H

#include "app.h"

extern float old_yaw;
extern float old_pitch;
extern float old_roll;
extern float old_temperature;

void mpu6050Setup();
void mpu6050Loop();

#endif