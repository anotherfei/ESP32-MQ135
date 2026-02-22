# ESP32-MQ2 Air Quality Monitor (AirSense)

An **ESP32-based smoke and gas monitoring system** using an MQ-2 sensor, featuring **real-time air quality analysis**, **hardware alerts**, and a **Wi-Fi–hosted web dashboard** for local monitoring.

---

## What This Project Does

- Reads **analog gas/smoke data** from an MQ-2 sensor
- Converts raw ADC values into a **0–100% air quality score**
- Detects:
  - **Sudden pollution spikes** (rate-of-change detection)
  - **Sustained dangerous air levels** (threshold-based)
- Drives a **hardware LED alert**
- Hosts a **real-time web dashboard** via ESP32 Wi-Fi Access Point

---

## How to View the Dashboard

The ESP32 runs in **Wi-Fi Access Point mode**.

- **SSID:** `myESP32`  
- **Password:** `esp32wifi`  
- **Web Interface:**  
  👉 `http://192.168.4.1`
      **Username:** `AirSense_Guest`
      **Password:** `guest123`

Once connected, the dashboard displays live air quality updates and visual alerts.

---

## Technology Stack

### Firmware
- **Language:** C++
- **Framework:** Arduino (ESP32)
- **Build System:** PlatformIO
- **Libraries:**
  - `WiFi.h`
  - `WebServer.h`
  - `ArduinoJson`

### Web Dashboard (Embedded)
- **HTML** – page structure
- **CSS** – UI layout and alert overlays
- **JavaScript** – real-time polling, alert logic, breach tracking

> The entire web interface is embedded directly in firmware using `PROGMEM`.

---

## Hardware Setup

| Component | ESP32 Pin |
|---------|-----------|
| MQ-2 Analog Output | GPIO 32 |
| Alert LED | GPIO 26 |

---

## Core Detection Logic

- **Air Quality Calculation**  
  Raw sensor values are normalized into a **percentage-based score** (0–100%).

- **Sudden Spike Detection**  
  Triggers when the difference between consecutive readings exceeds a fixed deviation limit.

- **Danger Threshold Detection**  
  Triggers when air quality exceeds **60%**.

- **Alert Output**  
  The LED turns ON if either condition is met.

---

## HTTP API Endpoints

| Endpoint | Method | Description |
|-------|------|------------|
| `/` | GET | Web dashboard |
| `/login` | POST | Local user authentication |
| `/aqi` | GET | Current air quality score |
| `/status` | GET | Device status info |

All responses are JSON-formatted where applicable.

---

## Project Structure

```
src/
 ├── main.cpp          # Sensor reading, air quality logic, LED control
 └── wifi_server.cpp   # Wi-Fi AP, HTTP server, web dashboard
include/
 └── wifi_server.h
platformio.ini
```

---

## Why This Project Matters

This project demonstrates:
- Embedded **sensor data processing**
- **Real-time system monitoring**
- **Firmware–web integration**
- ESP32 **networking and HTTP server design**
- Clean separation between **hardware logic** and **network/UI logic**

---

## Intended Use

Local air quality and smoke monitoring for:
- Indoor environments
- Embedded systems learning
- IoT dashboard prototyping

(Not intended for internet-facing deployment.)
