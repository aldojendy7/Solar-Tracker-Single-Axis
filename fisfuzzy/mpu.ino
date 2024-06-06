#include <MPU6050_tockn.h>
#include <Wire.h>

MPU6050 mpu6050(Wire);

// setup the MPU6050 to SDA SCL communication
void mpu_setup() {
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
}

// Get MPU6050 data with complementary filter
float get_x() {
  float data = mpu6050.getAngleY();
  return data;
}

void get_mpu(){
  mpu6050.update();
}
