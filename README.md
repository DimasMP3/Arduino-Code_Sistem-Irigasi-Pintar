# ESP8266 Smart Soil Moisture Sensor & Pump Control

This project uses an ESP8266 (NodeMCU/Wemos D1 Mini) to monitor soil moisture levels and control a water pump automatically or manually via MQTT.

## Features

- **Real-time Monitoring**: Reads soil moisture levels and sends data to an MQTT broker.
- **Automatic Mode**: Automatically turns on the water pump when moisture is below **60%**.
- **Manual Control**: Control the pump remotely via MQTT commands.
- **Secure Connection**: Uses MQTT over TLS (MQTTS) for secure communication.
- **Status Feedback**: Publishes the current pump status (ON/OFF) to MQTT.

## Hardware Requirements

- ESP8266 Development Board (e.g., NodeMCU, Wemos D1 Mini)
- Capacitive Soil Moisture Sensor (Analog)
- 5V Relay Module (Active Low recommended)
- Water Pump & Power Supply
- Jumper Wires

## Pin Configuration

| Component | ESP8266 Pin | Description |
|-----------|-------------|-------------|
| Soil Sensor | A0 | Analog Output from sensor |
| Relay Module | D0 (GPIO 16) | Control pin for the pump |

> **Note**: Calibration values in the code (`soilWet = 350`, `soilDry = 950`) may need adjustment depending on your specific sensor and soil type.

## Software Requirements

- [Arduino IDE](https://www.arduino.cc/en/software)
- ESP8266 Board Support Package installed in Arduino IDE.
- **Libraries**:
  - `PubSubClient` by Nick O'Leary
  - `ESP8266WiFi` (Built-in)
  - `WiFiClientSecure` (Built-in)

## Configuration

Before uploading, edit `main.ino` and update the following variables:

1. **WiFi Credentials**:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* pass = "YOUR_WIFI_PASSWORD";
   ```

2. **MQTT Broker Settings**:
   ```cpp
   const char* mqtt_server = "YOUR_MQTT_BROKER_URL";
   const int mqtt_port = 8883; // Default for MQTTS
   const char* mqtt_user = "YOUR_MQTT_USERNAME";
   const char* mqtt_pass = "YOUR_MQTT_PASSWORD";
   const char* client_id = "YOUR_CLIENT_ID";
   ```

## MQTT Topics

| Topic | Type | Description |
|-------|------|-------------|
| `sensor/tanah/data` | Publish | Sends JSON data: `{"persen": 45}` |
| `kontrol/pompa` | Subscribe | Receives commands: `MANUAL_ON`, `MANUAL_OFF`, `AUTO` |
| `pompa/status` | Publish | Sends pump status: `ON` or `OFF` |

## How It Works

1. The ESP8266 connects to WiFi and the HiveMQ (or other) MQTT broker.
2. Every second, it reads the soil moisture value from pin A0.
3. It converts the raw analog value into a percentage (0-100%).
4. **Auto Mode** (Default):
   - If moisture < 60%, the pump turns **ON**.
   - If moisture >= 60%, the pump turns **OFF**.
5. **Manual Mode**:
   - Send `MANUAL_ON` to `kontrol/pompa` to force the pump ON.
   - Send `MANUAL_OFF` to force the pump OFF.
   - Send `AUTO` to revert to automatic control based on sensor readings.
