/*
  开发板：ESP32-S3-N16R8 DevKitC-1
  环境：PlatformIO + Arduino
  库：Wire、Adafruit_MPU6050、Adafruit_Sensor
  引脚：MPU6050 VCC->3.3V，GND->GND，SDA->GPIO8，SCL->GPIO9
  功能：MPU6050 陀螺仪零偏校准 + micros() 定时 100Hz + CSV 输出
  CSV 列：t_us,ax_g,ay_g,az_g,gx_rps,gy_rps,gz_rps
*/

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define I2C_SDA 8
#define I2C_SCL 9
#define MPU_ADDR 0x68

#define SAMPLE_INTERVAL_US 10000  // 100Hz = 10000us

Adafruit_MPU6050 mpu;

float gyroBiasX = 0;
float gyroBiasY = 0;
float gyroBiasZ = 0;

uint32_t nextSampleUs = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (!mpu.begin(MPU_ADDR, &Wire)) {
    Serial.println("# ERROR: MPU6050 NOT FOUND");
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("# CALIBRATING: keep still");

  const int N = 500;
  for (int i = 0; i < N; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    gyroBiasX += g.gyro.x;
    gyroBiasY += g.gyro.y;
    gyroBiasZ += g.gyro.z;

    delay(10);
  }

  gyroBiasX /= N;
  gyroBiasY /= N;
  gyroBiasZ /= N;

  Serial.println("# CALIBRATION DONE");
  Serial.println("# t_us,ax_g,ay_g,az_g,gx_rps,gy_rps,gz_rps");

  nextSampleUs = micros();
}

void loop() {
  uint32_t nowUs = micros();

  if ((int32_t)(nowUs - nextSampleUs) >= 0) {
    nextSampleUs += SAMPLE_INTERVAL_US;

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float ax = a.acceleration.x / 9.80665;
    float ay = a.acceleration.y / 9.80665;
    float az = a.acceleration.z / 9.80665;

    float gx = g.gyro.x - gyroBiasX;
    float gy = g.gyro.y - gyroBiasY;
    float gz = g.gyro.z - gyroBiasZ;

    Serial.print(nowUs);
    Serial.print(",");
    Serial.print(ax, 4);
    Serial.print(",");
    Serial.print(ay, 4);
    Serial.print(",");
    Serial.print(az, 4);
    Serial.print(",");
    Serial.print(gx, 4);
    Serial.print(",");
    Serial.print(gy, 4);
    Serial.print(",");
    Serial.println(gz, 4);
  }
}