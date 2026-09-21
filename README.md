# 🎹 VibePiano Glove

## Gesture-Based Air Piano System Using ESP32

VibePiano Glove is a wearable gesture-based musical interface that converts finger movements into piano notes.

The system uses five flex sensors to detect finger bending and an MPU6050 sensor to detect wrist orientation. An ESP32 performs real-time sensor processing and communicates the detected musical notes to a Python-based sound engine. Vibration motors provide haptic feedback during finger activation.

---

## 📌 Project Overview

VibePiano Glove explores a wearable human-machine interface in which finger movements can be used to generate musical notes without requiring a physical piano keyboard.

The system combines:

- Five flex sensors for finger-bend detection
- MPU6050 for wrist orientation detection
- ESP32 for real-time processing
- Serial communication between ESP32 and PC
- Python-based piano sound generation
- Vibration motors for haptic feedback
- Automatic sensor calibration

---

## ✨ Features

- 🎹 Five-finger piano note generation
- 🖐️ Real-time finger bend detection
- 📐 Automatic flex sensor calibration
- 🧭 Wrist orientation detection
- 🎼 Wrist-based octave switching
- 📳 Haptic feedback
- 💻 Python-based audio playback
- ⚡ Real-time serial communication

---

## 🔧 Hardware Used

| Component | Quantity | Purpose |
|---|---:|---|
| ESP32 | 1 | Main microcontroller |
| Flex Sensors | 5 | Finger bend detection |
| MPU6050 | 1 | Wrist orientation detection |
| Vibration Motors | 5 | Haptic feedback |
| Glove | 1 | Wearable platform |
| Breadboard & Wires | 1 set | Circuit connections |

---

## 🎵 Finger-to-Note Mapping

| Finger | Note |
|---|---|
| Thumb | C |
| Index | D |
| Middle | E |
| Ring | F |
| Pinky | G |

The same five-note pattern is used across the supported octaves.

---

## 🎼 Octave Control

The MPU6050 is used to detect wrist orientation and select the octave.

| Wrist Position | Octave |
|---|---:|
| Downward Tilt | 3 |
| Neutral Position | 4 |
| Upward Tilt | 5 |

This allows the same finger gestures to generate notes in different octaves.

---

## ⚙️ System Architecture

The system follows this signal flow:

**Flex Sensors → ESP32 → Gesture Detection → Note Generation → Python Sound Engine → Piano Audio**

The MPU6050 provides wrist orientation information to the ESP32 for octave selection.

Vibration motors provide haptic feedback when finger gestures are detected.

---

## 🔄 Working Principle

### 1. Finger Sensing

Five flex sensors are attached to the fingers of the glove.

The ESP32 reads the analog values from the sensors to determine the amount of finger bending.

### 2. Flex Sensor Calibration

Each finger is calibrated using two positions:

- Straight position
- Fully bent position

The measured values are used to calculate the bend range and press/release thresholds.

### 3. Gesture Detection

When a finger crosses its calculated press threshold, the ESP32 generates the corresponding piano note.

When the finger returns below the release threshold, the note is stopped.

### 4. Wrist Orientation Detection

The MPU6050 measures wrist orientation.

Three calibration positions are used:

- Neutral position
- Upward tilt
- Downward tilt

These positions determine the selected octave.

### 5. Audio Generation

The ESP32 sends detected note information through serial communication.

The Python sound engine receives the note information and plays the corresponding WAV file.

### 6. Haptic Feedback

Vibration motors provide tactile feedback when a finger gesture is detected.

---

## 💻 Software

### ESP32

- Arduino IDE
- C/C++
- Adafruit MPU6050 Library
- Adafruit Unified Sensor Library

### PC Sound Engine

- Python 3
- PySerial
- Windows `winsound`

---

## 🔌 ESP32 Pin Configuration

### Flex Sensors

| Finger | ESP32 GPIO |
|---|---:|
| Thumb | GPIO 35 |
| Index | GPIO 36 |
| Middle | GPIO 34 |
| Ring | GPIO 32 |
| Pinky | GPIO 33 |

### Vibration Motors

| Finger | ESP32 GPIO |
|---|---:|
| Thumb | GPIO 25 |
| Index | GPIO 26 |
| Middle | GPIO 27 |
| Ring | GPIO 14 |
| Pinky | GPIO 13 |

### MPU6050

| MPU6050 Pin | ESP32 Pin |
|---|---|
| SDA | GPIO 21 |
| SCL | GPIO 22 |

---

## 📊 Calibration

The system performs calibration before entering performance mode.

### Flex Sensor Calibration

Each finger follows this process:

**Straight Position → Sensor Measurement → Fully Bent Position → Sensor Measurement → Bend Range → Press Threshold → Release Threshold**

### MPU6050 Calibration

The wrist is calibrated using:

**Neutral Position → Upward Tilt → Downward Tilt**

This allows the system to adapt to the sensor characteristics and the user's hand position.

---

## 🎹 Supported Notes

### Octave 3

C3, D3, E3, F3, G3

### Octave 4

C4, D4, E4, F4, G4

### Octave 5

C5, D5, E5, F5, G5

---

# 📷 Prototype & Results

## Hardware Prototype

The VibePiano Glove prototype consists of five flex sensors mounted on the fingers, an ESP32 controller, MPU6050 and supporting circuitry.

![Glove Hardware Prototype](glove_hardware_prototype.png)

---

## Prototype with Live Output

The prototype is shown operating while the computer displays the real-time system output.

![Prototype with Live Output](prototype_with_live_output.png)

---

## Flex Sensor Calibration

The serial monitor displays the calibration process for individual fingers, including straight and bent sensor values, bend range, press threshold and release threshold.

![Flex Sensor Calibration](flex_sensor_calibration.png)

---

## Note Detection Output

The serial output demonstrates real-time finger detection, note generation and octave information.

![Note Detection Output](note_detection_output.png)

---

## System Serial Output

The serial monitor displays detected notes, finger information, octave selection and system status during operation.

![System Serial Output](system_serial_output.png)

---

## 📈 Results

The developed prototype demonstrates:

- Five-finger gesture detection
- Finger-to-note mapping
- Wrist-based octave selection
- Haptic feedback
- Real-time sensor processing
- Computer-based piano sound playback
- Automatic sensor calibration

---

## 🚀 Future Improvements

Possible future improvements include:

- Full piano keyboard note range
- Dual-glove implementation
- Advanced gesture recognition
- Flexible PCB integration
- Improved wearable packaging
- 3D-printed enclosure
- AR/VR integration

---

## 📂 Project Structure

The repository currently contains:

- `VibePiano_Glove.ino` — ESP32 firmware
- `piano_sound_engine.py` — Python sound engine
- `requirements.txt` — Python dependency
- `Vibe-Piano_document.pdf` — Complete project documentation
- Prototype and system output images

---

## 📄 Documentation

The complete project documentation is available in:

**Vibe-Piano_document.pdf**

The report contains the project overview, methodology, hardware and software implementation, calibration procedure, results, demonstrations, conclusion and references.

---

## 🛠️ Technologies Used

**ESP32 · Arduino IDE · C/C++ · Python · PySerial · MPU6050 · Flex Sensors · Haptic Feedback · Serial Communication · Wearable Technology · Gesture Recognition**

---

## 👨‍💻 Project

### VibePiano Glove

**Gesture-Based Air Piano System**

Developed using ESP32, flex sensors, MPU6050 and Python-based audio processing.
