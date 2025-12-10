# DRV8833 Stepper Motor Wiring Guide

## Quick Reference Pinout

```
ESP32-C6 XIAO          DRV8833 Board
================       =============
3.3V        ---------> VCC
GND         ---------> GND (and to Power Supply GND)
GPIO 2      ---------> AIN1
GPIO 3      ---------> AIN2
GPIO 4      ---------> BIN1
GPIO 5      ---------> BIN2

Power Supply           DRV8833 Board
================       =============
+ (5-9V)    ---------> VM
- (GND)     ---------> GND (and to ESP32 GND)
```

## Detailed Wiring Diagram

```
                                    DRV8833 Driver Board
                                   ┌─────────────────────┐
                                   │                     │
    ESP32-C6                       │   ┌─────────────┐   │
    ┌──────────┐                   │   │   DRV8833   │   │
    │          │                   │   │     IC      │   │
    │  3.3V ───┼───────────────────┼───│ VCC         │   │
    │  GND  ───┼─────────┬─────────┼───│ GND         │   │
    │          │         │         │   │             │   │
    │  GPIO2───┼─────────┼─────────┼───│ AIN1        │   │
    │  GPIO3───┼─────────┼─────────┼───│ AIN2        │   │
    │  GPIO4───┼─────────┼─────────┼───│ BIN1        │   │
    │  GPIO5───┼─────────┼─────────┼───│ BIN2        │   │
    │          │         │         │   │             │   │
    │          │         │         │   │ SLP ─────── │ ──┐ Connect to VCC
    │          │         │         │   │ FLT         │   │ (leave floating)
    └──────────┘         │         │   └─────────────┘   │
                         │         │                     │
                         │         │  Motor Outputs      │
                         │         │  ┌──────────────┐   │
      Power Supply       │         │  │ AOUT1  AOUT2 │───┼─── Coil A ──┐
      ┌──────────┐       │         │  │ BOUT1  BOUT2 │───┼─── Coil B ──┤
      │          │       │         │  └──────────────┘   │             │
      │  + (5-9V)├───────┼─────────┼───│ VM              │     Stepper Motor
      │  - (GND) ├───────┴─────────┼───│ GND             │     (Bipolar)
      │          │                 │   │                 │             │
      └──────────┘                 │   │  [100uF Cap]    │ ─────────────┘
           │                       │   │  + VM / - GND   │
           │                       │   └─────────────────┘
           └─ 100uF Capacitor ─────┘   (Place cap close to
              (+ to VM, - to GND)       VM/GND pins!)
```

## Component List

### Required Components:
1. **ESP32-C6 XIAO** (or similar ESP32 board)
2. **DRV8833 Dual Motor Driver Board**
3. **Bipolar Stepper Motor** (4-wire or 6-wire configured as 4-wire)
   - Rated voltage: 5V-9V recommended
   - Rated current: <1.5A per coil
   - Step angle: 1.8° (200 steps/revolution) typical
4. **External Power Supply**
   - Voltage: 5V-9V (do NOT exceed 10.8V)
   - Current: At least 2x motor rated current
   - Common options: 6V battery pack, 9V wall adapter, bench power supply
5. **100uF Electrolytic Capacitor** (rated for at least 16V)
6. **Jumper Wires** (Male-to-Female recommended)

### Optional Components:
- Small heatsink for DRV8833 (if running >500mA)
- Breadboard for easier prototyping
- Multimeter for testing

## Step-by-Step Wiring Instructions

### Step 1: Power Off Everything
Before starting, ensure:
- ESP32 is disconnected from USB
- Power supply is OFF and disconnected
- All components are on a non-conductive surface

### Step 2: Connect Logic Power
1. Connect DRV8833 **VCC** to ESP32 **3.3V**
2. Connect DRV8833 **GND** to ESP32 **GND**

### Step 3: Connect Control Pins
Connect ESP32 GPIOs to DRV8833 inputs:
1. GPIO 2 → AIN1
2. GPIO 3 → AIN2
3. GPIO 4 → BIN1
4. GPIO 5 → BIN2

### Step 4: Connect Sleep Pin (Optional)
- Connect DRV8833 **SLP** pin to **VCC** (3.3V)
- This keeps the driver awake at all times
- Alternatively, connect to a GPIO pin for software sleep control

### Step 5: Identify Motor Coils
Before connecting the motor, identify which wires belong to which coil:

1. **Using a multimeter (resistance mode):**
   - Measure resistance between all wire pairs
   - Coil A: Two wires with low resistance (~few ohms)
   - Coil B: Other two wires with low resistance
   - Between different coils: Infinite/very high resistance

2. **Common wire colors (not standardized, verify with multimeter!):**
   - Coil A: Red & Green
   - Coil B: Blue & Yellow

3. **For 6-wire motors:**
   - Identify the two center-tap wires (usually white/black)
   - Leave center-taps disconnected (not used)
   - Use the remaining 4 wires as a 4-wire motor

### Step 6: Connect Motor to DRV8833
1. Connect Coil A to AOUT1 and AOUT2 (polarity doesn't matter at this stage)
2. Connect Coil B to BOUT1 and BOUT2 (polarity doesn't matter at this stage)

**Note:** If motor spins backward, swap the wires of ONE coil only.

### Step 7: Add Capacitor to Power Supply
**CRITICAL STEP - DO NOT SKIP!**

1. Identify capacitor polarity:
   - Longer leg = Positive (+)
   - Shorter leg with stripe = Negative (-)
2. Connect capacitor:
   - Positive (+) to power supply positive terminal
   - Negative (-) to power supply negative (GND) terminal
3. Place capacitor as close as possible to where power connects to DRV8833

### Step 8: Connect Motor Power
1. **VERIFY VOLTAGE:** Use multimeter to confirm power supply is 5-9V (MAX 10.8V!)
2. Connect power supply **positive (+)** to DRV8833 **VM**
3. Connect power supply **negative (-)** to DRV8833 **GND**
4. **IMPORTANT:** Also connect power supply GND to ESP32 GND (common ground)

### Step 9: Final Safety Check

Before powering on, verify:
- [ ] All connections are secure
- [ ] No wires are touching/shorting
- [ ] 100uF capacitor is installed with correct polarity
- [ ] Power supply voltage is ≤10.8V
- [ ] Common ground between ESP32 and power supply exists
- [ ] SLP pin is connected to VCC (not floating)
- [ ] Motor is connected (not loose)

### Step 10: Power Up Sequence
1. Connect ESP32 to USB (powers logic)
2. Turn on external power supply (powers motor)
3. Monitor serial output at 115200 baud
4. Motor should step CW then CCW repeatedly

## Troubleshooting

### Motor doesn't move at all:
- Check SLP pin is HIGH (connected to 3.3V)
- Verify common ground between ESP32 and power supply
- Check motor power supply voltage (should be 5-9V)
- Confirm all 4 control pins are connected correctly
- Test DRV8833 with multimeter: measure voltage on AIN1/AIN2/BIN1/BIN2 during operation

### Motor vibrates but doesn't step:
- Motor current may exceed 1.5A (driver can't handle it)
- Try reducing power supply voltage
- Check if motor is mechanically blocked
- Verify step sequence in code matches motor type

### Motor steps backward when it should go forward:
- Swap the two wires of ONE coil only (either Coil A or Coil B, not both)

### Motor is very hot:
- Normal for continuous operation, but concerning if too hot to touch
- Add heatsink to DRV8833
- Reduce power supply voltage
- Add cooling (fan)
- Use `stopMotor()` between movements to de-energize coils

### DRV8833 gets very hot or shuts down:
- Motor may be drawing too much current
- Reduce power supply voltage
- Check for mechanical binding on motor
- Consider upgrading to A4988 or DRV8825 for higher current

### Erratic behavior or resets:
- Insufficient capacitor (add larger capacitor or more capacitors in parallel)
- Power supply can't provide enough current
- Wiring is too long (causing voltage drop)
- Bad/loose connections

### FLT (Fault) pin is LOW:
- Over-current condition detected
- Over-temperature shutdown
- Under-voltage lockout triggered
- Power off and investigate before continuing

## Pin Reference Tables

### ESP32-C6 XIAO Pins Used
| Pin | Function | Connected To |
|-----|----------|--------------|
| 3.3V | Logic Power | DRV8833 VCC |
| GND | Ground | DRV8833 GND & PSU GND |
| GPIO 2 | Motor Control | DRV8833 AIN1 |
| GPIO 3 | Motor Control | DRV8833 AIN2 |
| GPIO 4 | Motor Control | DRV8833 BIN1 |
| GPIO 5 | Motor Control | DRV8833 BIN2 |

### DRV8833 Pin Functions
| Pin | Function | Connected To |
|-----|----------|--------------|
| VCC | Logic Power (3.3V) | ESP32 3.3V |
| GND | Ground | ESP32 GND & PSU GND |
| VM | Motor Power (2.7-10.8V) | Power Supply + |
| AIN1 | H-Bridge A Input 1 | ESP32 GPIO 2 |
| AIN2 | H-Bridge A Input 2 | ESP32 GPIO 3 |
| BIN1 | H-Bridge B Input 1 | ESP32 GPIO 4 |
| BIN2 | H-Bridge B Input 2 | ESP32 GPIO 5 |
| AOUT1 | H-Bridge A Output 1 | Motor Coil A |
| AOUT2 | H-Bridge A Output 2 | Motor Coil A |
| BOUT1 | H-Bridge B Output 1 | Motor Coil B |
| BOUT2 | H-Bridge B Output 2 | Motor Coil B |
| SLP | Sleep Control | 3.3V (or GPIO) |
| FLT | Fault Output | Not connected |

## Advanced: Adding Sleep Control

To save power and reduce heat, you can control the SLP pin with a GPIO:

```cpp
constexpr int SLP_PIN = 6;  // Use any available GPIO

void setup() {
  pinMode(SLP_PIN, OUTPUT);
  digitalWrite(SLP_PIN, HIGH);  // Wake up driver
  // ... rest of setup
}

// To sleep the driver:
void sleepDriver() {
  stopMotor();  // De-energize coils first
  digitalWrite(SLP_PIN, LOW);  // Put driver to sleep
}

// To wake the driver:
void wakeDriver() {
  digitalWrite(SLP_PIN, HIGH);  // Wake up
  delayMicroseconds(1);  // Allow wake-up time
}
```

## Safety Reminder

Always remember:
1. **Never exceed 10.8V on VM pin**
2. **Always use 100uF capacitor on power supply**
3. **Never hot-plug motor connections**
4. **Always connect common ground**
5. **Check motor current rating (<1.5A per coil)**
6. **Monitor temperature during operation**
