/*
  开发板：ESP32-S3-N16R8 DevKitC-1
  环境：PlatformIO + Arduino
  库：ESP32-BLE-Keyboard（本地 lib/ 目录）
  功能：模拟蓝牙键盘，连接后每 3 秒发送一次媒体按键
*/

#include <Arduino.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("灵犀指_测试", "Espressif", 100);

unsigned long lastSend = 0;
int step = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("启动蓝牙键盘...");
  bleKeyboard.begin();
}

void loop() {
  if (bleKeyboard.isConnected()) {
    if (millis() - lastSend > 3000) {
      lastSend = millis();

      switch (step) {
        case 0:
          Serial.println("发送：播放/暂停");
          bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
          break;
        case 1:
          Serial.println("发送：右箭头");
          bleKeyboard.write(KEY_RIGHT_ARROW);
          break;
        case 2:
          Serial.println("发送：左箭头");
          bleKeyboard.write(KEY_LEFT_ARROW);
          break;
        case 3:
          Serial.println("发送：静音");
          bleKeyboard.write(KEY_MEDIA_MUTE);
          break;
      }

      step = (step + 1) % 4;
    }
  } else {
    if (millis() - lastSend > 5000) {
      lastSend = millis();
      Serial.println("等待电脑连接蓝牙...");
    }
  }
}