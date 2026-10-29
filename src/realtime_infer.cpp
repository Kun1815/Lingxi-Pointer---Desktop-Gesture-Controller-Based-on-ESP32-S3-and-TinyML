/*
  开发板：ESP32-S3-N16R8 DevKitC-1
  环境：PlatformIO + Arduino
  库：TensorFlowLite_ESP32、Adafruit_MPU6050
  引脚：SDA=GPIO8，SCL=GPIO9，MPU 地址=0x68，按键=GPIO4 接 GND
  功能：按一下按键，采集 1 秒数据，推理一次，打印手势
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <TensorFlowLite_ESP32.h>
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

constexpr int kTensorArenaSize = 300 * 1024;
uint8_t* tensor_arena = nullptr;

tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

Adafruit_MPU6050 mpu;
float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;

float window[WINDOW_SIZE][6];
int windowIndex = 0;
uint32_t nextSampleUs = 0;

const char* gestureNames[5] = {"左滑", "右滑", "上翻", "下翻", "静止"};

// 状态机
enum State { WAITING, RECORDING, DONE };
State state = WAITING;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Wire.begin(I2C_SDA, I2C_SCL);

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

  Serial.println("Ready. Press button to recognize one gesture.");
  nextSampleUs = micros();
}

void loop() {
  uint32_t nowUs = micros();
  if ((int32_t)(nowUs - nextSampleUs) < 0) return;
  nextSampleUs += SAMPLE_INTERVAL_US;

  // 读 MPU6050，填窗口
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float ax = a.acceleration.x / 9.80665;
  float ay = a.acceleration.y / 9.80665;
  float az = a.acceleration.z / 9.80665;
  float gx = g.gyro.x - gyroBiasX;
  float gy = g.gyro.y - gyroBiasY;
  float gz = g.gyro.z - gyroBiasZ;

  // 按键状态机
  switch (state) {
    case WAITING:
      if (digitalRead(BUTTON_PIN) == LOW) {
        // 按键按下，开始采集
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
        // 窗口满，进入 DONE，等待松开
        state = DONE;

        // 归一化并推理
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
        } else if (bestIdx == 4) {
          Serial.println("静止，无手势");
        } else {
          Serial.print("手势: ");
          Serial.print(gestureNames[bestIdx]);
          Serial.print(", 置信度: ");
          Serial.println(bestProb, 4);
        }
      }
      break;

    case DONE:
      // 等待松开，避免一次按下重复触发
      if (digitalRead(BUTTON_PIN) == HIGH) {
        state = WAITING;
        Serial.println("Ready for next gesture.");
      }
      break;
  }
}