# 批量手势数据采集脚本（含静止类别，优化提示语）
# ESP32：ESP32-S3-N16R8 DevKitC-1
# MPU6050：SDA=GPIO8，SCL=GPIO9，地址=0x68
# PC 库：pyserial
# 串口：COM9，波特率：115200

import serial
import csv
import time
import msvcrt
import os

PORT = "COM9"
BAUD = 115200
DURATION = 2          # 每次采集 2 秒
REPEAT = 50           # 每个类别采集次数
REST_BEFORE = 1.0     # 采集前准备时间
REST_AFTER = 3.0      # 采集后休息时间，让手回到初始状态

DATA_DIR = "data"
os.makedirs(DATA_DIR, exist_ok=True)

ser = serial.Serial(PORT, BAUD, timeout=1)

print("=" * 60)
print("灵犀指 - 批量手势数据采集")
print(f"每次采集 {DURATION} 秒，连续 {REPEAT} 次")
print("按 1：左滑")
print("按 2：右滑")
print("按 3：上翻")
print("按 4：下翻")
print("按 5：静止 / 无手势")
print("按 q：退出")
print("=" * 60)
print("提示：每次采集完成后请休息，将手回到初始位置，准备下一个动作。\n")

def collect_one(gesture):
    filename = os.path.join(DATA_DIR, f"gesture_{gesture}_{int(time.time()*1000)}.csv")
    f = open(filename, "w", newline="")
    writer = csv.writer(f)
    writer.writerow(["t_us", "ax_g", "ay_g", "az_g",
                     "gx_rps", "gy_rps", "gz_rps"])
    start = time.time()
    count = 0
    while time.time() - start < DURATION:
        line = ser.readline().decode("utf-8", "ignore").strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split(",")
        if len(parts) == 7:
            try:
                values = [float(x) for x in parts]
                writer.writerow(values)
                count += 1
            except ValueError:
                pass
    f.close()
    return filename, count

while True:
    if msvcrt.kbhit():
        key = msvcrt.getch().decode("utf-8", "ignore").lower()
        if key in ['1', '2', '3', '4', '5']:
            print(f"\n{'='*60}")
            print(f"开始采集类别 {key}，共 {REPEAT} 次")
            print(f"{'='*60}")
            for i in range(REPEAT):
                print(f"\n--- 第 {i+1}/{REPEAT} 次 ---")
                print(f"请准备... {REST_BEFORE} 秒后开始动作")
                time.sleep(REST_BEFORE)
                if key == '5':
                    print(">>> 保持静止 2 秒！")
                else:
                    print(">>> 开始动作！")
                filename, count = collect_one(key)
                print(f"    已保存 {filename}，{count} 行")
                if i < REPEAT - 1:
                    print(f"    本次采集完成，请休息 {REST_AFTER} 秒，回到初始状态...")
                    time.sleep(REST_AFTER)
            print(f"\n{'='*60}")
            print(f"类别 {key} 采集完成！")
            print(f"{'='*60}\n")
        elif key == 'q':
            print("退出采集")
            break

ser.close()