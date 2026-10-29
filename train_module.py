# 轻量级 1D-CNN 手势识别训练脚本
# 环境：Python 3.11 + TensorFlow 2.21
# 输入：output/train_test.npz
# 输出：output/gesture_model.tflite、output/norm_params.npz、output/training_history.csv

import os
import numpy as np
import tensorflow as tf
import csv

INPUT_FILE = os.path.join("output", "train_test.npz")
TFLITE_FILE = os.path.join("output", "gesture_model.tflite")
NORM_FILE = os.path.join("output", "norm_params.npz")
HISTORY_FILE = os.path.join("output", "training_history.csv")

# ---------- 1. 加载数据 ----------
data = np.load(INPUT_FILE)
X_train = data["X_train"].astype(np.float32)
y_train = data["y_train"].astype(np.int32)
X_test = data["X_test"].astype(np.float32)
y_test = data["y_test"].astype(np.int32)
mean = data["mean"]
std = data["std"]

print("训练集:", X_train.shape, y_train.shape)
print("测试集:", X_test.shape, y_test.shape)

# ---------- 2. 构建轻量级 1D-CNN ----------
model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(100, 6)),

    tf.keras.layers.Conv1D(32, 5, activation="relu", padding="same"),
    tf.keras.layers.MaxPooling1D(2),

    tf.keras.layers.Conv1D(64, 3, activation="relu", padding="same"),
    tf.keras.layers.MaxPooling1D(2),

    tf.keras.layers.Flatten(),
    tf.keras.layers.Dense(64, activation="relu"),
    tf.keras.layers.Dropout(0.3),
    tf.keras.layers.Dense(5, activation="softmax"),
])

model.compile(
    optimizer=tf.keras.optimizers.Adam(learning_rate=1e-3),
    loss="sparse_categorical_crossentropy",
    metrics=["accuracy"],
)

model.summary()

# ---------- 3. 训练 ----------
history = model.fit(
    X_train, y_train,
    epochs=80,
    batch_size=32,
    validation_split=0.2,
    verbose=1,
)

# ---------- 4. 评估 ----------
test_loss, test_acc = model.evaluate(X_test, y_test, verbose=0)
print(f"\n测试集准确率: {test_acc:.4f}")

# 保存训练历史
with open(HISTORY_FILE, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["epoch", "loss", "accuracy", "val_loss", "val_accuracy"])
    for i in range(len(history.history["loss"])):
        writer.writerow([
            i + 1,
            history.history["loss"][i],
            history.history["accuracy"][i],
            history.history["val_loss"][i],
            history.history["val_accuracy"][i],
        ])
print(f"训练历史保存到 {HISTORY_FILE}")

# ---------- 5. 转换为 TFLite ----------
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

with open(TFLITE_FILE, "wb") as f:
    f.write(tflite_model)

print(f"TFLite 模型保存到 {TFLITE_FILE}")
print(f"模型大小: {len(tflite_model) / 1024:.1f} KB")

# ---------- 6. 保存归一化参数 ----------
np.savez(NORM_FILE, mean=mean, std=std)
print(f"归一化参数保存到 {NORM_FILE}")