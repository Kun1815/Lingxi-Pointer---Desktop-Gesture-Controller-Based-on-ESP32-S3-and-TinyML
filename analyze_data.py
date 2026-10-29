# 数据质量检查脚本
# 读取当前目录下所有 gesture_*.csv，输出每个文件的行数、平均采样间隔、加速度合模长范围

import glob
import csv
import math
import os

DATA_DIR = "data"
files = sorted(glob.glob(os.path.join(DATA_DIR, "gesture_*.csv")))

if not files:
    print("没有找到 gesture_*.csv 文件，请先运行 collect.py 采集数据")
    exit()

for filename in files:
    with open(filename, "r") as f:
        reader = csv.reader(f)
        header = next(reader)
        rows = [row for row in reader if row and not row[0].startswith("#")]

    if len(rows) < 2:
        print(f"{filename}: 数据太少，跳过")
        continue

    t_us_list = [float(row[0]) for row in rows]
    ax_list = [float(row[1]) for row in rows]
    ay_list = [float(row[2]) for row in rows]
    az_list = [float(row[3]) for row in rows]

    # 计算相邻行时间差
    diffs = [t_us_list[i+1] - t_us_list[i] for i in range(len(t_us_list)-1)]
    avg_diff = sum(diffs) / len(diffs)
    avg_rate = 1e6 / avg_diff if avg_diff > 0 else 0

    # 计算加速度合模长
    norms = [math.sqrt(ax*ax + ay*ay + az*az) for ax, ay, az in zip(ax_list, ay_list, az_list)]
    norm_min = min(norms)
    norm_max = max(norms)
    norm_avg = sum(norms) / len(norms)

    print(f"文件: {filename}")
    print(f"  行数: {len(rows)}")
    print(f"  平均采样间隔: {avg_diff:.1f} us")
    print(f"  估算采样率: {avg_rate:.1f} Hz")
    print(f"  加速度合模长: min={norm_min:.2f}g, max={norm_max:.2f}g, avg={norm_avg:.2f}g")
    print()