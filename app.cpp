// wifi
#include <ESP8266WiFi.h>

#include "app.h"

// const int sclPin = D1;  // SCL
// const int sdaPin = D2;  // SDA
// const int out1Control = D3; // use pnp // pulled up 2n3906
// const int out2Control = D4; // use pnp pulled up 2n3906
// L298 Dual H Bridge motor pins
const int azFwdPin = D5;
const int azRevPin = D6;
const int elFwdPin = D7;
const int elRevPin = D8;

// motor specs
// 0.6 RPM 12v Torque Worm
// 0.6 RPM = 3.6 Deg/s
// 278 ms/Deg

// az 0 to 360, el 0 to 90
// yaw,roll -180 to 180, pitch -90 to 90

uint32_t targetTimeAz = 0; // ms
uint32_t targetTimeEl = 0; // ms

// calibrate Az 360 degrees?
// uint8_t tmp_degree = 0;
// unsigned long tmp_start_ms; // ms
// float cal_msPerDeg;         // ms
uint32_t debug_ms = 0;
float debug_deg = 0.0;

// global
uint8_t motor_mode_az = 0;
float motor_az = 0.0;
uint8_t motor_mode_el = 0;
float motor_el = 0.0;

uint16_t run_time = 0;

// timing
int timezone = 8;
uint32_t epoch = 0;
uint8_t second = 0;
uint8_t minute = 0;
uint8_t hour = 0;
uint8_t day = 0;
uint8_t month = 0;
uint16_t year = 0;

void stopAz()
{
  digitalWrite(azFwdPin, LOW);
  digitalWrite(azRevPin, LOW);
  Serial.println("stopAz");
  motor_mode_az = 0;
  targetTimeAz = 0;
  // cal_msPerDeg = (millis() - tmp_start_ms) / 360; // calculate // calibration only
}

void stopEl()
{
  digitalWrite(elFwdPin, LOW);
  digitalWrite(elRevPin, LOW);
  Serial.println("stopEl");
  motor_mode_el = 0;
  targetTimeEl = 0;
}

void forwardAz()
{
  // tmp_start_ms = millis(); // save start // msPerDeg calibration only
  digitalWrite(azFwdPin, LOW);
  digitalWrite(azRevPin, HIGH);
  Serial.println("forwardAz");
  motor_mode_az = 1;
}

void reverseAz()
{
  digitalWrite(azFwdPin, HIGH);
  digitalWrite(azRevPin, LOW);
  Serial.println("reverseAz");
  motor_mode_az = 2;
}

void forwardEl()
{
  digitalWrite(elFwdPin, LOW);
  digitalWrite(elRevPin, HIGH);
  Serial.println("forwardEl");
  motor_mode_el = 1;
}

void reverseEl()
{
  digitalWrite(elFwdPin, HIGH);
  digitalWrite(elRevPin, LOW);
  Serial.println("reverseEl");
  motor_mode_el = 2;
}

void setAz(int deg) // 0 - 360
{
  if (deg >= 0 && deg <= 360 && motor_mode_az == 0)
  {
    int t_deg = motor_az - deg;

    uint32_t ms = abs(t_deg) * ms_per_deg;
    targetTimeAz = millis() + ms;

    if (t_deg < 0)
      forwardAz();
    else if (t_deg > 0)
      reverseAz();

    motor_az = deg;
  }
}

void setEl(int deg) // 0 - 90
{
  if (deg >= 0 && deg <= 90 && motor_mode_el == 0)
  {
    int t_deg = motor_el - deg;

    uint32_t ms = abs(t_deg) * ms_per_deg;
    targetTimeEl = millis() + ms;

    if (t_deg < 0)
      forwardEl();
    else if (t_deg > 0)
      reverseEl();

    motor_el = deg;
  }
}

void appSetup()
{
  pinMode(azFwdPin, OUTPUT);
  pinMode(azRevPin, OUTPUT);
  pinMode(elFwdPin, OUTPUT);
  pinMode(elRevPin, OUTPUT);

  stopAz();
  stopEl();

  // get from sensor, or manually fixed set
  motor_az = 0.0; // pointing north
  motor_el = 0.0; // pointing horizon

  getTLE("25544");
  delay(100);
  trackerSetup();

  mpu9250Setup();
}

void appLoop()
{
  static uint32_t msTick = millis();

  if (millis() - msTick >= 1000) // 1000ms refresh rate
  {
    msTick = millis();

    ntpLoop();

    if (analog_enable)
      analogLoop();

    if (tracker_enable)
      trackerLoop();

    if (mpu9250_enable)
      mpu9250Loop();

    if (second >= 59)
    {
      if (run_time < 65500)
        run_time++;

      // if (post_enable)
      //   postLoop();
      second = 0;

      /*
      if (minute >= 59)
      {
        minute = 0;
        if (hour >= 23)
        {
          hour = 0;
        }
        else
          hour++;
      }
      else
        minute++;
        */
    }
    else
      second++;
  }

  if (motor_mode_az > 0 && targetTimeAz != 0 && targetTimeAz <= millis())
    stopAz(); // stop
  if (motor_mode_el > 0 && targetTimeEl != 0 && targetTimeEl <= millis())
    stopEl(); // stop
}
