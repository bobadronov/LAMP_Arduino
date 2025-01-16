# LAMP_Arduino

A smart LED lamp controlled by an ESP32 microcontroller, offering various lighting effects and sensor integrations. The project uses Kotlin Multiplatform for the app, providing cross-platform support across Android, Windows, and iOS.

**App repository:** [LAMP_Multiplatform](https://github.com/bobadronov/LAMP_Multiplatform)

## Tested Platforms

| Platform  | Status               |
|-----------|----------------------|
| Android   | ✅ Tested             |
| Windows   | ✅ Tested             |
| MacOS     | ⚠️ Not tested         |
| iOS       | ⚠️ Not tested         |
| Linux     | ⚠️ Not tested         |

## Overview
LAMP_Arduino is a project for creating a smart LED lamp controlled by an ESP32 microcontroller. It supports various effects, integrates with a real-time clock module (DS3231), and optionally supports a temperature and humidity sensor (DHT11).

## Features
- Wi-Fi configuration through a captive portal
- Wireless controll via WiFi on Android, iOS, Windows
- Multiple LED effects (static colors, rainbow, fire, etc.)
- Real-time clock integration (DS3231) for time-based operations
- Optional temperature and humidity monitoring (DHT11)
- Button support for additional input options
- NTP synchronization for accurate timekeeping

## Requirements

### Hardware
- **ESP32**
- **WS2812B LED strip**
- **DS3231** (optional)
- **DHT11** (optional)


## Wiring Diagram

### WS2812B to ESP32
| WS2812B Pin | ESP32 Pin          |
|-------------|--------------------|
| GND         | GND                |
| VCC         | 5V                 |
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
   git clone https://github.com/bobadronov/LAMP_Arduino.git