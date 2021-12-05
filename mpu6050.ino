//pitch = 180 * atan2(accelX, sqrt(accelY*accelY + accelZ*accelZ))/PI;
//roll = 180 * atan2(accelY, sqrt(accelX*accelX + accelZ*accelZ))/PI;
// https://roboticsclubiitk.github.io/2017/12/21/Beginners-Guide-to-IMU.html

// D1 = SCL
// D2 = SDA
const int MPU_addr = 0x68;
const int minVal = 265;
const int maxVal = 402;

void mpu6050Setup() {
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  if (Wire.endTransmission() != 0) mpu_error++;
}

void mpu6050Loop() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  if (Wire.endTransmission() == 0) {
    Wire.requestFrom(MPU_addr, 14);
  
    int16_t AcX = Wire.read()<<8|Wire.read();
    int16_t AcY = Wire.read()<<8|Wire.read();
    int16_t AcZ = Wire.read()<<8|Wire.read();
    int16_t Tmp = Wire.read()<<8|Wire.read();
    int16_t GyX = Wire.read()<<8|Wire.read();
    int16_t GyY = Wire.read()<<8|Wire.read();
    int16_t GyZ = Wire.read()<<8|Wire.read();
    
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
  else {
    mpu_error++;
  }
}
