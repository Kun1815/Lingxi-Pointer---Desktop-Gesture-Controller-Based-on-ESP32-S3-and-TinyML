/*
  开发板：ESP32-S3-N16R8 DevKitC-1
  环境：PlatformIO + Arduino
  库：Adafruit_SSD1306、Adafruit_GFX
  引脚：OLED SDA=GPIO8，SCL=GPIO9，地址=0x3C
  功能：OLED 显示固定文字，验证驱动正常
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define I2C_SDA 8
#define I2C_SCL 9
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 init failed");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("LingXi Finger");

  display.setCursor(0, 16);
  display.setTextSize(2);
  display.println("OLED OK");

  display.setCursor(0, 48);
  display.setTextSize(1);
  display.println("SDA=8 SCL=9");

  display.display();

  Serial.println("OLED test done");
}

void loop() {
}