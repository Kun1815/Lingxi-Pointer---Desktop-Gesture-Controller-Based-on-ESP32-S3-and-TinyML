# Lingxi Pointer

**English | [简体中文](README.md)**

A desktop gesture controller based on ESP32-S3 and TinyML. Wear it to control your computer (page up/down, play/pause, mute) with simple hand gestures, completely offline.

## 📸 Demo

*(Add a photo or GIF of your device here)*
![Device Demo](https://via.placeholder.com/800x400.png?text=Your+Awesome+Photo+Here)

## 🌟 Features

- **Offline Inference**: Powered by TensorFlow Lite Micro and a lightweight 1D-CNN. All computation happens locally on the ESP32-S3, ensuring data privacy.
- **5 Gestures**: Left, Right, Up, Down, and Still.
- **BLE HID**: Acts as a standard Bluetooth keyboard. No drivers required, just pair and play.
- **Real-time Feedback**: OLED screen displays the current gesture, confidence score, and Bluetooth status.
- **Button Trigger**: The device captures 1 second of data and performs inference only when the button is pressed, preventing accidental triggers.

## 🛠️ Hardware Architecture

### Bill of Materials (BOM)
- **MCU**: ESP32-S3-N16R8 DevKitC-1 (Pin headers removed and soft wires soldered for compact size)
- **Sensor**: MPU6050 (6-axis Accelerometer + Gyroscope)
- **Display**: 0.96" OLED (SSD1306, I2C, Address 0x3C)
- **Interaction**: Push button
- **Power**: 3.7V 550mAh LiPo Battery + TP4056 Charger + S08 3.3V Buck-Boost Module

### Pinout
| Module | Pin | ESP32-S3 Pin | Note |
| :--- | :--- | :--- | :--- |
| **MPU6050 & OLED** | SDA | GPIO8 | I2C Data |
| **MPU6050 & OLED** | SCL / SCK | GPIO9 | I2C Clock |
| **Button** | One end | GPIO4 | Internal `INPUT_PULLUP` |
| **Button** | Other end | GND | Pulled low when pressed |
| **Power** | 3.3V | 3.3V | Connect to S08 VOUT+ |
| **Power** | GND | GND | Common Ground |

## 🚀 Quick Start

### 1. Build & Upload
1. Clone this repository.
2. Open the project with VS Code and [PlatformIO](https://platformio.org/).
3. Ensure `build_src_filter` in `platformio.ini` is set to: `build_src_filter = +<gesture_ble.cpp> -<main.cpp> ...`
4. Click the `Upload` button in PlatformIO.
5. Open the Serial Monitor (115200 baud) and keep the device still for 3 seconds for gyro calibration.
6. Search for **"灵犀指"** on your PC's Bluetooth settings and pair it.
7. Press the button, perform a gesture, and control your PC!

### 2. Key Mapping
| Gesture | PC Action |
| :--- | :--- |
| Left | `Left Arrow` (Previous / Rewind) |
| Right | `Right Arrow` (Next / Fast Forward) |
| Up | `Play/Pause` |
| Down | `Mute` |
| Still | None |

## 🧠 Model Training Guide

To modify gestures or retrain the model, refer to the Python scripts in the root directory.

1. **Environment**: Install Python 3.11, run `pip install tensorflow numpy pyserial matplotlib`.
2. **Collect Data**: Flash the `main.cpp` firmware, run `python collect_batch.py`, and follow the prompts to record gesture data (at least 50 samples per class recommended).
3. **Preprocessing**: Run `python prepare_data.py` and `python split_data.py`.
4. **Training**: Run `python train_module.py`. It will train the 1D-CNN and export `gesture_model.tflite` along with normalization parameters.
5. **Deploy**: Run `python tflite_to_c.py` to convert the model to a C header, then recompile the firmware.

## 📄 License

This project is dual-licensed:
- **Software**: [MIT License](https://opensource.org/licenses/MIT).
- **Hardware**: [CERN-OHL-P-2.0](https://ohwr.org/cern_ohl_p_v2.txt).
