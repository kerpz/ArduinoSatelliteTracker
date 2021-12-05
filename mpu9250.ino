// D1 = SCL
// D2 = SDA
#define MPU_addr 0x68
#define MAG_addr 0x0C

#define GYRO_FULL_SCALE_250_DPS 0x00
#define GYRO_FULL_SCALE_500_DPS 0x08
#define GYRO_FULL_SCALE_1000_DPS 0x10
#define GYRO_FULL_SCALE_2000_DPS 0x18

#define ACC_FULL_SCALE_2_G 0x00
#define ACC_FULL_SCALE_4_G 0x08
#define ACC_FULL_SCALE_8_G 0x10
#define ACC_FULL_SCALE_16_G 0x18

#define minVal 265
#define maxVal 402

// This function read Nbytes bytes from I2C device at address Address. 
// Put read bytes starting at register Register in the Data array. 
void I2Cread(uint8_t Address, uint8_t Register, uint8_t Nbytes, uint8_t *Data)
{
  // Set register address
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.endTransmission();

  // Read Nbytes
  Wire.requestFrom(Address, Nbytes); 
  uint8_t index=0;
  while (Wire.available())
  Data[index++]=Wire.read();
}
 
 
// Write a byte (Data) in device (Address) at register (Register)
void I2CwriteByte(uint8_t Address, uint8_t Register, uint8_t Data)
{
  // Set register address
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.write(Data);
  Wire.endTransmission();
}

void mpu9250Setup() {
  Wire.begin();
  I2CwriteByte(MPU_addr, 0x6B, 0x00);
  // Set accelerometers low pass filter at 5Hz
  //I2CwriteByte(MPU_addr, 29, 0x06);
  // Set gyroscope low pass filter at 5Hz
  //I2CwriteByte(MPU_addr, 26, 0x06);

  // Configure gyroscope range
  //I2CwriteByte(MPU_addr, 27, GYRO_FULL_SCALE_1000_DPS);
  // Configure accelerometers range
  //I2CwriteByte(MPU_addr, 28, ACC_FULL_SCALE_4_G);
  // Set by pass mode for the magnetometers
  //I2CwriteByte(MPU_addr, 0x37, 0x02);

  // Request continuous magnetometer measurements in 16 bits
  //I2CwriteByte(MAG_addr, 0x0A, 0x16);
  /*
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  */
}

void mpu9250Loop() {
  /*
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);

  int16_t AcX = Wire.read()<<8|Wire.read();
  int16_t AcY = Wire.read()<<8|Wire.read();
  int16_t AcZ = Wire.read()<<8|Wire.read();
  int16_t Tmp = Wire.read()<<8|Wire.read();
  int16_t GyX = Wire.read()<<8|Wire.read();
  int16_t GyY = Wire.read()<<8|Wire.read();
  int16_t GyZ = Wire.read()<<8|Wire.read();
  */

  // Read accelerometer and gyroscope
  uint8_t Buf[14];
  I2Cread(MPU_addr, 0x3B, 14, Buf);
   
  int16_t AcX = Buf[0]<<8|Buf[1];
  int16_t AcY = Buf[2]<<8|Buf[3];
  int16_t AcZ = Buf[4]<<8|Buf[5];
  int16_t Tmp = Buf[6]<<8|Buf[7];
  int16_t GyX = Buf[8]<<8|Buf[9];
  int16_t GyY = Buf[10]<<8|Buf[11];
  int16_t GyZ = Buf[12]<<8|Buf[13];

  // mpu6050
  //temperature = ((float)Tmp) / 340.0 + 36.53;
  //temperature = ((double)Tmp + 12412.0) / 340.0;
  // mpu9250
  temperature = ((float) Tmp) / 333.87 + 21.0;
  

  int xAng = map(AcX, minVal, maxVal, -90, 90);
  int yAng = map(AcY, minVal, maxVal, -90, 90);
  int zAng = map(AcZ, minVal, maxVal, -90, 90);

  x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
  y = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
  z = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);
  //x = atan2(xAng, yAng) * 180 / PI;
}
