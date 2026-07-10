# 🖐️ Wearable IMU Glove Controller for Gesture-Controlled Robot Car

A wrist-worn motion controller that drives a robot car using hand tilt — no camera, no app, no buttons. An MPU6050 IMU strapped to the wrist streams pitch/roll data to an ESP32, which classifies hand orientation and sends drive commands over MQTT to a second ESP32 controlling the robot's motors.

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Platform](https://img.shields.io/badge/platform-ESP32%20%7C%20MPU6050-orange)

---

## 📋 Table of Contents

- [Project Overview](#-project-overview)
- [Features](#-features)
- [System Architecture](#️-system-architecture)
- [Hardware Requirements](#-hardware-requirements)
- [Software Requirements](#-software-requirements)
- [Installation Guide](#-installation-guide)
- [Configuration](#️-configuration)
- [Calibration](#-calibration)
- [Usage](#-usage)
- [Gesture Mapping](#-gesture-mapping)
- [File Structure](#-file-structure)
- [Troubleshooting](#-troubleshooting)
- [Roadmap](#️-roadmap)
- [License](#-license)

---

## 🎯 Project Overview

This project is a wearable evolution of a camera-based gesture robot: instead of a webcam reading finger count, an **MPU6050 IMU sensor** worn on the wrist reads the hand's pitch and roll relative to a calibrated neutral position. Tilting the palm up/down/left/right is translated into drive commands, published wirelessly over MQTT to the same robot car hardware.

The system consists of:

1. **Glove controller (ESP32 + MPU6050)** — reads accelerometer data, classifies hand tilt into a gesture, publishes MQTT commands
2. **Messaging layer (MQTT / Mosquitto)** — relays commands over WiFi between the two ESP32 boards
3. **Robot layer (ESP32 + L298N)** — subscribes to commands and drives two DC motors

---

## ✨ Features

- 🖐️ Motion-based control using real accelerometer pitch/roll — no camera required
- 📶 Fully wireless glove-to-bot communication over WiFi + MQTT
- 🎯 Data-driven gesture classification calibrated from real sensor readings
- 🔋 Compact wearable design — sensor + ESP32 mounted directly on the wrist
- 🔌 Drop-in replacement for the camera-based controller — robot-side firmware is unchanged

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│               WEARABLE IMU GLOVE CONTROLLER SYSTEM                │
└─────────────────────────────────────────────────────────────────┘

   ┌──────────────┐      ┌───────────────┐      ┌────────────────┐
   │   MPU6050    │─────▶│  ESP32 (Glove)│─────▶│ Pitch/Roll      │
   │ (on wrist)   │ I2C  │  Read Sensor  │      │ Classification  │
   └──────────────┘      └───────────────┘      └────────────────┘
                                                          │
                                                          ▼
                                                 ┌────────────────┐
                                                 │  MQTT Publish  │
                                                 │ (Mosquitto)    │
                                                 └────────────────┘
                                                          │
                                                       WiFi
                                                          │
                                                          ▼
                                                 ┌────────────────┐
                                                 │  ESP32 (Bot)   │
                                                 │  MQTT Subscribe│
                                                 └────────────────┘
                                                          │
                                                          ▼
                                                 ┌────────────────┐
                                                 │  L298N Driver  │
                                                 │  → DC Motors   │
                                                 └────────────────┘
```

---

## 🔧 Hardware Requirements

| Component | Quantity | Purpose |
|---|---|---|
| ESP32 DevKit | 2 | Glove controller + Robot controller |
| MPU6050 IMU (GY-521) | 1 | Accelerometer/gyroscope for hand tilt |
| L298N Motor Driver Module | 1 | Dual DC motor driver (robot side) |
| BO/Geared DC Motors | 2 | Drive wheels |
| Robot chassis + caster wheel | 1 | Frame |
| 7.4V Li-ion battery pack | 1 | Motor power |
| Wrist strap / velcro mount | 1 | Secures MPU6050 to hand |
| Jumper wires | Multiple | Connections |

### Pin Configuration — Glove ESP32

```
Glove ESP32 Pin Configuration:
├─ GPIO21  : MPU6050 SDA (I2C data)
├─ GPIO22  : MPU6050 SCL (I2C clock)
├─ 5V      : MPU6050 VCC
└─ GND     : Common ground
```

### Pin Configuration — Bot ESP32

```
Bot ESP32 Pin Configuration:
├─ GPIO32  : L298N ENA (left motor speed, PWM)
├─ GPIO33  : L298N ENB (right motor speed, PWM)
├─ GPIO26  : L298N IN1
├─ GPIO27  : L298N IN2
├─ GPIO14  : L298N IN3
├─ GPIO25  : L298N IN4
└─ GND     : Common ground with L298N and battery
```

> **Note:** The MPU6050 breakout board used in this build is powered from **5V** rather than 3.3V — some GY-521 modules are unreliable on 3.3V logic. Run an I2C scan to confirm detection at address `0x68` before proceeding.

---

## 💻 Software Requirements

### Arduino (both ESP32 boards)
- Arduino IDE 2.0+
- Board: **ESP32 Dev Module** (via Espressif board manager)
- Libraries:
  - **Adafruit MPU6050**
  - **Adafruit Unified Sensor**
  - **PubSubClient** (Nick O'Leary)

### Broker
- **Mosquitto MQTT Broker** (runs locally on a laptop on the same WiFi network)

---

## 📦 Installation Guide

### Step 1: Clone the Repository

```bash
git clone https://github.com/Iniyan-philomath/wearable-imu-glove-controller.git
cd wearable-imu-glove-controller
```

### Step 2: Install and Start Mosquitto Broker

- Download from [mosquitto.org/download](https://mosquitto.org/download/)
- Install with default settings (runs as a Windows service, auto-starts)
- Verify it's running:
```bash
sc.exe query mosquitto
```

### Step 3: Wire the MPU6050 to the Glove ESP32

Connect SDA → GPIO21, SCL → GPIO22, VCC → 5V, GND → GND. Run an I2C scanner sketch first to confirm the sensor is detected at `0x68`.

### Step 4: Flash the Glove Controller ESP32

1. Open `arduino/glove_controller/glove_controller.ino`
2. Install **Adafruit MPU6050**, **Adafruit Unified Sensor**, and **PubSubClient** libraries
3. Update WiFi credentials and broker IP (see [Configuration](#️-configuration))
4. Select **Tools → Board → ESP32 Dev Module**
5. Flash to the glove ESP32

### Step 5: Flash the Bot ESP32

1. Open `arduino/bot_esp32/bot_esp32.ino`
2. Update WiFi credentials and broker IP
3. Flash to the robot's ESP32

---

## ⚙️ Configuration

### WiFi & MQTT (both `.ino` files)

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "192.168.x.x";  // your laptop's IP running Mosquitto
```

### Gesture Thresholds (`glove_controller.ino`)

Classification is based on real calibrated pitch/roll ranges — adjust if your sensor mount angle differs:

```cpp
String getPhysicalAction(float pitch, float roll) {
  if (pitch > 50) return "forward";
  else if (pitch < -50) return "reverse";
  else if (roll > 50) return "left";
  else if (roll < -50) return "right";
  else return "stop";
}
```

---

## 🎯 Calibration

Because IMU mounting angle varies by wrist and strap position, calibrate thresholds for your own setup:

1. Flash the glove firmware and open Serial Monitor (115200 baud)
2. Hold your hand flat (neutral/origin pose) and note the Pitch/Roll values printed
3. Perform each gesture (palm up, palm down, roll left, roll right) and note the resulting Pitch/Roll ranges
4. Update the threshold values in `getPhysicalAction()` to match your observed ranges, leaving a comfortable margin between gesture clusters
5. Re-flash and verify all 5 states classify correctly and consistently

---

## 🚀 Usage

1. Ensure Mosquitto broker is running
2. Power on the **bot ESP32** — confirm WiFi + MQTT connection via Serial Monitor
3. Power on the **glove ESP32** — confirm WiFi + MQTT connection
4. Hold your hand flat/neutral, then perform gestures (see [Gesture Mapping](#-gesture-mapping))
5. Test with the robot on a stand (wheels off the ground) before floor testing

---

## ✋ Gesture Mapping

| Gesture | Hand Position | Action |
|---|---|---|
| ⬆️ | Palm tilts up | Forward |
| ⬇️ | Palm tilts down | Reverse |
| ↩️ | Wrist rolls left | Left turn |
| ↪️ | Wrist rolls right | Right turn |
| ✋ | Hand flat (neutral) | Stop |

---

## 📂 File Structure

```
wearable-imu-glove-controller/
├── arduino/
│   ├── glove_controller/
│   │   └── glove_controller.ino    # MPU6050 read + classification + MQTT publisher
│   └── bot_esp32/
│       └── bot_esp32.ino           # Motor controller + MQTT subscriber
│
├── .gitignore
├── LICENSE
└── README.md
```

---

## 🔍 Troubleshooting

### MPU6050 not found / I2C read fails

```
Solution:
1. Run an I2C scanner sketch to confirm detection (should show 0x68)
2. Check VCC — some GY-521 boards need 5V rather than 3.3V
3. Reseat all jumper wires; loose breadboard connections are a common cause
4. Secure the sensor firmly to the wrist — wire flex during movement can
   cause intermittent I2C dropouts
```

### Pitch/Roll values stuck at zero

```
Solution:
1. Confirm I2C scan detects the sensor consistently, not just once
2. Check SDA/SCL are on the correct GPIO pins for your board
3. Add Wire.setClock(100000) to slow the I2C bus if using long wires
```

### Gestures misclassify or overlap

```
Solution:
1. Re-run calibration and log actual Pitch/Roll ranges for each gesture
2. Check pitch is evaluated BEFORE roll in the classifier — forward/reverse
   gestures can produce large roll values near ±180° that get misread as
   left/right if roll is checked first
3. Widen the gap between threshold values if two gestures overlap
```

### Bot doesn't respond to glove movement

```
Solution:
1. Confirm both ESP32 boards are on the same WiFi network
2. Test manually: mosquitto_pub -h localhost -t gesturebot/command -m "forward"
3. Check Serial Monitor on both boards for "connected" MQTT status
```

---

## 🗺️ Roadmap

- [ ] Continuous/proportional speed control from tilt magnitude
- [ ] Simultaneous forward + turn (differential drive curves)
- [ ] Gyroscope fusion for improved stability (complementary/Kalman filter)
- [ ] Rechargeable battery integration for the glove unit
- [ ] Bluetooth fallback for lower-latency short-range control

---

## 📝 License

This project is licensed under the MIT License — see [LICENSE](LICENSE) for details.

---

## 🙏 Acknowledgments

- [Adafruit](https://adafruit.com) — MPU6050 and Unified Sensor libraries
- [Eclipse Mosquitto](https://mosquitto.org/) — MQTT broker
- [Espressif Systems](https://www.espressif.com/) — ESP32 platform

---

**Built by Iniyan — Second-year EEE student, Saranathan College of Engineering**
