# Stepper Motor Test - DRV8833 Driver

This sketch tests a bipolar stepper motor using the DRV8833 dual H-bridge motor driver with an ESP32-C6.

## Quick Start

1. **Read [SAFETY.md](SAFETY.md) first** - Contains critical warnings about voltage limits and safety precautions
2. **Follow [WIRING.md](WIRING.md)** - Detailed wiring instructions with diagrams
3. **Upload the sketch** using the command below
4. **Monitor serial output** at 115200 baud

## Uploading the Sketch

From the `/esp32` directory:

```bash
# Compile
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C6 stepper_test

# Upload (adjust port as needed)
arduino-cli upload -p /dev/cu.usbmodem* --fqbn esp32:esp32:XIAO_ESP32C6 stepper_test

# Monitor serial output
arduino-cli monitor -p /dev/cu.usbmodem* -c baudrate=115200
```

## What This Sketch Does

The test program continuously runs the motor through this sequence:

1. **Clockwise rotation** - One full revolution (slow speed, 2ms per step)
2. **1 second pause**
3. **Counter-clockwise rotation** - One full revolution (fast speed, 1ms per step)
4. **1 second pause**
5. **Repeat**

## Expected Serial Output

```
=================================
DRV8833 Stepper Motor Test
=================================
Motor Control Ready!

>>> Spinning CLOCKWISE (slow)...
    Completed 1 revolution CW
>>> Spinning COUNTER-CLOCKWISE (fast)...
    Completed 1 revolution CCW

>>> Spinning CLOCKWISE (slow)...
    Completed 1 revolution CW
...
```

## Pin Configuration

| ESP32 Pin | DRV8833 Pin | Function |
|-----------|-------------|----------|
| GPIO 2 | AIN1 | H-Bridge A, Input 1 |
| GPIO 3 | AIN2 | H-Bridge A, Input 2 |
| GPIO 4 | BIN1 | H-Bridge B, Input 1 |
| GPIO 5 | BIN2 | H-Bridge B, Input 2 |
| 3.3V | VCC | Logic power |
| GND | GND | Ground (also connect to power supply GND) |

**Motor Power:**
- Power Supply (+) → DRV8833 VM (5-9V recommended, 10.8V absolute max)
- Power Supply (-) → DRV8833 GND (must share common ground with ESP32)

**Critical:** 100uF capacitor required across VM and GND!

## Motor Requirements

This driver is suitable for:
- **Voltage:** 5-9V nominal motors
- **Current:** Less than 1.5A per coil
- **Type:** Bipolar stepper motors (4-wire or 6-wire configured as 4-wire)
- **Step angle:** Any (code assumes 1.8° / 200 steps per revolution)

**Not suitable for:**
- NEMA 17 motors rated >1.5A per coil
- Motors requiring >10.8V
- Unipolar stepper motors (5-wire)

## Customization

### Changing Step Speed

Edit the delay values in `stepper_test.cpp:165` and `stepper_test.cpp:176`:

```cpp
// Slower: increase delay (in microseconds)
delayMicroseconds(5000);  // 5ms per step = very slow

// Faster: decrease delay
delayMicroseconds(500);   // 0.5ms per step = fast

// WARNING: Too fast causes missed steps!
// Most small steppers: 500-2000 microseconds works well
```

### Changing Motor Pins

Edit pin definitions in `stepper_test.cpp:93-96`:

```cpp
constexpr int AIN1_PIN = 2;  // Change to your GPIO
constexpr int AIN2_PIN = 3;
constexpr int BIN1_PIN = 4;
constexpr int BIN2_PIN = 5;
```

### Changing Steps Per Revolution

If your motor has different step angle:

```cpp
// 0.9° motor (400 steps/revolution)
constexpr int STEPS_PER_REVOLUTION = 400;

// 7.5° motor (48 steps/revolution)
constexpr int STEPS_PER_REVOLUTION = 48;
```

### Adding Sleep Control

To reduce power consumption and heat between movements, uncomment the sleep code at the end of `loop()`:

```cpp
// Optional: Uncomment to stop motor between cycles
stopMotor();    // De-energize coils
delay(2000);    // Wait 2 seconds before next cycle
```

Or implement GPIO-controlled sleep (see [WIRING.md](WIRING.md#advanced-adding-sleep-control) for details).

## Troubleshooting

### Motor doesn't move
- Check SLP pin is connected to VCC (3.3V)
- Verify common ground between ESP32 and power supply
- Confirm motor power supply is 5-9V
- Test pins with multimeter during operation

### Motor vibrates but doesn't rotate
- Motor may require too much current (>1.5A)
- Try lowering power supply voltage
- Check if motor is mechanically blocked

### Motor spins backward
- Swap the two wires of ONE coil (either Coil A or Coil B, not both)

### DRV8833 gets very hot
- Normal to be warm, but if too hot to touch:
  - Add heatsink
  - Reduce voltage
  - Use `stopMotor()` when idle
  - Check motor current rating

### Erratic behavior
- Insufficient capacitor (add larger/more capacitors)
- Power supply can't provide enough current
- Loose connections

See [SAFETY.md](SAFETY.md#troubleshooting-safety-issues) for detailed troubleshooting.

## Code Structure

### Main Functions

- `stepMotor(int direction)` - Executes one step in the given direction
  - `direction = 1` → Clockwise
  - `direction = -1` → Counter-clockwise

- `stopMotor()` - De-energizes all coils (reduces heat and power)

- `setup()` - Initializes GPIO pins and serial communication

- `loop()` - Runs continuous CW/CCW test pattern

### Step Sequence

The code uses a 4-step full-step sequence:

```cpp
constexpr int STEP_SEQUENCE[4][4] = {
  {HIGH, LOW,  HIGH, LOW},   // Step 0: A+, B+
  {LOW,  HIGH, HIGH, LOW},   // Step 1: A-, B+
  {LOW,  HIGH, LOW,  HIGH},  // Step 2: A-, B-
  {HIGH, LOW,  LOW,  HIGH}   // Step 3: A+, B-
};
```

This energizes the motor coils in sequence to create rotation.

## Safety Reminders

- ⚠️ **Never exceed 10.8V** on VM pin (will destroy chip)
- ⚠️ **Always use 100uF capacitor** on power supply
- ⚠️ **Check motor current** (<1.5A per coil)
- ⚠️ **Never hot-plug** motor connections
- ⚠️ **Connect common ground** (ESP32 GND ↔ PSU GND)

Read [SAFETY.md](SAFETY.md) for complete safety information.

## Files in This Directory

- `stepper_test.cpp` - Main Arduino sketch (C++ implementation)
- `stepper_test.ino` - Arduino sketch wrapper (minimal/empty)
- `README.md` - This file (overview and quick reference)
- `WIRING.md` - Detailed wiring diagrams and connection instructions
- `SAFETY.md` - Critical safety precautions and warnings

## Differences from A4988 Version

The previous version of this sketch used an A4988 stepper driver. Key differences:

| Feature | A4988 | DRV8833 |
|---------|-------|---------|
| Control Interface | 2 pins (STEP/DIR) | 4 pins (AIN1/2, BIN1/2) |
| Voltage Range | 8V - 35V | 2.7V - 10.8V |
| Current Rating | 2A per coil | 1.5A per coil |
| Microstepping | Yes (up to 1/16) | No (full-step only) |
| Code Complexity | Simpler | Manual step sequencing |
| Price | Higher | Lower |

## Next Steps

After verifying basic operation:

1. Experiment with different speeds (adjust delay values)
2. Try reversing motor direction (modify step sequence direction)
3. Implement acceleration/deceleration profiles
4. Add potentiometer for speed control
5. Integrate with BLE control (see `/mobile` directory)
6. Implement sleep mode for power savings

## Resources

- [DRV8833 Datasheet](https://www.ti.com/lit/ds/symlink/drv8833.pdf) - Texas Instruments official datasheet
- [ESP32-C6 Pinout](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/) - XIAO ESP32-C6 documentation
- [Stepper Motor Basics](https://www.youtube.com/watch?v=09Mpkjcr0bo) - Understanding how stepper motors work

## License

This code is provided as-is for educational purposes. See [SAFETY.md](SAFETY.md) for important disclaimers.

---

**Hardware:** ESP32-C6 XIAO + DRV8833 + Bipolar Stepper Motor
**Last Updated:** 2025-12-07
**Version:** 2.0 (DRV8833)
