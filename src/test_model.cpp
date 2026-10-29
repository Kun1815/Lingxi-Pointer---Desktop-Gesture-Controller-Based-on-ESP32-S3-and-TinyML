/*
  开发板：ESP32-S3-N16R8 DevKitC-1
  环境：PlatformIO + Arduino
  库：TensorFlowLite_ESP32
  功能：加载 gesture_model.tflite，用全零输入跑一次推理，打印输出
*/

#include <Arduino.h>
#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "gesture_model.h"
#include "test_sample.h"

constexpr int kTensorArenaSize = 300 * 1024;
uint8_t* tensor_arena = nullptr;

tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // 打印 PSRAM 状态，用于诊断
  Serial.printf("PSRAM size: %d bytes\n", ESP.getPsramSize());
  Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());

  tensor_arena = (uint8_t*)ps_malloc(kTensorArenaSize);
  if (tensor_arena == nullptr) {
    Serial.println("ps_malloc failed");
    while (1);
  }

  model = tflite::GetModel(gesture_model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema version mismatch");
    while (1);
  }

  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    while (1);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.print("Input shape: ");
  for (int i = 0; i < input->dims->size; i++) {
    Serial.print(input->dims->data[i]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("Output shape: ");
  for (int i = 0; i < output->dims->size; i++) {
    Serial.print(output->dims->data[i]);
    Serial.print(" ");
  }
  Serial.println();

  // 用真实数据输入
  for (int t = 0; t < 100; t++) {
    for (int c = 0; c < 6; c++) {
      input->data.f[t * 6 + c] = test_sample[t][c];
    }
  }

  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Invoke() failed");
    while (1);
  }

  Serial.print("真实标签: ");
  Serial.println(test_sample_label);
  Serial.println("ESP32 推理结果:");
  for (int i = 0; i < output->dims->data[1]; i++) {
    Serial.print(i);
    Serial.print(": ");
    Serial.println(output->data.f[i], 4);
  }
}

void loop() {
}