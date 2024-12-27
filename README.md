# LAMP_Arduino

## Overview
LAMP_Arduino is a project for creating a smart LED lamp controlled by an ESP32 microcontroller. It supports various effects, a real-time clock module (DS3231), and an optional temperature and humidity sensor (DHT11).

## Features
- Wi-Fi configuration through a captive portal
- Multiple LED effects (static colors, rainbow, fire, etc.)
- Real-time clock integration (DS3231) for time-based operations
- Optional temperature and humidity monitoring (DHT11)

## Requirements
### Hardware
- **ESP32**
- **WS2812B LED strip**
- **DS3231** (optional)
- **DHT11** (optional)

### Software
- Arduino IDE or PlatformIO with the required libraries:
  - `FastLED`
  - `ESPAsyncWebServer`
  - `DNSServer`
  - `DHT`
  - `GyverDS3231`
  - `ArduinoJson`
  - `Preferences`

## Wiring Diagram

### WS2812B to ESP32
| WS2812B Pin | ESP32 Pin          |
|-------------|--------------------|
| GND         | GND                |
| VCC         | 5V                |
| Data In     | GPIO12             |

### DS3231 to ESP32
| DS3231 Module Pin | ESP32 Pin          |
|--------------------|--------------------|
| GND               | GND                |
| VCC               | 3.3V               |
| SDA               | GPIO21 (I2C SDA)   |
| SCL               | GPIO22 (I2C SCL)   |

### DHT11 to ESP32
| DHT11 Pin | ESP32 Pin |
|-----------|-----------|
| Data      | GPIO15    |
| GND       | GND       |
| VCC       | 5V        |

## Installation
1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/LAMP_Arduino.git
   
   https://github.com/bobadronov/LAMP_Arduino.git

## TODO:
  Implement encoder management for additional input options.
  Integrate GyverNTP to fetch and synchronize time from an NTP server.
  
