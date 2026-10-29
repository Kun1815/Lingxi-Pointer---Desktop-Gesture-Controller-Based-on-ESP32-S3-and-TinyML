# 检查 dataset.npz 是否正常
# 数据来源：prepare_data.py 生成的 dataset.npz
# 库：numpy
# 开发板：不涉及，PC 端脚本

import numpy as np
import os
data = np.load(os.path.join("output", "dataset.npz"))


print("dataset.npz 中的键：", list(data.keys()))
print()

X = data["X"]
y = data["y"]
mean = data["mean"]
std = data["std"]

print(f"X.shape = {X.shape}")
print(f"y.shape = {y.shape}")
print(f"mean = {mean}")
print(f"std  = {std}")
print()

# 统计每个类别有多少个窗口
unique, counts = np.unique(y, return_counts=True)
gesture_names = ["左滑", "右滑", "上翻", "下翻"]

print("每个手势的窗口数量：")
for label, count in zip(unique, counts):
    name = gesture_names[label] if label < len(gesture_names) else f"标签{label}"
    print(f"  {label} ({name}): {count} 个窗口")