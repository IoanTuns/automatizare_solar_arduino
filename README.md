# Solar-Powered Greenhouse Automation System

A robust, modular automation system built for the **Arduino UNO R4 WiFi**. This project manages greenhouse environments through intelligent irrigation, climate control, and motorized ventilation, featuring a secure web-based control panel.

## Core Features
- **Secure Web Dashboard**: Real-time monitoring and manual override for all subsystems. Protected by SHA-256 hashed authentication and session management.
- **Multi-Zone Irrigation**: 3-zone control utilizing capacitive soil moisture sensors, solenoid valves, and water pumps with hysteresis logic.
- **Climate Management**: Dual DHT sensor integration (Internal/External) to automate ventilation fans.
- **Motorized Control**: Linear actuator control for doors and ventilation traps with limit switch feedback and safety timeouts.
- **Hardware Diagnostics**: Built-in I2C scanner and automated Multiplexer (CD74HC4067) validation to ensure wiring integrity.
- **Data Logging**: Local logging of sensor telemetry to an SD card with RTC (DS3231) timestamps.

## Hardware Architecture
- **Controller**: Arduino UNO R4 WiFi
- **I/O Expansion**: Dual PCF8574 I2C Expanders
  - **PCF1 (0x20)**: Valves, Pumps, and Fans.
  - **PCF2 (0x21)**: Door/Trap motors and Trap limit switches.
- **Analog Expansion**: CD74HC4067 16-channel Multiplexer for soil and rain sensors.
- **Sensors**: DHT22/11, Water Flow Meters (Interrupt-driven), Rain Sensor.

## Setup Instructions

### 1. Configuration
All hardware pin assignments, I2C addresses, and sensor thresholds are centralized in `src/config.h` and `src/config.cpp`. Ensure these match your physical wiring before deployment.

### 2. Secret Credentials
Create a file named `src/secrets.h` to store sensitive information. This file is excluded from version control:

```cpp
#ifndef SECRETS_H
#define SECRETS_H

// WiFi Credentials
#define WIFI_SSID "your_network_name"
#define WIFI_PASS "your_network_password"

// Default web authentication credentials
#define DEFAULT_WEB_USERNAME "admin"
#define DEFAULT_WEB_PASSWORD "your_secure_password"

#endif
```

### 3. Build & Deployment
This project is developed using **PlatformIO**.
1. Open the project in VS Code with the PlatformIO extension.
2. Connect your Arduino UNO R4 WiFi.
3. Build and Upload using the PlatformIO toolbar.
4. Open the Serial Monitor at **115200 baud** to view system initialization and IP address.

## Project Structure
- `src/main.cpp`: Application entry point and sensor polling loop.
- `src/HardwareInit.cpp`: Hardware abstraction and diagnostic routines.
- `src/SolarWebServer.cpp`: Secure HTTP server and HTML UI generation.
- `src/WebAuthentication.cpp`: Session handling and SHA-256 credential validation.
- `src/IrrigationControl.cpp`: Logic for moisture-based watering.
- `src/DoorControl.cpp` & `src/TrapControl.cpp`: Motorized actuator management.

---
*Note: This project was developed as a learning exercise in integrating Arduino hardware with modular C++ design and AI-assisted engineering.*
