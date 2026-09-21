# 🎹 VibePiano Glove

## Gesture-Based Air Piano System Using ESP32

VibePiano Glove is a wearable gesture-based musical interface that converts finger movements into piano notes without requiring a physical piano keyboard.

The system uses five flex sensors for finger-bend detection and an MPU6050 IMU for wrist-orientation detection. An ESP32 processes the sensor data and communicates detected notes to a Python-based sound engine through serial communication. Vibration motors provide haptic feedback.

---

## 📌 Project Overview

The system combines:

- Five flex sensors for finger-bend detection
- MPU6050 for wrist orientation
- ESP32 for real-time processing
- Serial communication between ESP32 and PC
- Python-based audio playback
- Vibration motors for haptic feedback
- Automatic sensor calibration

---

## ✨ Features

- 🎹 Five-finger piano note generation
- ✋ Real-time finger-bend detection
- 📐 Flex sensor calibration
- 🧭 Wrist orientation detection
- 🎼 Wrist-based octave switching
- 📳 Haptic feedback
- 💻 Python-based audio playback
- ⚡ Real-time serial communication
- 🖐️ Wearable glove interface

---

## 🔧 Hardware Used

| Component | Quantity | Purpose |
|---|---:|---|
| ESP32 | 1 | Main microcontroller |
| Flex Sensors | 5 | Finger-bend detection |
| MPU6050 | 1 | Wrist orientation |
| Vibration Motors | 5 | Haptic feedback |
| Glove | 1 | Wearable platform |

---

## 🎯 Finger-to-Note Mapping

| Finger | Note |
|---|---|
| Thumb | C |
| Index Finger | D |
| Middle Finger | E |
| Ring Finger | F |
| Pinky Finger | G |

---

## 🎼 Octave Control

| Wrist Position | Octave |
|---|---|
| Neutral | Octave 4 |
| Tilt Up | Octave 5 |
| Tilt Down | Octave 3 |

---

## ⚙️ System Architecture

```text
Flex Sensors ─────┐
                  │
MPU6050 ──────────┤
                  ▼
              ┌───────┐
              │ ESP32 │
              └───┬───┘
                  │
            Serial Data
                  │
                  ▼
          ┌──────────────┐
          │    Python    │
          │ Sound Engine │
          └──────┬───────┘
                 │
                 ▼
            Audio Output

Vibration Motors ← ESP32
