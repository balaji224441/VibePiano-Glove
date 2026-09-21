# 🎹 VibePiano Glove

## Gesture-Based Air Piano System Using ESP32

VibePiano Glove is a wearable gesture-based musical interface that converts finger movements into piano notes without requiring a physical piano keyboard.

The system uses five flex sensors to detect finger bending and an MPU6050 IMU to detect wrist orientation. An ESP32 processes the sensor data and communicates the detected musical notes to a Python-based sound engine through serial communication. Vibration motors provide haptic feedback when a finger activates a note.

---

## 📌 Project Overview

The VibePiano Glove combines wearable sensing, embedded processing, gesture recognition, audio generation, and haptic feedback into a single system.

The system combines:

- 🎛️ Five flex sensors for finger-bend detection
- 🧭 MPU6050 IMU for wrist orientation detection
- ⚡ ESP32 for real-time processing
- 🔄 Serial communication between ESP32 and PC
- 🎵 Python-based piano sound playback
- 📳 Vibration motors for haptic feedback
- 🔧 Automatic sensor calibration

---

## ✨ Features

- 🎹 Five-finger piano note generation
- ✋ Real-time finger bend detection
- 📐 Automatic flex sensor calibration
- 🧭 Wrist orientation detection
- 🎼 Wrist-based octave switching
- 📳 Haptic feedback
- 💻 Python-based audio playback
- ⚡ Real-time serial communication
- 🖐️ Wearable glove-based interface

---

## 🔧 Hardware Used

| Component | Quantity | Purpose |
|---|---:|---|
| ESP32 | 1 | Main microcontroller |
| Flex Sensors | 5 | Finger bend detection |
| MPU6050 | 1 | Wrist orientation detection |
| Vibration Motors | 5 | Haptic feedback |
| Glove | 1 | Wearable platform |
| Connecting Wires | - | Circuit connections |

---

## 🎯 Finger-to-Note Mapping

Each finger is assigned to a different piano note.

| Finger | Note |
|---|---|
| Thumb | C |
| Index Finger | D |
| Middle Finger | E |
| Ring Finger | F |
| Pinky Finger | G |

```text
Thumb  → C
Index  → D
Middle → E
Ring   → F
Pinky  → G
