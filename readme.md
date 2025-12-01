# Launcha - ESP32-C6 Remote Control System

A remote control system for XIAO ESP32-C6 with Bluetooth and WiFi connectivity options.

## Project Structure

```
launcha/
├── esp32/          # ESP32-C6 Arduino sketches
├── mobile/         # iOS app for Bluetooth control
└── svr/            # Web server (future implementation)
```

## Current Implementation: LED Control via Bluetooth

The initial implementation provides basic LED control of the ESP32-C6's built-in LED via Bluetooth Low Energy (BLE) from an iOS app.

### Components

#### 1. ESP32 Firmware (`/esp32/led_control`)
- BLE server for wireless communication
- Controls built-in LED on GPIO 15
- Device name: "ESP32-LED"
- See `/esp32/led_control/README.md` for details

#### 2. iOS App (`/mobile/LEDControl`)
- Native Swift/SwiftUI application
- CoreBluetooth for BLE communication
- Simple toggle interface for LED control
- See `/mobile/README.md` for details

## Getting Started

### 1. Setup ESP32

1. Open `/esp32/led_control/led_control.ino` in Arduino IDE
2. Select board: "XIAO_ESP32C6"
3. Upload to your ESP32-C6
4. Open Serial Monitor (115200 baud) to verify BLE advertising

### 2. Setup iOS App

1. Open `/mobile/LEDControl.xcodeproj` in Xcode
2. Connect your iPhone
3. Configure signing with your Apple ID
4. Build and run on device (BLE requires physical device)

### 3. Connect and Control

1. Power on ESP32-C6
2. Open LED Control app on iPhone
3. Tap "Scan for Devices"
4. Connect to "ESP32-LED"
5. Use toggle to control LED

## BLE Communication Protocol

- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Characteristic UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- **Commands**:
  - `0x01` = LED ON
  - `0x00` = LED OFF

## Future Plans

- `/svr`: Web server for WiFi-based control
- Additional sensor/actuator support
- Extended control features

## Requirements

### ESP32
- XIAO ESP32-C6
- Arduino IDE with ESP32 board support
- BLE library (included in ESP32 core)

### iOS
- iOS 17.0+
- Xcode 15.0+
- Physical iPhone/iPad (BLE not supported in simulator)
