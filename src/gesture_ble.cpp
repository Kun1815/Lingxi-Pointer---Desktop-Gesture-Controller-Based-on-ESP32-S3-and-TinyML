/*
  开发板：ESP32-S3-N16R8 DevKitC-1
  环境：PlatformIO + Arduino
  库：TensorFlowLite_ESP32、Adafruit_MPU6050、Adafruit_SSD1306、ESP32-BLE-Keyboard
  引脚：SDA=GPIO8，SCL=GPIO9，MPU 地址=0x68，OLED 地址=0x3C，按键=GPIO4 接 GND
  功能：按一下按键，采集 1 秒，识别手势，OLED 显示，蓝牙发送按键
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <TensorFlowLite_ESP32.h>
#include <BleKeyboard.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "gesture_model.h"
#include "norm_params.h"

#define I2C_SDA 8
#define I2C_SCL 9
#define MPU_ADDR 0x68
#define BUTTON_PIN 4
#define WINDOW_SIZE 100
#define SAMPLE_INTERVAL_US 10000

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

constexpr int kTensorArenaSize = 300 * 1024;
uint8_t* tensor_arena = nullptr;

tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

Adafruit_MPU6050 mpu;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
BleKeyboard bleKeyboard("灵犀指", "Espressif", 100);

float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;
float window[WINDOW_SIZE][6];
int windowIndex = 0;
uint32_t nextSampleUs = 0;

const char* gestureNames[5] = {"LEFT", "RIGHT", "UP", "DOWN", "STILL"};

enum State { WAITING, RECORDING, DONE };
State state = WAITING;

const char* lastGesture = "None";
float lastConfidence = 0.0f;
unsigned long lastOledUpdate = 0;

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("BT: ");
  display.println(bleKeyboard.isConnected() ? "Connected" : "Waiting");

  display.setCursor(0, 16);
  display.print("Gesture: ");
  display.println(lastGesture);

  display.setCursor(0, 32);
  display.print("Conf: ");
  display.println(lastConfidence, 4);

  display.setCursor(0, 48);
  display.println("Press button");
  display.display();
}

void sendGestureCommand(int idx) {
  if (!bleKeyboard.isConnected()) {
    Serial.println("蓝牙未连接，跳过发送");
    return;
  }
  switch (idx) {
    case 0: bleKeyboard.write(KEY_LEFT_ARROW); Serial.println("发送: 左箭头"); break;
    case 1: bleKeyboard.write(KEY_RIGHT_ARROW); Serial.println("发送: 右箭头"); break;
    case 2: bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE); Serial.println("发送: 播放/暂停"); break;
    case 3: bleKeyboard.write(KEY_MEDIA_MUTE); Serial.println("发送: 静音"); break;
    case 4: Serial.println("静止，无手势，不发送"); break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 init failed");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("LingXi Finger");
    display.setCursor(0, 16);
    display.println("OLED OK");
    display.display();
    delay(1000);
  }

  if (!mpu.begin(MPU_ADDR, &Wire)) {
    Serial.println("MPU6050 not found");
    while (1);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("Calibrating gyro, keep still...");
  delay(3000);
  const int N = 500;
  for (int i = 0; i < N; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    gyroBiasX += g.gyro.x;
    gyroBiasY += g.gyro.y;
    gyroBiasZ += g.gyro.z;
    delay(10);
  }
  gyroBiasX /= N; gyroBiasY /= N; gyroBiasZ /= N;
  Serial.println("Gyro calibrated");

  tensor_arena = (uint8_t*)ps_malloc(kTensorArenaSize);
  if (tensor_arena == nullptr) {
    Serial.println("ps_malloc failed");
    while (1);
  }

  model = tflite::GetModel(gesture_model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch");
    while (1);
  }

  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors failed");
    while (1);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  bleKeyboard.begin();
  Serial.println("Ready. Press button to recognize one gesture.");
  updateOLED();
  nextSampleUs = micros();
}

void loop() {
  uint32_t nowUs = micros();
  if ((int32_t)(nowUs - nextSampleUs) < 0) return;
  nextSampleUs += SAMPLE_INTERVAL_US;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float ax = a.acceleration.x / 9.80665;
  float ay = a.acceleration.y / 9.80665;
  float az = a.acceleration.z / 9.80665;
  float gx = g.gyro.x - gyroBiasX;
  float gy = g.gyro.y - gyroBiasY;
  float gz = g.gyro.z - gyroBiasZ;

  switch (state) {
    case WAITING:
      if (digitalRead(BUTTON_PIN) == LOW) {
        windowIndex = 0;
        state = RECORDING;
        Serial.println("Recording...");
      }
      break;

    case RECORDING:
      window[windowIndex][0] = ax;
      window[windowIndex][1] = ay;
      window[windowIndex][2] = az;
      window[windowIndex][3] = gx;
      window[windowIndex][4] = gy;
      window[windowIndex][5] = gz;
      windowIndex++;

      if (windowIndex >= WINDOW_SIZE) {
        state = DONE;

        for (int t = 0; t < WINDOW_SIZE; t++) {
          for (int c = 0; c < 6; c++) {
            input->data.f[t * 6 + c] = (window[t][c] - norm_mean[c]) / norm_std[c];
          }
        }

        if (interpreter->Invoke() != kTfLiteOk) {
          Serial.println("Invoke failed");
          break;
        }

        int bestIdx = 0;
        float bestProb = output->data.f[0];
        for (int i = 1; i < 5; i++) {
          if (output->data.f[i] > bestProb) {
            bestProb = output->data.f[i];
            bestIdx = i;
          }
        }

        if (bestProb < 0.90f) {
          Serial.println("Low confidence, ignored");
          lastGesture = "LowConf";
          lastConfidence = bestProb;
        } else {
          Serial.print("手势: ");
          Serial.print(gestureNames[bestIdx]);
          Serial.print(", 置信度: ");
          Serial.println(bestProb, 4);
          lastGesture = gestureNames[bestIdx];
          lastConfidence = bestProb;
          sendGestureCommand(bestIdx);
        }
        updateOLED();
      }
      break;

    case DONE:
      if (digitalRead(BUTTON_PIN) == HIGH) {
        state = WAITING;
        Serial.println("Ready for next gesture.");
      }
      break;
  }

  // 每 500ms 刷新 OLED，但录音期间不刷新，避免影响 I2C 采样
  if (state != RECORDING && millis() - lastOledUpdate > 500) {
    lastOledUpdate = millis();
    updateOLED();
  }
}