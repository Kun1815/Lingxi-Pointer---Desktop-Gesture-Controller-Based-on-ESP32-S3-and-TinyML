# 手势数据可视化脚本
# 读取当前目录下所有 gesture_*.csv，为每个文件画图并保存为 PNG
# 开发板：ESP32-S3-N16R8 DevKitC-1
# 引脚：SDA=GPIO8，SCL=GPIO9，MPU 地址=0x68

import glob
import csv
import matplotlib.pyplot as plt

import os

DATA_DIR = "data"
OUTPUT_DIR = "output"

files = sorted(glob.glob(os.path.join(DATA_DIR, "gesture_*.csv")))

if not files:
    print("没有找到 gesture_*.csv 文件")
    exit()

for filename in files:
    with open(filename, "r") as f:
        reader = csv.reader(f)
        header = next(reader)
        rows = [row for row in reader if row and not row[0].startswith("#")]

    if len(rows) < 2:
        print(f"{filename}: 数据太少，跳过")
        continue

    t_us = [float(row[0]) for row in rows]
    ax = [float(row[1]) for row in rows]
    ay = [float(row[2]) for row in rows]
    az = [float(row[3]) for row in rows]
    gx = [float(row[4]) for row in rows]
    gy = [float(row[5]) for row in rows]
    gz = [float(row[6]) for row in rows]

    # 时间转成秒，从 0 开始
    t0 = t_us[0]
    t_s = [(t - t0) / 1e6 for t in t_us]

    fig, axes = plt.subplots(2, 1, figsize=(10, 6), sharex=True)

    axes[0].plot(t_s, ax, label="ax")
    axes[0].plot(t_s, ay, label="ay")
    axes[0].plot(t_s, az, label="az")
    axes[0].set_ylabel("Accel (g)")
    axes[0].legend()
    axes[0].grid(True)

    axes[1].plot(t_s, gx, label="gx")
    axes[1].plot(t_s, gy, label="gy")
    axes[1].plot(t_s, gz, label="gz")
    axes[1].set_ylabel("Gyro (rad/s)")
    axes[1].set_xlabel("Time (s)")
    axes[1].legend()
    axes[1].grid(True)

    fig.suptitle(filename)
    fig.tight_layout()

    out_png = os.path.join(OUTPUT_DIR, os.path.basename(filename).replace(".csv", ".png"))
    fig.savefig(out_png)
    plt.close(fig)

    print(f"已生成 {out_png}")