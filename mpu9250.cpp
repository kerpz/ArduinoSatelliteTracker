#include <Arduino.h>
#include <Wire.h>

#include "mpu9250.h"
// pitch = 180 * atan2(accelX, sqrt(accelY*accelY + accelZ*accelZ))/PI;
// roll = 180 * atan2(accelY, sqrt(accelX*accelX + accelZ*accelZ))/PI;
//  https://roboticsclubiitk.github.io/2017/12/21/Beginners-Guide-to-IMU.html

// https://github.com/miniben-90/mpu9250
// https://github.com/psiphi75/esp-mpu9250/blob/master/components/mpu9250/calibrate.c
// https://github.com/makerportal/mpu92-calibration

float yaw;
float pitch;
float roll;
float temperature;

uint16_t mpu_error;

// Specify sensor full scale
// uint8_t Gscale = GFS_250DPS, Ascale = AFS_2G, Mscale = MFS_16BITS, Mmode = M_100Hz, sampleRate = 0x04;
uint8_t Gscale = GFS_250DPS, Ascale = AFS_2G, Mscale = MFS_16BITS, Mmode = M_100Hz, sampleRate = 0x00;
float aRes, gRes, mRes; // scale

// vvvvvvvvvvvvvvvvvv  VERY VERY IMPORTANT vvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// These are the previously determined offsets and correction factors for accelerometer and magnetometer, using MPU9250_cal and Magneto 1.2
// The AHRS will NOT work well or at all if these are not correct
//
// redetermined 12/25/2020 (0 rejected)
// acel offsets and correction matrix
// float A_B[3] { 565.83, 195.43, 848.90 };

// float A_Ainv[3][3] {
//   {1.00380, -0.00143, 0.00704},
//   {-0.00143, 1.00976, -0.00026},
//   {0.00704, -0.00026, 0.98564}
// };

// mag offsets and correction matrix
// float M_B[3] {17.22, 28.11, -39.81};

// float M_Ainv[3][3] {
//   {1.19679, 0.00488, 0.00902},
//   {0.00488, 1.20826, 0.00392},
//   {0.00902, 0.00392, 1.21853}
// };

float G_off[3] = {0.0, 0.0, 0.0}; // raw offsets, determined for gyro at rest
float M_coeffs[3] = {0.0, 0.0, 0.0};

int16_t aRaw[3];
int16_t gRaw[3];
int16_t mRaw[3];
int16_t tRaw;

float Axyz[3];
float Gxyz[3];
float Mxyz[3];

float SelfTest[6]; // holds results of gyro and accelerometer self test

// These are the free parameters in the Mahony filter and fusion scheme,
// Kp for proportional feedback, Ki for integral
// with MPU-9250, angles start oscillating at Kp=40. Ki does not seem to help and is not required.
#define Kp 30.0
#define Ki 0.0

// Madwick filter parameters
static float GyroMeasError = 40.0 * (PI / 180.0);
// gyroscope measurement drift in rad/s/s (start at 0.0 deg/s/s)
static float GyroMeasDrift = 0.0 * (PI / 180.0);
// There is a tradeoff in the beta parameter between accuracy and response
// speed. In the original Madgwick study, beta of 0.041 (corresponding to
// GyroMeasError of 2.7 degrees/s) was found to give optimal accuracy.
// However, with this value, the LSM9SD0 response time is about 10 seconds
// to a stable initial quaternion. By increasing beta by factor of
// fifteen, the response time constant is reduced to ~2 sec. I haven't
// noticed any reduction in solution accuracy.
static float beta = sqrt(3.0 / 4.0) * GyroMeasError; // Compute beta
// Compute zeta, the other free parameter in the Madgwick scheme usually
// set to a small or zero value
static float zeta = sqrt(3.0 / 4.0) * GyroMeasDrift;

uint32_t now = 0, last = 0;
float deltat = 0.0;

float q[4] = {1.0, 0.0, 0.0, 0.0}; // vector to hold quaternion
float eInt[3] = {0.0, 0.0, 0.0};   // vector to hold integral error for Mahony method

void I2Cread(uint8_t Address, uint8_t Register, uint8_t Nbytes, uint8_t *Data)
{
  Wire.beginTransmission(Address);
  Wire.write(Register);
  if (Wire.endTransmission() != 0)
  {
    mpu_error++;
  }
  else
  {
    Wire.requestFrom(Address, Nbytes);
    uint8_t index = 0;
    while (Wire.available())
      Data[index++] = Wire.read();
  }
}

void I2Cwrite(uint8_t Address, uint8_t Register, uint8_t Data)
{
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.write(Data);
  if (Wire.endTransmission() != 0)
    mpu_error++;
}

//===================================================================================================================
//====== Set of useful function to access acceleration. gyroscope, magnetometer, and temperature data
//===================================================================================================================
void getMres()
{
  switch (Mscale)
  {
  // Possible magnetometer scales (and their register bit settings) are:
  // 14 bit resolution (0) and 16 bit resolution (1)
  case MFS_14BITS:
    mRes = 4912.0 / 8190.0; // Proper scale to return uT
    break;
  case MFS_16BITS:
    mRes = 4912.0 / 32760.0; // Proper scale to return uT
    break;
  }
}

void getGres()
{
  switch (Gscale)
  {
  // Possible gyro scales (and their register bit settings) are:
  // 250 DPS (00), 500 DPS (01), 1000 DPS (10), and 2000 DPS  (11).
  // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
  case GFS_250DPS:
    gRes = 250.0 / 32768.0;
    break;
  case GFS_500DPS:
    gRes = 500.0 / 32768.0;
    break;
  case GFS_1000DPS:
    gRes = 1000.0 / 32768.0;
    break;
  case GFS_2000DPS:
    gRes = 2000.0 / 32768.0;
    break;
  }
}

void getAres()
{
  switch (Ascale)
  {
  // Possible accelerometer scales (and their register bit settings) are:
  // 2 Gs (00), 4 Gs (01), 8 Gs (10), and 16 Gs  (11).
  // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
  case AFS_2G:
    aRes = 2.0 / 32768.0;
    break;
  case AFS_4G:
    aRes = 4.0 / 32768.0;
    break;
  case AFS_8G:
    aRes = 8.0 / 32768.0;
    break;
  case AFS_16G:
    aRes = 16.0 / 32768.0;
    break;
  }
}

void readAccelData(int16_t *destination)
{
  uint8_t rawData[6];                                       // x/y/z accel register data stored here
  I2Cread(MPU9250_ADDRESS, ACCEL_XOUT_H, 6, &rawData[0]);   // Read the six raw data registers into data array
  destination[0] = ((int16_t)rawData[0] << 8) | rawData[1]; // Turn the MSB and LSB into a signed 16-bit value
  destination[1] = ((int16_t)rawData[2] << 8) | rawData[3];
  destination[2] = ((int16_t)rawData[4] << 8) | rawData[5];
}

void readGyroData(int16_t *destination)
{
  uint8_t rawData[6];                                       // x/y/z gyro register data stored here
  I2Cread(MPU9250_ADDRESS, GYRO_XOUT_H, 6, &rawData[0]);    // Read the six raw data registers sequentially into data array
  destination[0] = ((int16_t)rawData[0] << 8) | rawData[1]; // Turn the MSB and LSB into a signed 16-bit value
  destination[1] = ((int16_t)rawData[2] << 8) | rawData[3];
  destination[2] = ((int16_t)rawData[4] << 8) | rawData[5];
}

void readMagData(int16_t *destination)
{
  uint8_t Buf[1] = {0};
  uint8_t rawData[7]; // x/y/z gyro register data, ST2 register stored here, must read ST2 at end of data acquisition

  // I2Cwrite(MPU9250_ADDRESS, INT_PIN_CFG, 0x02); //set i2c bypass enable pin to true to access magnetometer
  // delay(10);
  // I2Cwrite(AK8963_ADDRESS, AK8963_CNTL, 0x01); //enable the magnetometer
  // delay(10);

  // I2Cread(AK8963_ADDRESS, AK8963_ST1, 1, Buf);
  // if (Buf[0] & 0x01) {
  I2Cread(AK8963_ADDRESS, AK8963_XOUT_L, 7, &rawData[0]); // Read the six raw data and ST2 registers sequentially into data array
  if (!(rawData[6] & 0x08))
  {                                                           // Check if magnetic sensor overflow set, if not then report data
    destination[0] = ((int16_t)rawData[1] << 8) | rawData[0]; // Turn the MSB and LSB into a signed 16-bit value
    destination[1] = ((int16_t)rawData[3] << 8) | rawData[2]; // Data stored as little Endian
    destination[2] = ((int16_t)rawData[5] << 8) | rawData[4];
  }
  //}
}

int16_t readTempData()
{
  uint8_t rawData[2];                                   // x/y/z gyro register data stored here
  I2Cread(MPU9250_ADDRESS, TEMP_OUT_H, 2, &rawData[0]); // Read the two raw data registers sequentially into data array
  return ((int16_t)rawData[0] << 8) | rawData[1];       // Turn the MSB and LSB into a 16-bit value
}

void mpu9250Init()
{
  uint8_t Buf[1] = {0};
  I2Cwrite(MPU9250_ADDRESS, PWR_MGMT_1, 0x00);
  delay(100);
  I2Cwrite(MPU9250_ADDRESS, PWR_MGMT_1, 0x01);
  delay(200);

  // Configure Gyro and Thermometer
  // Disable FSYNC and set thermometer and gyro bandwidth to 41 and 42 Hz, respectively;
  // minimum delay time for this setting is 5.9 ms, which means sensor fusion update rates cannot
  // be higher than 1 / 0.0059 = 170 Hz
  // DLPF_CFG = bits 2:0 = 011; this limits the sample rate to 1000 Hz for both
  // With the MPU9250, it is possible to get gyro sample rates of 32 kHz (!), 8 kHz, or 1 kHz
  I2Cwrite(MPU9250_ADDRESS, CONFIG, 0x00); // 0x03

  // Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
  I2Cwrite(MPU9250_ADDRESS, SMPLRT_DIV, sampleRate);

  // Set gyroscope full scale range
  // Range selects FS_SEL and AFS_SEL are 0 - 3, so 2-bit values are left-shifted into positions 4:3
  I2Cread(MPU9250_ADDRESS, GYRO_CONFIG, 1, Buf); // get current GYRO_CONFIG register value
  // Buf[0] = Buf[0] & ~0xE0; // Clear self-test bits [7:5]
  Buf[0] = Buf[0] & ~0x02;                        // Clear Fchoice bits [1:0] // 0x03
  Buf[0] = Buf[0] & ~0x18;                        // Clear GFS bits [4:3]
  Buf[0] = Buf[0] | Gscale << 3;                  // Set full scale range for the gyro
  I2Cwrite(MPU9250_ADDRESS, GYRO_CONFIG, Buf[0]); // Write new GYRO_CONFIG value to register

  // Set accelerometer full-scale range configuration
  I2Cread(MPU9250_ADDRESS, ACCEL_CONFIG, 1, Buf); // get current ACCEL_CONFIG register value
  // Buf[0] = Buf[0] & ~0xE0; // Clear self-test bits [7:5]
  Buf[0] = Buf[0] & ~0x18;                         // Clear AFS bits [4:3]
  Buf[0] = Buf[0] | Ascale << 3;                   // Set full scale range for the accelerometer
  I2Cwrite(MPU9250_ADDRESS, ACCEL_CONFIG, Buf[0]); // Write new ACCEL_CONFIG register value

  // Set accelerometer sample rate configuration
  // It is possible to get a 4 kHz sample rate from the accelerometer by choosing 1 for
  // accel_fchoice_b bit [3]; in this case the bandwidth is 1.13 kHz
  // I2Cread(MPU9250_ADDRESS, ACCEL_CONFIG2, 1, Buf); // get current ACCEL_CONFIG2 register value
  // Buf[0] = Buf[0] & ~0x0F; // Clear accel_fchoice_b (bit 3) and A_DLPFG (bits [2:0])
  // Buf[0] = Buf[0] | 0x03;  // Set accelerometer rate to 1 kHz and bandwidth to 41 Hz
  // I2Cwrite(MPU9250_ADDRESS, ACCEL_CONFIG2, Buf[0]); // Write new ACCEL_CONFIG2 register value

  I2Cwrite(MPU9250_ADDRESS, INT_PIN_CFG, 0x22); // 0x10
  // enable ak8963
  I2Cwrite(MPU9250_ADDRESS, INT_ENABLE, 0x01);
  delay(100);

  // I2Cwrite(MPU9250_ADDRESS, USER_CTRL, 0x20);          // Enable I2C Master mode
  // I2Cwrite(MPU9250_ADDRESS, I2C_MST_CTRL, 0x1D);       // I2C configuration STOP after each transaction, master I2C bus at 400 KHz
  // I2Cwrite(MPU9250_ADDRESS, I2C_MST_DELAY_CTRL, 0x81); // Use blocking data retreival and enable delay for mag sample rate mismatch
  // I2Cwrite(MPU9250_ADDRESS, I2C_SLV4_CTRL, 0x01);      // Delay mag data retrieval to on
}

void mpu9250Calibrate(float *dest1, float *dest2)
{
  uint8_t data[12]; // data array to hold accelerometer and gyro x, y, z, data
  uint16_t ii, packet_count, fifo_count;
  int32_t gyro_bias[3] = {0, 0, 0};
  int32_t accel_bias[3] = {0, 0, 0};

  // reset device
  I2Cwrite(MPU9250_ADDRESS, PWR_MGMT_1, 0x80); // Write a one to bit 7 reset bit; toggle reset device
  delay(100);

  // get stable time source; Auto select clock source to be PLL gyroscope reference if ready
  // else use the internal oscillator, bits 2:0 = 001
  I2Cwrite(MPU9250_ADDRESS, PWR_MGMT_1, 0x01);
  I2Cwrite(MPU9250_ADDRESS, PWR_MGMT_2, 0x00);
  delay(200);

  // Configure device for bias calculation
  I2Cwrite(MPU9250_ADDRESS, INT_ENABLE, 0x00);   // Disable all interrupts
  I2Cwrite(MPU9250_ADDRESS, FIFO_EN, 0x00);      // Disable FIFO
  I2Cwrite(MPU9250_ADDRESS, PWR_MGMT_1, 0x00);   // Turn on internal clock source
  I2Cwrite(MPU9250_ADDRESS, I2C_MST_CTRL, 0x00); // Disable I2C master
  I2Cwrite(MPU9250_ADDRESS, USER_CTRL, 0x00);    // Disable FIFO and I2C master modes
  I2Cwrite(MPU9250_ADDRESS, USER_CTRL, 0x0C);    // Reset FIFO and DMP
  delay(15);

  // Configure MPU6050 gyro and accelerometer for bias calculation
  I2Cwrite(MPU9250_ADDRESS, CONFIG, 0x01);       // Set low-pass filter to 188 Hz
  I2Cwrite(MPU9250_ADDRESS, SMPLRT_DIV, 0x00);   // Set sample rate to 1 kHz
  I2Cwrite(MPU9250_ADDRESS, GYRO_CONFIG, 0x00);  // Set gyro full-scale to 250 degrees per second, maximum sensitivity
  I2Cwrite(MPU9250_ADDRESS, ACCEL_CONFIG, 0x00); // Set accelerometer full-scale to 2 g, maximum sensitivity

  uint16_t gyrosensitivity = 131;    // = 131 LSB/degrees/sec
  uint16_t accelsensitivity = 16384; // = 16384 LSB/g

  // Configure FIFO to capture accelerometer and gyro data for bias calculation
  I2Cwrite(MPU9250_ADDRESS, USER_CTRL, 0x40); // Enable FIFO
  I2Cwrite(MPU9250_ADDRESS, FIFO_EN, 0x78);   // Enable gyro and accelerometer sensors for FIFO  (max size 512 bytes in MPU-9150)
  delay(40);                                  // accumulate 40 samples in 40 milliseconds = 480 bytes

  // At end of sample accumulation, turn off FIFO sensor read
  I2Cwrite(MPU9250_ADDRESS, FIFO_EN, 0x00); // Disable gyro and accelerometer sensors for FIFO
  I2Cread(MPU9250_ADDRESS, FIFO_COUNTH, 2, &data[0]);
  fifo_count = ((uint16_t)data[0] << 8) | data[1];
  packet_count = fifo_count / 12; // How many sets of full gyro and accelerometer data for averaging

  for (ii = 0; ii < packet_count; ii++)
  {
    int16_t accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
    I2Cread(MPU9250_ADDRESS, FIFO_R_W, 12, &data[0]);
    accel_temp[0] = (int16_t)(((int16_t)data[0] << 8) | data[1]); // Form signed 16-bit integer for each sample in FIFO
    accel_temp[1] = (int16_t)(((int16_t)data[2] << 8) | data[3]);
    accel_temp[2] = (int16_t)(((int16_t)data[4] << 8) | data[5]);
    gyro_temp[0] = (int16_t)(((int16_t)data[6] << 8) | data[7]);
    gyro_temp[1] = (int16_t)(((int16_t)data[8] << 8) | data[9]);
    gyro_temp[2] = (int16_t)(((int16_t)data[10] << 8) | data[11]);

    accel_bias[0] += (int32_t)accel_temp[0]; // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
    accel_bias[1] += (int32_t)accel_temp[1];
    accel_bias[2] += (int32_t)accel_temp[2];
    gyro_bias[0] += (int32_t)gyro_temp[0];
    gyro_bias[1] += (int32_t)gyro_temp[1];
    gyro_bias[2] += (int32_t)gyro_temp[2];
  }
  accel_bias[0] /= (int32_t)packet_count; // Normalize sums to get average count biases
  accel_bias[1] /= (int32_t)packet_count;
  accel_bias[2] /= (int32_t)packet_count;
  gyro_bias[0] /= (int32_t)packet_count;
  gyro_bias[1] /= (int32_t)packet_count;
  gyro_bias[2] /= (int32_t)packet_count;

  if (accel_bias[2] > 0L)
  {
    accel_bias[2] -= (int32_t)accelsensitivity; // Remove gravity from the z-axis accelerometer bias calculation
  }
  else
  {
    accel_bias[2] += (int32_t)accelsensitivity;
  }

  // Construct the gyro biases for push to the hardware gyro bias registers, which are reset to zero upon device startup
  data[0] = (-gyro_bias[0] / 4 >> 8) & 0xFF; // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
  data[1] = (-gyro_bias[0] / 4) & 0xFF;      // Biases are additive, so change sign on calculated average gyro biases
  data[2] = (-gyro_bias[1] / 4 >> 8) & 0xFF;
  data[3] = (-gyro_bias[1] / 4) & 0xFF;
  data[4] = (-gyro_bias[2] / 4 >> 8) & 0xFF;
  data[5] = (-gyro_bias[2] / 4) & 0xFF;

  // Push gyro biases to hardware registers
  I2Cwrite(MPU9250_ADDRESS, XG_OFFSET_H, data[0]);
  I2Cwrite(MPU9250_ADDRESS, XG_OFFSET_L, data[1]);
  I2Cwrite(MPU9250_ADDRESS, YG_OFFSET_H, data[2]);
  I2Cwrite(MPU9250_ADDRESS, YG_OFFSET_L, data[3]);
  I2Cwrite(MPU9250_ADDRESS, ZG_OFFSET_H, data[4]);
  I2Cwrite(MPU9250_ADDRESS, ZG_OFFSET_L, data[5]);

  // Output scaled gyro biases for display in the main program
  dest1[0] = (float)gyro_bias[0] / (float)gyrosensitivity;
  dest1[1] = (float)gyro_bias[1] / (float)gyrosensitivity;
  dest1[2] = (float)gyro_bias[2] / (float)gyrosensitivity;

  // Construct the accelerometer biases for push to the hardware accelerometer bias registers. These registers contain
  // factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold
  // non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for temperature
  // compensation calculations. Accelerometer bias registers expect bias input as 2048 LSB per g, so that
  // the accelerometer biases calculated above must be divided by 8.

  int32_t accel_bias_reg[3] = {0, 0, 0}; // A place to hold the factory accelerometer trim biases
  I2Cread(MPU9250_ADDRESS, XA_OFFSET_H, 2, &data[0]);
  accel_bias_reg[0] = (int32_t)(((int16_t)data[0] << 8) | data[1]);
  I2Cread(MPU9250_ADDRESS, YA_OFFSET_H, 2, &data[0]);
  accel_bias_reg[1] = (int32_t)(((int16_t)data[0] << 8) | data[1]);
  I2Cread(MPU9250_ADDRESS, ZA_OFFSET_H, 2, &data[0]);
  accel_bias_reg[2] = (int32_t)(((int16_t)data[0] << 8) | data[1]);

  uint32_t mask = 1uL;             // Define mask for temperature compensation bit 0 of lower byte of accelerometer bias registers
  uint8_t mask_bit[3] = {0, 0, 0}; // Define array to hold mask bit for each accelerometer bias axis

  for (ii = 0; ii < 3; ii++)
  {
    if ((accel_bias_reg[ii] & mask))
      mask_bit[ii] = 0x01; // If temperature compensation bit is set, record that fact in mask_bit
  }

  // Construct total accelerometer bias, including calculated average accelerometer bias from above
  accel_bias_reg[0] -= (accel_bias[0] / 8); // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g (16 g full scale)
  accel_bias_reg[1] -= (accel_bias[1] / 8);
  accel_bias_reg[2] -= (accel_bias[2] / 8);

  data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
  data[1] = (accel_bias_reg[0]) & 0xFF;
  data[1] = data[1] | mask_bit[0]; // preserve temperature compensation bit when writing back to accelerometer bias registers
  data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
  data[3] = (accel_bias_reg[1]) & 0xFF;
  data[3] = data[3] | mask_bit[1]; // preserve temperature compensation bit when writing back to accelerometer bias registers
  data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
  data[5] = (accel_bias_reg[2]) & 0xFF;
  data[5] = data[5] | mask_bit[2]; // preserve temperature compensation bit when writing back to accelerometer bias registers

  // Apparently this is not working for the acceleration biases in the MPU-9250
  // Are we handling the temperature correction bit properly?
  // Push accelerometer biases to hardware registers
  I2Cwrite(MPU9250_ADDRESS, XA_OFFSET_H, data[0]);
  I2Cwrite(MPU9250_ADDRESS, XA_OFFSET_L, data[1]);
  I2Cwrite(MPU9250_ADDRESS, YA_OFFSET_H, data[2]);
  I2Cwrite(MPU9250_ADDRESS, YA_OFFSET_L, data[3]);
  I2Cwrite(MPU9250_ADDRESS, ZA_OFFSET_H, data[4]);
  I2Cwrite(MPU9250_ADDRESS, ZA_OFFSET_L, data[5]);

  // Output scaled accelerometer biases for display in the main program
  dest2[0] = (float)accel_bias[0] / (float)accelsensitivity;
  dest2[1] = (float)accel_bias[1] / (float)accelsensitivity;
  dest2[2] = (float)accel_bias[2] / (float)accelsensitivity;
}

/*
void mpu9250MagCal(float * dest1, float * dest2)
{
  uint16_t ii = 0, sample_count = 0;
  int32_t mag_bias[3] = {0, 0, 0}, mag_scale[3] = {0, 0, 0};
  int16_t mag_max[3] = {-32767, -32767, -32767}, mag_min[3] = {32767, 32767, 32767}, mag_temp[3] = {0, 0, 0};

  Serial.println("Mag Calibration: Wave device in a figure eight until done!");
  delay(4000);

  // shoot for ~fifteen seconds of mag data
  if(Mmode == 0x02) sample_count = 128;  // at 8 Hz ODR, new mag data is available every 125 ms
  if(Mmode == 0x06) sample_count = 1500;  // at 100 Hz ODR, new mag data is available every 10 ms
  for(ii = 0; ii < sample_count; ii++) {
    readMagData(mag_temp);  // Read the mag data
    for (int jj = 0; jj < 3; jj++) {
      if(mag_temp[jj] > mag_max[jj]) mag_max[jj] = mag_temp[jj];
      if(mag_temp[jj] < mag_min[jj]) mag_min[jj] = mag_temp[jj];
    }
    if(Mmode == 0x02) delay(135);  // at 8 Hz ODR, new mag data is available every 125 ms
    if(Mmode == 0x06) delay(12);  // at 100 Hz ODR, new mag data is available every 10 ms
  }

  //Serial.println("mag x min/max:"); Serial.println(mag_max[0]); Serial.println(mag_min[0]);
  //Serial.println("mag y min/max:"); Serial.println(mag_max[1]); Serial.println(mag_min[1]);
  //Serial.println("mag z min/max:"); Serial.println(mag_max[2]); Serial.println(mag_min[2]);

  // Get hard iron correction
  mag_bias[0]  = (mag_max[0] + mag_min[0])/2;  // get average x mag bias in counts
  mag_bias[1]  = (mag_max[1] + mag_min[1])/2;  // get average y mag bias in counts
  mag_bias[2]  = (mag_max[2] + mag_min[2])/2;  // get average z mag bias in counts

  dest1[0] = (float) mag_bias[0] * mRes * mCal[0];  // save mag biases in G for main program
  dest1[1] = (float) mag_bias[1] * mRes * mCal[1];
  dest1[2] = (float) mag_bias[2] * mRes * mCal[2];

  // Get soft iron correction estimate
  mag_scale[0]  = (mag_max[0] - mag_min[0])/2;  // get average x axis max chord length in counts
  mag_scale[1]  = (mag_max[1] - mag_min[1])/2;  // get average y axis max chord length in counts
  mag_scale[2]  = (mag_max[2] - mag_min[2])/2;  // get average z axis max chord length in counts

  float avg_rad = mag_scale[0] + mag_scale[1] + mag_scale[2];
  avg_rad /= 3.0;

  dest2[0] = avg_rad/((float)mag_scale[0]);
  dest2[1] = avg_rad/((float)mag_scale[1]);
  dest2[2] = avg_rad/((float)mag_scale[2]);

  Serial.println("Mag Calibration done!");
}
*/

// Accelerometer and gyroscope self test; check calibration wrt factory settings
// Should return percent deviation from factory trim values, +/- 14 or less deviation is a pass
void mpu9250SelfTest(float *destination)
{
  uint8_t rawData[6] = {0, 0, 0, 0, 0, 0};
  uint8_t selfTest[6];
  int32_t gAvg[3] = {0}, aAvg[3] = {0}, aSTAvg[3] = {0}, gSTAvg[3] = {0};
  float factoryTrim[6];
  uint8_t FS = 0;

  I2Cwrite(MPU9250_ADDRESS, SMPLRT_DIV, 0x00);      // Set gyro sample rate to 1 kHz
  I2Cwrite(MPU9250_ADDRESS, CONFIG, 0x02);          // Set gyro sample rate to 1 kHz and DLPF to 92 Hz
  I2Cwrite(MPU9250_ADDRESS, GYRO_CONFIG, FS << 3);  // Set full scale range for the gyro to 250 dps
  I2Cwrite(MPU9250_ADDRESS, ACCEL_CONFIG2, 0x02);   // Set accelerometer rate to 1 kHz and bandwidth to 92 Hz
  I2Cwrite(MPU9250_ADDRESS, ACCEL_CONFIG, FS << 3); // Set full scale range for the accelerometer to 2 g

  for (int ii = 0; ii < 200; ii++)
  {                                                                // get average current values of gyro and acclerometer
    I2Cread(MPU9250_ADDRESS, ACCEL_XOUT_H, 6, &rawData[0]);        // Read the six raw data registers into data array
    aAvg[0] += (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]); // Turn the MSB and LSB into a signed 16-bit value
    aAvg[1] += (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]);
    aAvg[2] += (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]);

    I2Cread(MPU9250_ADDRESS, GYRO_XOUT_H, 6, &rawData[0]);         // Read the six raw data registers sequentially into data array
    gAvg[0] += (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]); // Turn the MSB and LSB into a signed 16-bit value
    gAvg[1] += (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]);
    gAvg[2] += (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]);
  }

  for (int ii = 0; ii < 3; ii++)
  { // Get average of 200 values and store as average current readings
    aAvg[ii] /= 200;
    gAvg[ii] /= 200;
  }

  // Configure the accelerometer for self-test
  I2Cwrite(MPU9250_ADDRESS, ACCEL_CONFIG, 0xE0); // Enable self test on all three axes and set accelerometer range to +/- 2 g
  I2Cwrite(MPU9250_ADDRESS, GYRO_CONFIG, 0xE0);  // Enable self test on all three axes and set gyro range to +/- 250 degrees/s
  delay(25);                                     // Delay a while to let the device stabilize

  for (int ii = 0; ii < 200; ii++)
  {                                                                  // get average self-test values of gyro and acclerometer
    I2Cread(MPU9250_ADDRESS, ACCEL_XOUT_H, 6, &rawData[0]);          // Read the six raw data registers into data array
    aSTAvg[0] += (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]); // Turn the MSB and LSB into a signed 16-bit value
    aSTAvg[1] += (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]);
    aSTAvg[2] += (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]);

    I2Cread(MPU9250_ADDRESS, GYRO_XOUT_H, 6, &rawData[0]);           // Read the six raw data registers sequentially into data array
    gSTAvg[0] += (int16_t)(((int16_t)rawData[0] << 8) | rawData[1]); // Turn the MSB and LSB into a signed 16-bit value
    gSTAvg[1] += (int16_t)(((int16_t)rawData[2] << 8) | rawData[3]);
    gSTAvg[2] += (int16_t)(((int16_t)rawData[4] << 8) | rawData[5]);
  }

  for (int ii = 0; ii < 3; ii++)
  { // Get average of 200 values and store as average self-test readings
    aSTAvg[ii] /= 200;
    gSTAvg[ii] /= 200;
  }

  // Configure the gyro and accelerometer for normal operation
  I2Cwrite(MPU9250_ADDRESS, ACCEL_CONFIG, 0x00);
  I2Cwrite(MPU9250_ADDRESS, GYRO_CONFIG, 0x00);
  delay(25); // Delay a while to let the device stabilize

  // Retrieve accelerometer and gyro factory Self-Test Code from USR_Reg
  I2Cread(MPU9250_ADDRESS, SELF_TEST_X_ACCEL, 1, &rawData[0]);
  selfTest[0] = rawData[0]; // X-axis accel self-test results
  I2Cread(MPU9250_ADDRESS, SELF_TEST_Y_ACCEL, 1, &rawData[0]);
  selfTest[1] = rawData[0]; // Y-axis accel self-test results
  I2Cread(MPU9250_ADDRESS, SELF_TEST_Z_ACCEL, 1, &rawData[0]);
  selfTest[2] = rawData[0]; // Z-axis accel self-test results
  I2Cread(MPU9250_ADDRESS, SELF_TEST_X_GYRO, 1, &rawData[0]);
  selfTest[3] = rawData[0]; // X-axis gyro self-test results
  I2Cread(MPU9250_ADDRESS, SELF_TEST_Y_GYRO, 1, &rawData[0]);
  selfTest[4] = rawData[0]; // Y-axis gyro self-test results
  I2Cread(MPU9250_ADDRESS, SELF_TEST_Z_GYRO, 1, &rawData[0]);
  selfTest[5] = rawData[0]; // Z-axis gyro self-test results

  // Retrieve factory self-test value from self-test code reads
  factoryTrim[0] = (float)(2620 / 1 << FS) * (pow(1.01, ((float)selfTest[0] - 1.0))); // FT[Xa] factory trim calculation
  factoryTrim[1] = (float)(2620 / 1 << FS) * (pow(1.01, ((float)selfTest[1] - 1.0))); // FT[Ya] factory trim calculation
  factoryTrim[2] = (float)(2620 / 1 << FS) * (pow(1.01, ((float)selfTest[2] - 1.0))); // FT[Za] factory trim calculation
  factoryTrim[3] = (float)(2620 / 1 << FS) * (pow(1.01, ((float)selfTest[3] - 1.0))); // FT[Xg] factory trim calculation
  factoryTrim[4] = (float)(2620 / 1 << FS) * (pow(1.01, ((float)selfTest[4] - 1.0))); // FT[Yg] factory trim calculation
  factoryTrim[5] = (float)(2620 / 1 << FS) * (pow(1.01, ((float)selfTest[5] - 1.0))); // FT[Zg] factory trim calculation

  // Report results as a ratio of (STR - FT)/FT; the change from Factory Trim of the Self-Test Response
  // To get percent, must multiply by 100
  for (int i = 0; i < 3; i++)
  {
    destination[i] = 100.0 * ((float)(aSTAvg[i] - aAvg[i])) / factoryTrim[i] - 100.;         // Report percent differences
    destination[i + 3] = 100.0 * ((float)(gSTAvg[i] - gAvg[i])) / factoryTrim[i + 3] - 100.; // Report percent differences
  }
}

void ak8963Init(float *destination)
{
  // First extract the factory calibration for each magnetometer axis
  uint8_t rawData[3];                          // x/y/z gyro calibration data stored here
  I2Cwrite(AK8963_ADDRESS, AK8963_CNTL, 0x00); // Power down magnetometer
  delay(10);
  I2Cwrite(AK8963_ADDRESS, AK8963_CNTL, 0x0F); // Enter Fuse ROM access mode
  delay(10);
  I2Cread(AK8963_ADDRESS, AK8963_ASAX, 3, &rawData[0]);     // Read the x-, y-, and z-axis calibration values
  destination[0] = ((float)rawData[0] - 128) / 256.0 + 1.0; // Return x-axis sensitivity adjustment values, etc.
  destination[1] = ((float)rawData[1] - 128) / 256.0 + 1.0;
  destination[2] = ((float)rawData[2] - 128) / 256.0 + 1.0;
  I2Cwrite(AK8963_ADDRESS, AK8963_CNTL, 0x00); // Power down magnetometer
  delay(10);
  // Configure the magnetometer for continuous read and highest resolution
  // set Mscale bit 4 to 1 (0) to enable 16 (14) bit resolution in CNTL register,
  // and enable continuous mode data acquisition Mmode (bits [3:0]), 0010 for 8 Hz and 0110 for 100 Hz sample rates
  I2Cwrite(AK8963_ADDRESS, AK8963_CNTL, Mscale << 4 | Mmode); // Set magnetometer data resolution and sample ODR
  delay(10);
}

// -------------------------------------------------------------------------------------------------------------------------------------

void MadgwickQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz, float deltat)
{
  float q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3]; // short name local variable for readability
  float norm;
  float hx, hy, _2bx, _2bz;
  float s1, s2, s3, s4;
  float qDot1, qDot2, qDot3, qDot4;

  // Auxiliary variables to avoid repeated arithmetic
  float _2q1mx;
  float _2q1my;
  float _2q1mz;
  float _2q2mx;
  float _4bx;
  float _4bz;
  float _2q1 = 2.0f * q1;
  float _2q2 = 2.0f * q2;
  float _2q3 = 2.0f * q3;
  float _2q4 = 2.0f * q4;
  float _2q1q3 = 2.0f * q1 * q3;
  float _2q3q4 = 2.0f * q3 * q4;
  float q1q1 = q1 * q1;
  float q1q2 = q1 * q2;
  float q1q3 = q1 * q3;
  float q1q4 = q1 * q4;
  float q2q2 = q2 * q2;
  float q2q3 = q2 * q3;
  float q2q4 = q2 * q4;
  float q3q3 = q3 * q3;
  float q3q4 = q3 * q4;
  float q4q4 = q4 * q4;

  /* changed for magneto I guess
  // Normalise accelerometer measurement
  norm = sqrt(ax * ax + ay * ay + az * az);
  if (norm == 0.0f) return; // handle NaN
  norm = 1.0f / norm;
  ax *= norm;
  ay *= norm;
  az *= norm;

  // Normalise magnetometer measurement
  norm = sqrt(mx * mx + my * my + mz * mz);
  if (norm == 0.0f) return; // handle NaN
  norm = 1.0f / norm;
  mx *= norm;
  my *= norm;
  mz *= norm;
  */

  // Reference direction of Earth's magnetic field
  _2q1mx = 2.0f * q1 * mx;
  _2q1my = 2.0f * q1 * my;
  _2q1mz = 2.0f * q1 * mz;
  _2q2mx = 2.0f * q2 * mx;
  hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
  hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
  _2bx = sqrt(hx * hx + hy * hy);
  _2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
  _4bx = 2.0f * _2bx;
  _4bz = 2.0f * _2bz;

  // Gradient decent algorithm corrective step
  s1 = -_2q3 * (2.0f * q2q4 - _2q1q3 - ax) + _2q2 * (2.0f * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
  s2 = _2q4 * (2.0f * q2q4 - _2q1q3 - ax) + _2q1 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q2 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + _2bz * q4 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
  s3 = -_2q1 * (2.0f * q2q4 - _2q1q3 - ax) + _2q4 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q3 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
  s4 = _2q2 * (2.0f * q2q4 - _2q1q3 - ax) + _2q3 * (2.0f * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
  norm = sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4); // normalise step magnitude
  norm = 1.0f / norm;
  s1 *= norm;
  s2 *= norm;
  s3 *= norm;
  s4 *= norm;

  // Compute rate of change of quaternion
  qDot1 = 0.5f * (-q2 * gx - q3 * gy - q4 * gz) - beta * s1;
  qDot2 = 0.5f * (q1 * gx + q3 * gz - q4 * gy) - beta * s2;
  qDot3 = 0.5f * (q1 * gy - q2 * gz + q4 * gx) - beta * s3;
  qDot4 = 0.5f * (q1 * gz + q2 * gy - q3 * gx) - beta * s4;

  // Integrate to yield quaternion
  q1 += qDot1 * deltat;
  q2 += qDot2 * deltat;
  q3 += qDot3 * deltat;
  q4 += qDot4 * deltat;
  norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4); // normalise quaternion
  norm = 1.0f / norm;
  q[0] = q1 * norm;
  q[1] = q2 * norm;
  q[2] = q3 * norm;
  q[3] = q4 * norm;
}
// Similar to Madgwick scheme but uses proportional and integral filtering on the error between estimated reference vectors and
// measured ones.
void MahonyQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz, float deltat)
{
  float q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3]; // short name local variable for readability
  float norm;
  float hx, hy, bx, bz;
  float vx, vy, vz, wx, wy, wz;
  float ex, ey, ez;
  float pa, pb, pc;

  // Auxiliary variables to avoid repeated arithmetic
  float q1q1 = q1 * q1;
  float q1q2 = q1 * q2;
  float q1q3 = q1 * q3;
  float q1q4 = q1 * q4;
  float q2q2 = q2 * q2;
  float q2q3 = q2 * q3;
  float q2q4 = q2 * q4;
  float q3q3 = q3 * q3;
  float q3q4 = q3 * q4;
  float q4q4 = q4 * q4;

  // Normalise accelerometer measurement
  norm = sqrt(ax * ax + ay * ay + az * az);
  if (norm == 0.0f)
    return;           // handle NaN
  norm = 1.0f / norm; // use reciprocal for division
  ax *= norm;
  ay *= norm;
  az *= norm;

  // Normalise magnetometer measurement
  norm = sqrt(mx * mx + my * my + mz * mz);
  if (norm == 0.0f)
    return;           // handle NaN
  norm = 1.0f / norm; // use reciprocal for division
  mx *= norm;
  my *= norm;
  mz *= norm;

  // Reference direction of Earth's magnetic field
  hx = 2.0f * mx * (0.5f - q3q3 - q4q4) + 2.0f * my * (q2q3 - q1q4) + 2.0f * mz * (q2q4 + q1q3);
  hy = 2.0f * mx * (q2q3 + q1q4) + 2.0f * my * (0.5f - q2q2 - q4q4) + 2.0f * mz * (q3q4 - q1q2);
  bx = sqrt((hx * hx) + (hy * hy));
  bz = 2.0f * mx * (q2q4 - q1q3) + 2.0f * my * (q3q4 + q1q2) + 2.0f * mz * (0.5f - q2q2 - q3q3);

  // Estimated direction of gravity and magnetic field
  vx = 2.0f * (q2q4 - q1q3);
  vy = 2.0f * (q1q2 + q3q4);
  vz = q1q1 - q2q2 - q3q3 + q4q4;
  wx = 2.0f * bx * (0.5f - q3q3 - q4q4) + 2.0f * bz * (q2q4 - q1q3);
  wy = 2.0f * bx * (q2q3 - q1q4) + 2.0f * bz * (q1q2 + q3q4);
  wz = 2.0f * bx * (q1q3 + q2q4) + 2.0f * bz * (0.5f - q2q2 - q3q3);

  // Error is cross product between estimated direction and measured direction of gravity
  ex = (ay * vz - az * vy) + (my * wz - mz * wy);
  ey = (az * vx - ax * vz) + (mz * wx - mx * wz);
  ez = (ax * vy - ay * vx) + (mx * wy - my * wx);
  if (Ki > 0.0f)
  {
    eInt[0] += ex; // accumulate integral error
    eInt[1] += ey;
    eInt[2] += ez;
  }
  else
  {
    eInt[0] = 0.0f; // prevent integral wind up
    eInt[1] = 0.0f;
    eInt[2] = 0.0f;
  }

  // Apply feedback terms
  gx = gx + Kp * ex + Ki * eInt[0];
  gy = gy + Kp * ey + Ki * eInt[1];
  gz = gz + Kp * ez + Ki * eInt[2];

  // Integrate rate of change of quaternion
  pa = q2;
  pb = q3;
  pc = q4;
  q1 = q1 + (-q2 * gx - q3 * gy - q4 * gz) * (0.5f * deltat);
  q2 = pa + (q1 * gx + pb * gz - pc * gy) * (0.5f * deltat);
  q3 = pb + (q1 * gy - pa * gz + pc * gx) * (0.5f * deltat);
  q4 = pc + (q1 * gz + pa * gy - pb * gx) * (0.5f * deltat);

  // Normalise quaternion
  norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);
  norm = 1.0f / norm;
  q[0] = q1 * norm;
  q[1] = q2 * norm;
  q[2] = q3 * norm;
  q[3] = q4 * norm;
}

void MahonyQuaternionUpdate2(float ax, float ay, float az, float gx, float gy, float gz)
{
  float norm;
  float vx, vy, vz;
  float ex, ey, ez;
  float qa, qb, qc;

  // compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
  if (!((ax == 0.0) && (ay == 0.0) && (az == 0.0)))
  {
    // normalise accelerometer measurement
    norm = sqrt(ax * ax + ay * ay + az * az);
    ax /= norm;
    ay /= norm;
    az /= norm;

    // Estimated direction of gravity and vector perpendicular to magnetic flux
    vx = q[1] * q[3] - q[0] * q[2];
    vy = q[0] * q[1] + q[2] * q[3];
    vz = q[0] * q[0] - 0.5 + q[3] * q[3];

    // Error is sum of cross product between estimated and measured direction of gravity
    ex = (ay * vz - az * vy);
    ey = (az * vx - ax * vz);
    ez = (ax * vy - ay * vx);

    // Compute and apply integral feedback if enabled
    if (Ki > 0.0f)
    {
      eInt[0] += Ki * ex * (1.0 / deltat); // integral error scaled by Ki
      eInt[1] += Ki * ey * (1.0 / deltat);
      eInt[2] += Ki * ez * (1.0 / deltat);
      gx += eInt[0]; // apply integral feedback
      gy += eInt[1];
      gz += eInt[2];
    }
    else
    {
      eInt[0] = 0.0; // prevent integral windup
      eInt[1] = 0.0;
      eInt[2] = 0.0;
    }

    // Apply proportional feedback
    gx += Kp * ex;
    gy += Kp * ey;
    gz += Kp * ez;
  }

  // Integrate rate of change of quaternion
  gx *= (0.5 * (1.0 / deltat)); // pre-multiply common factors
  gy *= (0.5 * (1.0 / deltat));
  gz *= (0.5 * (1.0 / deltat));
  qa = q[0];
  qb = q[1];
  qc = q[2];
  q[0] += (-qb * gx - qc * gy - q[3] * gz);
  q[1] += (qa * gx + qc * gz - q[3] * gy);
  q[2] += (qa * gy - qb * gz + q[3] * gx);
  q[3] += (qa * gz + qb * gy - qc * gx);

  // Normalise quaternion
  norm = sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
  q[0] /= norm;
  q[1] /= norm;
  q[2] /= norm;
  q[3] /= norm;
}

void magnetoSamples(int samples)
{
  Serial.println("Gyro bias collection ... KEEP SENSOR STILL");
  for (int i = 0; i < samples; i++)
  { // 300 samples
    readGyroData(gRaw);
    Gxyz[0] += (float)gRaw[0]; // 250 LSB(d/s) default to radians/s
    Gxyz[1] += (float)gRaw[1];
    Gxyz[2] += (float)gRaw[2];
  }
  G_off[0] = Gxyz[0] / samples;
  G_off[1] = Gxyz[1] / samples;
  G_off[2] = Gxyz[2] / samples;
  Serial.print("Done. Gyro offsets (raw) ");
  Serial.print(G_off[0], 1);
  Serial.print(", ");
  Serial.print(G_off[1], 1);
  Serial.print(", ");
  Serial.print(G_off[2], 1);
  Serial.println();

  Serial.print("Collecting ");
  Serial.print(samples);
  Serial.println(" points for scaling, 3/second");
  Serial.println("TURN SENSOR VERY SLOWLY AND CAREFULLY IN 3D");

  float M_mag = 0, A_mag = 0;
  int i, j;
  j = samples;
  while (j-- >= 0)
  {
    readAccelData(aRaw);
    Axyz[0] = (float)aRaw[0];
    Axyz[1] = (float)aRaw[1];
    Axyz[2] = (float)aRaw[2];
    readMagData(mRaw);
    Mxyz[0] = (float)mRaw[0];
    Mxyz[1] = (float)mRaw[1];
    Mxyz[2] = (float)mRaw[2];

    for (i = 0; i < 3; i++)
    {
      M_mag += Mxyz[i] * Mxyz[i];
      A_mag += Axyz[i] * Axyz[i];
    }
    Serial.print(aRaw[0]);
    Serial.print(",");
    Serial.print(aRaw[1]);
    Serial.print(",");
    Serial.print(aRaw[2]);
    Serial.print(",");
    Serial.print(mRaw[0]);
    Serial.print(",");
    Serial.print(mRaw[1]);
    Serial.print(",");
    Serial.println(mRaw[2]);
    delay(300);
  }
  Serial.print("Done. ");
  Serial.print("rms A = ");
  Serial.print(sqrt(A_mag / samples));
  Serial.print(", rms M = ");
  Serial.println(sqrt(M_mag / samples));
}

void calibrateGyro(int samples)
{
  Serial.println("Gyro bias collection ... KEEP SENSOR STILL");
  for (int i = 0; i < samples; i++)
  { // 500 samples
    readGyroData(gRaw);
    Gxyz[0] += (float)gRaw[0]; // 250 LSB(d/s) default to radians/s
    Gxyz[1] += (float)gRaw[1];
    Gxyz[2] += (float)gRaw[2];
  }
  G_off[0] = Gxyz[0] / samples;
  G_off[1] = Gxyz[1] / samples;
  G_off[2] = Gxyz[2] / samples;
  Serial.print("Done. Gyro offsets (raw) ");
  Serial.print(G_off[0], 1);
  Serial.print(", ");
  Serial.print(G_off[1], 1);
  Serial.print(", ");
  Serial.print(G_off[2], 1);
  Serial.println();
}

void mpu9250Setup()
{
  uint8_t Buf[1] = {0};

  Serial.print("Starting MPU9250 ... ");
  Wire.begin();
  I2Cread(MPU9250_ADDRESS, WHO_AM_I_MPU9250, 1, Buf);
  if (Buf[0] == 0x71)
  {
    // mpu9250_found = 1;

    // set conversion res
    getAres();
    getGres();
    getMres();

    Serial.println("Success");

    I2Cwrite(MPU9250_ADDRESS, PWR_MGMT_1, 0x80); // Set bit 7 to reset MPU9250
    delay(100);

    // mpu9250SelfTest(SelfTest); // Start by performing self test and reporting values
    // Serial.print("x-axis self test: acceleration trim within : "); Serial.print(SelfTest[0], 1); Serial.println("% of factory value");
    // Serial.print("y-axis self test: acceleration trim within : "); Serial.print(SelfTest[1], 1); Serial.println("% of factory value");
    // Serial.print("z-axis self test: acceleration trim within : "); Serial.print(SelfTest[2], 1); Serial.println("% of factory value");
    // Serial.print("x-axis self test: gyration trim within : "); Serial.print(SelfTest[3], 1); Serial.println("% of factory value");
    // Serial.print("y-axis self test: gyration trim within : "); Serial.print(SelfTest[4], 1); Serial.println("% of factory value");
    // Serial.print("z-axis self test: gyration trim within : "); Serial.print(SelfTest[5], 1); Serial.println("% of factory value");

    // Comment out if using pre-measured, pre-stored offset biases
    // mpu9250Calibrate(gBias, aBias); // Calibrate gyro and accelerometers, load biases in bias registers
    // Serial.println("accel biases (mg)"); Serial.println(1000.0 * aBias[0]); Serial.println(1000.0 * aBias[1]); Serial.println(1000.0 * aBias[2]);
    // Serial.println("gyro biases (dps)"); Serial.println(gBias[0]); Serial.println(gBias[1]); Serial.println(gBias[2]);
    // delay(1000);

    mpu9250Init();
    // Serial.println("MPU9250 initialized for active data mode...."); // Initialize device for active mode read of acclerometer, gyroscope, and temperature

    Serial.print("Starting AK8963 ... ");
    I2Cread(AK8963_ADDRESS, WHO_AM_I_AK8963, 1, Buf);
    if (Buf[0] == 0x48)
    {
      // ak8963_found = 1;
      Serial.println("Success");
      delay(10);
      ak8963Init(M_coeffs);

      // Serial.println("AK8963 initialized for active data mode...."); // Initialize device for active mode read of magnetometer
      // I2Cwrite(AK8963_ADDRESS, AK8963_CNTL, 0x01); //enable the magnetometer

      // magnetoSamples(300);
      //  Comment out if using pre-measured, pre-stored offset biases
      // mpu9250MagCal(mBias, mScale);
      // Serial.println("AK8963 mag biases (mG)"); Serial.println(mBias[0]); Serial.println(mBias[1]); Serial.println(mBias[2]);
      // Serial.println("AK8963 mag scale (mG)"); Serial.println(mScale[0]); Serial.println(mScale[1]); Serial.println(mScale[2]);
      // delay(2000); // add delay to see results before serial spew of data

      // calibrateGyro(500);

      //  Serial.println("Calibration values: ");
      // Serial.print("X-Axis sensitivity adjustment value "); Serial.println(mCal[0], 2);
      // Serial.print("Y-Axis sensitivity adjustment value "); Serial.println(mCal[1], 2);
      // Serial.print("Z-Axis sensitivity adjustment value "); Serial.println(mCal[2], 2);
    }
    else
    {
      Serial.println("Failed!");
    }
  }
  else
  {
    Serial.println("Failed!");
  }

  // I2Cwrite(MPU9250_ADDRESS, 28, ACC_FULL_SCALE_16_G);
  // I2Cwrite(MPU9250_ADDRESS, 27, GYRO_FULL_SCALE_2000_DPS);

  // I2Cwrite(MPU9250_ADDRESS, 0x37, 0x02);
  // I2Cwrite(MPU9250_ADDRESS, 0x0A, 0x01); // single measurement?
  now = micros();
}

void mpu9250Loop()
{
  // float temp[3];

  uint8_t Buf[14] = {0};
  I2Cread(MPU9250_ADDRESS, INT_STATUS, 1, Buf);
  if (Buf[0] & 0x01)
  {
    readAccelData(aRaw);
    Axyz[0] = (float)aRaw[0] * aRes;
    Axyz[1] = (float)aRaw[1] * aRes;
    Axyz[2] = (float)aRaw[2] * aRes;
    // apply offsets and scale factors from Magneto
    //  for (int i = 0; i < 3; i++) Axyz[i] = (Axyz[i] - A_cal[i]) * A_cal[i + 3];
    //  vector_normalize(Axyz);
    // for (int i = 0; i < 3; i++) temp[i] = (Axyz[i] - A_B[i]);
    // Axyz[0] = A_Ainv[0][0] * temp[0] + A_Ainv[0][1] * temp[1] + A_Ainv[0][2] * temp[2];
    // Axyz[1] = A_Ainv[1][0] * temp[0] + A_Ainv[1][1] * temp[1] + A_Ainv[1][2] * temp[2];
    // Axyz[2] = A_Ainv[2][0] * temp[0] + A_Ainv[2][1] * temp[1] + A_Ainv[2][2] * temp[2];
    // vector_normalize(Axyz);

    readGyroData(gRaw);
    Gxyz[0] = (float)gRaw[0] * gRes;
    Gxyz[1] = (float)gRaw[1] * gRes;
    Gxyz[2] = (float)gRaw[2] * gRes;

    readMagData(mRaw);
    // Mxyz[0] = M_coeffs[0] * ((float) mRaw[0] / pow(2.0, 15.0)) * 4800.0; // magnetometer sensitivity: 4800 uT
    // Mxyz[1] = M_coeffs[1] * ((float) mRaw[1] / pow(2.0, 15.0)) * 4800.0;
    // Mxyz[2] = M_coeffs[2] * ((float) mRaw[2] / pow(2.0, 15.0)) * 4800.0;
    Mxyz[0] = (float)mRaw[0] * mRes * M_coeffs[0];
    Mxyz[1] = (float)mRaw[1] * mRes * M_coeffs[1];
    Mxyz[2] = (float)mRaw[2] * mRes * M_coeffs[2];
    // apply offsets and scale factors from Magneto
    //  for (int i = 0; i < 3; i++) Mxyz[i] = (Mxyz[i] - M_cal[i]) * M_cal[i + 3];
    //  vector_normalize(Mxyz);
    // for (int i = 0; i < 3; i++) temp[i] = (Mxyz[i] - M_B[i]);
    // Mxyz[0] = M_Ainv[0][0] * temp[0] + M_Ainv[0][1] * temp[1] + M_Ainv[0][2] * temp[2];
    // Mxyz[1] = M_Ainv[1][0] * temp[0] + M_Ainv[1][1] * temp[1] + M_Ainv[1][2] * temp[2];
    // Mxyz[2] = M_Ainv[2][0] * temp[0] + M_Ainv[2][1] * temp[1] + M_Ainv[2][2] * temp[2];
    // vector_normalize(Mxyz);

    tRaw = readTempData();
    temperature = ((float)tRaw) / 333.87 + 21.0; // Temperature in degrees Centigrade

    // test
    /*
    float bh = sqrt(Mxyz[0] * Mxyz[0] + Mxyz[1] * Mxyz[1]);
    Serial.println("------------------------------------------------------------");
    Serial.println("_____________________________");
    Serial.println("ACCELEROMETER");
    Serial.print("x-dir: ");
    Serial.print(Axyz[0], 2);
    Serial.println(" g");
    Serial.print("y-dir: ");
    Serial.print(Axyz[1], 2);
    Serial.println(" g");
    Serial.print("z-dir: ");
    Serial.print(Axyz[2], 2);
    Serial.println(" g");
    Serial.println("_____________________________");
    Serial.println("GYROSCOPE");
    Serial.print("x-dir: ");
    Serial.print(Gxyz[0], 2);
    Serial.println(" dps");
    Serial.print("y-dir: ");
    Serial.print(Gxyz[1], 2);
    Serial.println(" dps");
    Serial.print("z-dir: ");
    Serial.print(Gxyz[2], 2);
    Serial.println(" dps");
    Serial.println("_____________________________");
    Serial.println("MAGNETOMETER");
    Serial.print("x-dir: ");
    Serial.print(Mxyz[0], 2);
    Serial.println(" uT");
    Serial.print("y-dir: ");
    Serial.print(Mxyz[1], 2);
    Serial.println(" uT");
    Serial.print("z-dir: ");
    Serial.print(Mxyz[2], 2);
    Serial.println(" uT");
    Serial.print("bh: ");
    Serial.print(bh, 2);
    Serial.println(" uT");
    */
  }

  now = micros();
  deltat = ((now - last) / 1000000.0f); // set integration time by time elapsed since last filter update
  last = now;

  // https://github.com/TKJElectronics/KalmanFilter/blob/master/examples/MPU6050/MPU6050.ino
  // #ifdef RESTRICT_PITCH // Eq. 25 and 26
  // roll  = atan2(aRaw[1], aRaw[2]) * RAD_TO_DEG;
  // pitch = atan(-aRaw[0] / sqrt(aRaw[1] * aRaw[1] + aRaw[2] * aRaw[2])) * RAD_TO_DEG;
  roll = atan2(Axyz[1], Axyz[2]);
  pitch = atan(-Axyz[0] / sqrt(Axyz[1] * Axyz[1] + Axyz[2] * Axyz[2]));
  // #else // Eq. 28 and 29
  //   roll  = atan(aRaw[1] / sqrt(aRaw[0] * aRaw[0] + aRaw[2] * aRaw[2])) * RAD_TO_DEG;
  //   pitch = atan2(-aRaw[0], aRaw[2]) * RAD_TO_DEG;
  // #endif

  // MahonyQuaternionUpdate2(Axyz[0], Axyz[1], Axyz[2], Gxyz[0], Gxyz[1], Gxyz[2]);
  // yaw   = -atan2(2.0f * (q[1] * q[2] + q[0] * q[3]), q[0] * q[0] + q[1] * q[1] - q[2] * q[2] - q[3] * q[3]) * 57.29577951;
  // pitch = asin(2.0f * (q[1] * q[3] - q[0] * q[2])) * 57.29577951;
  // roll  = atan2(2.0f * (q[0] * q[1] + q[2] * q[3]), q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3]) * 57.29577951;

  /*
  MahonyQuaternionUpdate(Axyz[0], Axyz[1], Axyz[2], Gxyz[0], Gxyz[1], Gxyz[2], Mxyz[1], Mxyz[0], -Mxyz[2], deltat);
  //MadgwickQuaternionUpdate(AX, AY, AZ, GX*PI/180.0f, GY*PI/180.0f, GZ*PI/180.0f, MY, MX, MZ);
  //MahonyQuaternionUpdate(AX, AY, AZ, GX*PI/180.0f, GY*PI/180.0f, GZ*PI/180.0f, MY, MX, MZ);


  // data proc
  float qw = q[0], qx = q[1], qy = q[2], qz = q[3];

  roll  = atan2(2.0 * (qw * qx + qy * qz), 1.0 - 2.0 * (qx * qx + qy * qy));
  pitch = asin(2.0 * (qw * qy - qx * qz));
  yaw   = atan2(2.0 * (qx * qy + qw * qz), 1.0 - 2.0 * ( qy * qy + qz * qz));
  // to degrees
  yaw   *= 180.0 / PI;
  pitch *= 180.0 / PI;
  roll *= 180.0 / PI;

  // http://www.ngdc.noaa.gov/geomag-web/#declination
  //conventional nav, yaw increases CW from North, corrected for my declination

  yaw = -yaw + declination;
  */
}
