# TensorFlow 环境和训练数据检查
# 开发板：不涉及，PC 端脚本
# 库：tensorflow、numpy
# 输入：output/train_test.npz

import os
import numpy as np
import tensorflow as tf

print("TensorFlow 版本:", tf.__version__)
print("NumPy 版本:", np.__version__)

path = os.path.join("output", "train_test.npz")
data = np.load(path)

X_train = data["X_train"]
y_train = data["y_train"]
X_test = data["X_test"]
y_test = data["y_test"]

print("训练集:", X_train.shape, y_train.shape)
print("测试集:", X_test.shape, y_test.shape)
print("X 数据类型:", X_train.dtype)
print("y 数据类型:", y_train.dtype)
print("y 唯一值:", np.unique(y_train))
print("X 范围:", X_train.min(), X_train.max())
print("X 均值:", X_train.mean(axis=(0, 1)))
print("X 标准差:", X_train.std(axis=(0, 1)))