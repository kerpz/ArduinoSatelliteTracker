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
  Wire.endTransmission(true);
  mpu9250_found = 1;
}

void mpu6050Loop() {
  float AX, AY, AZ, TP, GX, GY, GZ;
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);

  AX = Wire.read()<<8|Wire.read();
  AY = Wire.read()<<8|Wire.read();
  AZ = Wire.read()<<8|Wire.read();
  TP = Wire.read()<<8|Wire.read();
  GX = Wire.read()<<8|Wire.read();
  GY = Wire.read()<<8|Wire.read();
  GZ = Wire.read()<<8|Wire.read();
  
  temperature = TP / 340.00 + 36.53;

  int xAng = map(AX, minVal, maxVal, -90, 90);
  int yAng = map(AY, minVal, maxVal, -90, 90);
  int zAng = map(AZ, minVal, maxVal, -90, 90);

  yaw = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
  pitch = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
  roll = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);
//pitch = 180 * atan2(accelX, sqrt(accelY*accelY + accelZ*accelZ))/PI;
//roll = 180 * atan2(accelY, sqrt(accelX*accelX + accelZ*accelZ))/PI;
// https://roboticsclubiitk.github.io/2017/12/21/Beginners-Guide-to-IMU.html
// 
}
