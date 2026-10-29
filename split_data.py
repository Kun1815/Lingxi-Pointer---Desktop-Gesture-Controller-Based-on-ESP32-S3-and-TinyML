# 数据集划分脚本
# 功能：把 dataset.npz 中的 475 个窗口按 8:2 分层划分成训练集和测试集
# 输入：output/dataset.npz
# 输出：output/train_test.npz

import os
import numpy as np

INPUT_FILE = os.path.join("output", "dataset.npz")
OUTPUT_FILE = os.path.join("output", "train_test.npz")

data = np.load(INPUT_FILE)
X = data["X"]  # (475, 100, 6)
y = data["y"]  # (475,)
mean = data["mean"]
std = data["std"]

print(f"原始数据: X={X.shape}, y={y.shape}")

# 固定随机种子，保证每次划分结果一致
rng = np.random.default_rng(42)

train_idx = []
test_idx = []

for label in np.unique(y):
    idx = np.where(y == label)[0]
    rng.shuffle(idx)

    n = len(idx)
    n_train = int(n * 0.8)
    train_idx.extend(idx[:n_train])
    test_idx.extend(idx[n_train:])

train_idx = np.array(train_idx)
test_idx = np.array(test_idx)

rng.shuffle(train_idx)
rng.shuffle(test_idx)

X_train, y_train = X[train_idx], y[train_idx]
X_test, y_test = X[test_idx], y[test_idx]

print(f"训练集: X={X_train.shape}, y={y_train.shape}")
print(f"测试集: X={X_test.shape}, y={y_test.shape}")

# 检查每类分布
print("\n训练集每类数量：")
for label in np.unique(y_train):
    print(f"  {label}: {np.sum(y_train == label)}")

print("测试集每类数量：")
for label in np.unique(y_test):
    print(f"  {label}: {np.sum(y_test == label)}")

np.savez(OUTPUT_FILE,
         X_train=X_train, y_train=y_train,
         X_test=X_test, y_test=y_test,
         mean=mean, std=std)

print(f"\n保存 {OUTPUT_FILE}")