/*
  开发板：ESP32-S3-N16R8 DevKitC-1
  环境：PlatformIO + Arduino
  库：Wire
  引脚：SDA=GPIO8，SCL=GPIO9
  功能：扫描 I2C 总线，打印所有设备地址
*/

#include <Arduino.h>
#include <Wire.h>

#define I2C_SDA 8
#define I2C_SCL 9

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("I2C scan start...");

  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      found++;
    }
  }

  if (found == 0) {
    Serial.println("No I2C device found");
  } else {
    Serial.print("Total: ");
    Serial.println(found);
  }
}

void loop() {
}