# PC 端手势数据采集脚本（带按键标注）
# 开发板：ESP32-S3-N16R8 DevKitC-1
# ESP32 引脚：SDA=GPIO8，SCL=GPIO9，MPU 地址=0x68
# PC 库：pyserial
# 串口：COM9，波特率：115200

import serial
import csv
import time
import msvcrt

PORT = "COM9"
BAUD = 115200
DURATION = 2  # 每次采集 2 秒

ser = serial.Serial(PORT, BAUD, timeout=1)

print("=" * 40)
print("手势数据采集工具")
print("按 1 采集：左滑")
print("按 2 采集：右滑")
print("按 3 采集：上翻")
print("按 4 采集：下翻")
print("按 q 退出")
print("=" * 40)

collecting = False
current_gesture = None
start_time = 0
file = None
writer = None

while True:
    if collecting:
        # 检查是否采集满 2 秒
        if time.time() - start_time >= DURATION:
            collecting = False
            file.close()
            print(f"手势 {current_gesture} 采集完成，文件已保存")
            continue

        # 读取串口数据  
        line = ser.readline().decode("utf-8", "ignore").strip()
        if not line or line.startswith("#"):
            continue

        parts = line.split(",")
        if len(parts) == 7:
            try:
                values = [float(x) for x in parts]
                writer.writerow(values)
            except ValueError:
                pass
    else:
        # 等待按键
        if msvcrt.kbhit():
            key = msvcrt.getch().decode("utf-8", "ignore").lower()
            if key in ['1', '2', '3', '4']:
                current_gesture = key
                collecting = True
                start_time = time.time()
                filename = f"gesture_{key}_{int(start_time)}.csv"
                file = open(filename, "w", newline="")
                writer = csv.writer(file)
                writer.writerow(["t_us", "ax_g", "ay_g", "az_g",
                                 "gx_rps", "gy_rps", "gz_rps"])
                print(f"开始采集手势 {key}，请做动作...")
            elif key == 'q':
                print("退出采集")
                break

ser.close()