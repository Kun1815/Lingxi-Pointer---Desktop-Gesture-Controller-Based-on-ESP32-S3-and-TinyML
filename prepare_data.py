# 数据预处理脚本（支持 5 类：0~4，含静止）
# 数据来源：data/gesture_*.csv
# 输出：output/dataset.npz
# 窗口：100 点，步长 50

import glob
import csv
import os
import numpy as np

WINDOW_SIZE = 100
STEP = 50
STILL_THRESHOLD = 0.1   # 只对动作类别做静止过滤

DATA_DIR = "data"
OUTPUT_DIR = "output"
os.makedirs(OUTPUT_DIR, exist_ok=True)

files = sorted(glob.glob(os.path.join(DATA_DIR, "gesture_*.csv")))

if not files:
    print("没有找到 data/gesture_*.csv 文件")
    exit()

X_list = []
y_list = []
skipped = []

for filename in files:
    base = os.path.basename(filename)
    parts = base.split("_")
    if len(parts) < 2 or not parts[1].isdigit():
        print(f"跳过无法解析标签的文件: {filename}")
        continue

    label_num = int(parts[1])
    if label_num < 1 or label_num > 5:
        print(f"跳过非法标签: {filename}")
        continue
    label = label_num - 1  # 1~5 -> 0~4

    with open(filename, "r") as f:
        reader = csv.reader(f)
        header = next(reader)
        rows = [row for row in reader if row and not row[0].startswith("#")]

    if len(rows) < WINDOW_SIZE:
        skipped.append((filename, "行数不足 100"))
        continue

    data = np.array([[float(x) for x in row[1:7]] for row in rows])
    acc_norm = np.sqrt(data[:, 0]**2 + data[:, 1]**2 + data[:, 2]**2)

    # 只对动作类别（label 0~3）做静止过滤，静止类别（label 4）保留
    if label < 4 and acc_norm.max() - acc_norm.min() < STILL_THRESHOLD:
        skipped.append((filename, f"静止对照，变化仅 {acc_norm.max()-acc_norm.min():.3f}g"))
        continue

    for start in range(0, len(data) - WINDOW_SIZE + 1, STEP):
        window = data[start:start + WINDOW_SIZE]
        X_list.append(window)
        y_list.append(label)

if not X_list:
    print("没有生成任何有效窗口")
    exit()

X = np.array(X_list)
y = np.array(y_list)

mean = X.reshape(-1, 6).mean(axis=0)
std = X.reshape(-1, 6).std(axis=0)
std[std == 0] = 1e-8

X_norm = (X - mean) / std

np.savez(os.path.join(OUTPUT_DIR, "dataset.npz"),
         X=X_norm, y=y, mean=mean, std=std)

print(f"保存 output/dataset.npz")
print(f"X.shape = {X_norm.shape}, y.shape = {y.shape}")
print(f"mean = {mean}")
print(f"std  = {std}")
print("\n每个类别窗口数：")
for label in sorted(set(y)):
    print(f"  {label}: {np.sum(y == label)}")
print("\n跳过的文件：")
for f, reason in skipped:
    print(f"  {f}: {reason}")