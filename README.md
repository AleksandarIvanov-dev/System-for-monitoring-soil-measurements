# System for monitoring soil measurements.

This project is designed to read temperature, humidity, and electrical conductivity (EC) data from sensors and publish the readings to an MQTT broker using an ESP8266 microcontroller.

## Features

- Reads temperature, humidity, and EC data from sensors.
- Connects to a WiFi network.
- Publishes sensor readings to an MQTT broker.
- ESP8266 enters a deep sleep mode to save power between readings.

## Requirements

- ESP8266 microcontroller
- Sensor 3 in 1
- Module Max485
- Arduino IDE
- Required Libraries:
  - `ESP8266WiFi.h`
  - `Arduino.h`
  - `PubSubClient.h`
  - `ArduinoJson.h`
  - `SoftwareSerial.h`

## Wiring

### ESP8266
  - D2 (GPIO4) is connected to DE and RE of Max485.
  - D4 (GPIO2) is connected to DI of Max485.
  - 3v3 is connected to Vcc of Max485.
  - GND is connected to GND of Max485.
  - D5 (GPIO14) is connected to R0 of Max485.

### Sensor

  - A from the sensor is connected to A of Max485.
  - B from the sensor is connected to B of Max485.

![image](https://github.com/user-attachments/assets/8bdaaf27-8aae-494c-9809-70c21d543957)

### Code Configuration

1. **WiFi Credentials**: Update the `ssid` and `pass` variables with your WiFi network credentials:
    ```cpp
    const char* ssid = "YOUR_SSID";
    const char* pass = "YOUR_PASSWORD";
    ```

2. **MQTT Broker Settings**: Update the `mqttServer`, `mqttUserName`, `mqttPassword`, and `port` variables with your MQTT broker details:
    ```cpp
    const char* mqttServer = "YOUR_MQTT_SERVER";
    const char* mqttUserName = "YOUR_MQTT_USERNAME";
    const char* mqttPassword = "YOUR_MQTT_PASSWORD";
    int port = YOUR_MQTT_PORT;
    ```

3. **MQTT Topic**: Set the MQTT topic for publishing sensor readings:
    ```cpp
    const char topic[]  = "sensor/readings";
    ```
