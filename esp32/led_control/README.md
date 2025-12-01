# ESP32-C6 BLE LED Control

## Overview
This sketch enables BLE (Bluetooth Low Energy) communication for controlling the built-in LED on the XIAO ESP32C6.

## Hardware
- XIAO ESP32C6
- Built-in LED on GPIO 15

## BLE Service Details
- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Characteristic UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- **Device Name**: `ESP32-LED`

## Protocol
Write a single byte to the characteristic:
- `0x01` - Turn LED ON
- `0x00` - Turn LED OFF

## Upload Instructions
1. Open `led_control.ino` in Arduino IDE
2. Select Board: "XIAO_ESP32C6"
3. Select the correct serial port
4. Click Upload

## Serial Monitor
Open Serial Monitor at 115200 baud to see connection status and LED state changes.
