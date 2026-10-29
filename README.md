# 灵犀指 (Lingxi Pointer)

**[English](README_EN.md) | 简体中文**

基于 ESP32-S3 与 TinyML 的桌面手势控制器。戴上它，用手势控制电脑翻页、播放/暂停、静音，彻底摆脱鼠标束缚。

## 📸 演示 (Demo)

![Uploading dfc23ad205db4c7294fa439febb68c39.jpg…]()


## 🌟 功能特点

- **本地推理**：基于 TensorFlow Lite Micro 和轻量级 1D-CNN，所有计算在 ESP32-S3 本地完成，数据不上传，保护隐私。
- **5 种手势识别**：左滑、右滑、上翻、下翻、静止。
- **蓝牙 HID 直连**：模拟成标准蓝牙键盘，无需电脑安装任何驱动，配对即用。
- **实时反馈**：OLED 屏幕实时显示当前手势、置信度与蓝牙连接状态。
- **按键触发**：按下按键才采集 1 秒数据并进行推理，有效防止误触和重复触发。

## 🛠️ 硬件架构

### 物料清单 (BOM)
- **主控**: ESP32-S3-N16R8 DevKitC-1 (为缩小体积，已拆除排针并用软线焊接)
- **传感器**: MPU6050 (六轴加速度+陀螺仪)
- **显示屏**: 0.96" OLED (SSD1306, I2C 接口, 地址 0x3C)
- **交互**: 普通轻触按键
- **供电**: 3.7V 550mAh 锂电池 + TP4056 充电保护模块 + S08 3.3V 升降压稳压模块

### 引脚接线表
| 模块 | 引脚 | ESP32-S3 引脚 | 说明 |
| :--- | :--- | :--- | :--- |
| **MPU6050 & OLED** | SDA | GPIO8 | I2C 数据线 |
| **MPU6050 & OLED** | SCL / SCK | GPIO9 | I2C 时钟线 |
| **按键** | 一端 | GPIO4 | 内部上拉 `INPUT_PULLUP` |
| **按键** | 另一端 | GND | 按下时拉低 |
| **供电** | 3.3V | 3.3V | 接 S08 稳压模块 VOUT+ |
| **供电** | GND | GND | 共地 |

## 🚀 快速开始

### 1. 编译与烧录
1. 克隆本仓库到本地。
2. 使用 VS Code 和 [PlatformIO](https://platformio.org/) 打开本项目。
3. 确保 `platformio.ini` 中的编译过滤设置为：`build_src_filter = +<gesture_ble.cpp> -<main.cpp> ...`
4. 点击 PlatformIO 底部的 `Upload` 按钮烧录固件。
5. 打开串口监视器 (115200)，静止 3 秒等待陀螺仪校准。
6. 电脑打开蓝牙，搜索并连接设备 **"灵犀指"**。
7. 按下按键，做手势，即可控制电脑！

### 2. 电脑端映射
| 手势 | 电脑操作 |
| :--- | :--- |
| 左滑 | `Left Arrow` (上一页/快退) |
| 右滑 | `Right Arrow` (下一页/快进) |
| 上翻 | `Play/Pause` (播放/暂停) |
| 下翻 | `Mute` (静音) |
| 静止 | 无动作 |

## 🧠 模型训练指南

如果你需要修改手势或重新训练模型，请参考根目录下的 Python 脚本。

1. **准备环境**: 安装 Python 3.11，运行 `pip install tensorflow numpy pyserial matplotlib`。
2. **采集数据**: 烧录 `main.cpp` 采集固件，运行 `python collect_batch.py`，按提示采集手势数据（建议每类 50 次以上）。
3. **预处理**: 运行 `python prepare_data.py` 和 `python split_data.py`。
4. **模型训练**: 运行 `python train_module.py`，脚本会自动训练 1D-CNN 并导出 `gesture_model.tflite` 和归一化参数。
5. **部署固件**: 运行 `python tflite_to_c.py` 将模型转为 C 头文件，重新编译固件即可。

## 📄 开源协议

本项目采用双协议开源：
- **软件代码**: 遵循 [MIT License](https://opensource.org/licenses/MIT)。
- **硬件设计**: 遵循 [CERN-OHL-P-2.0](https://ohwr.org/cern_ohl_p_v2.txt)。
