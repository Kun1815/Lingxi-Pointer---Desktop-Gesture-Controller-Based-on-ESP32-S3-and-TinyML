import os
import numpy as np
import tensorflow as tf

# 加载数据
data = np.load(os.path.join("output", "train_test.npz"))
X_test = data["X_test"].astype(np.float32)
y_test = data["y_test"]

# 取第一个测试样本
sample = X_test[0]
label = y_test[0]

print(f"真实标签: {label}")
print(f"样本形状: {sample.shape}")

# 用 PC 端 TFLite 模型推理
interpreter = tf.lite.Interpreter(model_path=os.path.join("output", "gesture_model.tflite"))
interpreter.allocate_tensors()
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

interpreter.set_tensor(input_details[0]['index'], np.expand_dims(sample, axis=0))
interpreter.invoke()
output = interpreter.get_tensor(output_details[0]['index'])[0]

print("PC 端 TFLite 推理结果:")
for i, p in enumerate(output):
    print(f"  {i}: {p:.4f}")

# 生成 C 头文件
header_path = os.path.join("include", "test_sample.h")
with open(header_path, "w") as f:
    f.write("#ifndef TEST_SAMPLE_H\n")
    f.write("#define TEST_SAMPLE_H\n\n")
    f.write(f"const int test_sample_label = {label};\n")
    f.write("const float test_sample[100][6] = {\n")
    for t in range(100):
        f.write("  {")
        for c in range(6):
            f.write(f"{sample[t][c]:.6f}f")
            if c < 5:
                f.write(", ")
        f.write("},\n")
    f.write("};\n\n")
    f.write("#endif\n")

print(f"\n已生成 {header_path}")