# LED Control iOS App

A native iOS app for controlling the ESP32-C6 LED via Bluetooth Low Energy (BLE).

## Features
- Scan for nearby ESP32-LED devices
- Connect to ESP32 via BLE
- Toggle LED on/off with a switch
- Real-time connection status
- Clean SwiftUI interface

## Requirements
- iOS 17.0 or later
- Xcode 15.0 or later
- Physical iOS device with Bluetooth (BLE not supported in simulator)

## Setup Instructions

1. Open the project in Xcode:
   ```bash
   open LEDControl.xcodeproj
   ```

2. Configure signing:
   - Select the LEDControl target
   - Go to "Signing & Capabilities"
   - Select your development team
   - Xcode will automatically manage provisioning

3. Connect your iPhone via USB

4. Select your iPhone as the build target in Xcode

5. Build and run (Cmd+R)

## Usage

1. Make sure your ESP32-C6 is powered on and running the `led_control` sketch

2. Open the LED Control app on your iPhone

3. Tap "Scan for Devices" - the app will search for the ESP32-LED device

4. Once found, tap on "ESP32-LED" to connect

5. After connection, use the toggle switch to control the LED:
   - ON: LED turns on
   - OFF: LED turns off

6. Tap "Disconnect" to end the BLE connection

## Troubleshooting

- **No devices found**: Make sure the ESP32 is powered on and running the sketch
- **Cannot connect**: Try resetting the ESP32 and scanning again
- **App crashes**: Ensure Bluetooth permissions are granted in Settings > LEDControl
- **BLE not working**: Remember that BLE only works on physical devices, not the simulator

## Project Structure

```
LEDControl/
├── LEDControlApp.swift      # App entry point
├── ContentView.swift         # Main UI with toggle and device list
├── BLEManager.swift          # Bluetooth communication handler
├── Info.plist                # App permissions and configuration
└── Assets.xcassets/          # App icons and colors
```

## BLE Protocol

The app communicates with the ESP32 using these UUIDs:
- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Characteristic UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`

Commands sent:
- `0x01` - Turn LED ON
- `0x00` - Turn LED OFF
