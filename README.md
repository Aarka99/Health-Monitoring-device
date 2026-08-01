# 🩺 IoT Smart Health Monitoring System using ESP32

A real-time IoT-based Health Monitoring System built using the **ESP32** microcontroller for continuous monitoring of vital physiological parameters such as **Heart Rate**, **Blood Oxygen Saturation (SpO₂)**, and **Body Temperature**.

The system collects sensor data, processes it locally on the ESP32, securely uploads it to **Firebase Realtime Database**, and displays live health metrics through a **Flutter mobile application**. The architecture is designed to support future integration with India's **Ayushman Bharat Health Account (ABHA)** ecosystem. :contentReference[oaicite:1]{index=1}

---

## 📌 Features

- ❤️ Real-time Heart Rate Monitoring
- 🫁 Blood Oxygen (SpO₂) Measurement
- 🌡️ Body Temperature Monitoring
- ☁️ Firebase Cloud Integration
- 📱 Flutter Mobile Dashboard
- 📊 Historical Health Data Visualization
- 📁 CSV Report Export
- 🔔 Emergency Alert System (Buzzer)
- 📺 LCD Display for Live Readings
- 🔐 Secure Cloud Communication
- 🇮🇳 Future-ready ABHA (Ayushman Bharat) Integration

---

# System Architecture

```
                   +----------------------+
                   |     MAX30102 Sensor  |
                   +----------+-----------+
                              |
                              |
                   +----------v-----------+
                   |        ESP32         |
                   |  Data Processing     |
                   |  Validation          |
                   +----------+-----------+
                              |
                     Wi-Fi (HTTPS)
                              |
                              v
                 Firebase Realtime Database
                              |
              +---------------+---------------+
              |                               |
              |                               |
      Flutter Mobile App              Historical Reports
              |                               |
              +---------------+---------------+
                              |
                     Future ABHA Integration
```

---

# Hardware Used

| Component | Purpose |
|-----------|---------|
| ESP32 DevKit | Main Controller |
| MAX30102 | Heart Rate & SpO₂ Sensor |
| NTC Thermistor | Body Temperature |
| 16x2 I2C LCD | Local Display |
| Active Buzzer | Emergency Alert |
| Breadboard | Prototyping |
| Jumper Wires | Connections |
| USB Cable | Programming |

---

# Software Stack

- ESP32 Arduino Framework
- PlatformIO
- Firebase Realtime Database
- Flutter
- Dart
- Arduino Libraries
- Git & GitHub

---

# Project Structure

```
HealthMonitor/
│
├── include/
├── lib/
├── src/
│   ├── main.cpp
│   ├── sensor.cpp
│   ├── firebase.cpp
│   └── lcd.cpp
│
├── test/
├── platformio.ini
└── README.md
```

---

# Working

1. MAX30102 measures Heart Rate and SpO₂.
2. Thermistor measures body temperature.
3. ESP32 validates and processes sensor data.
4. Data is securely transmitted over Wi-Fi.
5. Firebase stores live patient data.
6. Flutter application displays:
   - Live Health Metrics
   - Historical Charts
   - Alerts
   - CSV Reports

---

# Technologies

- ESP32
- C++
- PlatformIO
- Firebase
- Flutter
- Dart
- FreeRTOS
- JSON
- Wi-Fi

---

# Screenshots

## Hardware

- ESP32 Development Board
- MAX30102 Sensor
- LCD Display
- Temperature Sensor

## Mobile App

- Live Dashboard
- Health Graphs
- Alert Screen
- Historical Records

(Add screenshots here)

---

# Future Improvements

- ABHA Integration
- Electronic Health Records (EHR)
- AI-based Health Prediction
- Remote Doctor Dashboard
- MQTT Support
- Wearable Device Integration
- ECG Sensor
- Blood Pressure Sensor
- Cloud Analytics
- Machine Learning-Based Anomaly Detection

---

# Research Paper

This project is based on our research work:

**Design and Implementation of an IoT-Based Smart Health Monitoring System using ESP32 with Cloud Integration and Future ABHA Compatibility**. :contentReference[oaicite:2]{index=2}

---

## Open Project

Open using **PlatformIO** in **Visual Studio Code**.

---

## Configure

Update:

- Wi-Fi SSID
- Wi-Fi Password
- Firebase URL
- Firebase Authentication Token

inside the source code.

---

## Upload

```
PlatformIO → Upload
```

or

```bash
pio run --target upload
```

---

# Requirements

- ESP32 DevKit
- Arduino Framework
- PlatformIO
- Firebase Project
- Flutter SDK

---

# Contributors

- **Aaryan Kanwar**
- **Dhruv Kapoor**
- **Harshit Goswami**

---

# License

This project is released under the MIT License.

---

# Contact

**Aaryan Kanwar**

📧 Email: aaryankanwar5671@gmail.com

GitHub: https://github.com/Aarka99

---

⭐ If you found this project useful, please consider giving it a Star.
