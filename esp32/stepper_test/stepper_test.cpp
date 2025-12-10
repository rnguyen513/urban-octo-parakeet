/*
 * Stepper Motor Test for DRV8833 Driver
 *
 * === WIRING INSTRUCTIONS (12-pin DRV8833 Board + ESP32-C6) ===
 *
 * Your DRV8833 board has 12 pins:
 * in1, in2, in3, in4 (inputs), gnd, vcc, eep, out1, out2, out3, out4 (outputs), ult
 *
 * 1. Ground (CRITICAL - Connect First!):
 * - gnd   -> ESP32 GND AND Power Supply GND (common ground required!)
 *
 * 2. Motor Power:
 * - vcc   -> External Power Supply (+) (3V - 10V for motor power)
 *           * This powers the H-bridges for the motor
 *           * Board has onboard regulator for logic power
 *           * DO NOT connect ESP32 3.3V here!
 *
 * 3. Control Pins (4-wire control for bipolar stepper):
 * - in1   -> ESP32 D1 (GPIO 2)  [Controls H-Bridge 1, out1/out2 - Coil A]
 * - in2   -> ESP32 D2 (GPIO 3)  [Controls H-Bridge 1, out1/out2 - Coil A]
 * - in3   -> ESP32 D3 (GPIO 4)  [Controls H-Bridge 2, out3/out4 - Coil B]
 * - in4   -> ESP32 D4 (GPIO 5)  [Controls H-Bridge 2, out3/out4 - Coil B]
 *
 * 4. Sleep Mode Control:
 * - ult   -> ESP32 3.3V (to keep driver awake)
 *           * Pull HIGH (3.3V) for normal operation
 *           * Pull LOW for ultra-low power sleep mode
 *           * Can connect to GPIO pin for software sleep control
 *
 * 5. Enable Pin (if present on your board):
 * - eep   -> Leave unconnected or connect to 3.3V
 *           * Function depends on your specific board variant
 *
 * === SAFETY PRECAUTIONS ===
 *
 * VOLTAGE LIMITS (on vcc pin):
 * - ABSOLUTE MAX: 10V (some boards may support up to 10.8V, check datasheet)
 * - RECOMMENDED: 5V-9V for most small steppers
 * - NEVER exceed max voltage or you WILL destroy the driver!
 *
 * CURRENT LIMITS:
 * - Max continuous: 1.5A per motor channel
 * - Peak: 2A (brief pulses only)
 * - Check your stepper motor's rated current
 * - If motor draws >1.5A, you NEED a different driver (like A4988 or DRV8825)
 *
 * CAPACITOR (REQUIRED):
 * - Place 100uF electrolytic capacitor across vcc and gnd on DRV8833
 * - Place as close as possible to the driver board
 * - This prevents voltage spikes that can destroy the driver
 * - Observe polarity: (-) to gnd, (+) to vcc
 *
 * COMMON GROUND:
 * - ESP32 GND and Power Supply GND MUST be connected
 * - Without common ground, the logic signals won't work properly
 *
 * HOT-PLUGGING:
 * - NEVER connect/disconnect motor while VM is powered
 * - NEVER connect/disconnect power while motor is running
 * - Always power off completely before wiring changes
 *
 * THERMAL MANAGEMENT:
 * - DRV8833 can get HOT during continuous operation
 * - Consider a small heatsink if running >500mA continuously
 * - Allow airflow around the driver board
 *
 * 6. Motor Connections (Bipolar Stepper):
 * - out1 & out2 -> Coil A (H-Bridge 1 outputs)
 * - out3 & out4 -> Coil B (H-Bridge 2 outputs)
 *
 * How to identify coils with multimeter:
 * 1. Measure resistance between all wire pairs
 * 2. Coil A: Two wires with ~few ohms resistance (e.g., Red & Green)
 * 3. Coil B: The other two wires with ~few ohms resistance (e.g., Blue & Yellow)
 * 4. Between coils: Infinite/very high resistance
 *
 * === COMPLETE WIRING SUMMARY ===
 *
 * ESP32 Connections:
 * - D1 (GPIO2) -> in1
 * - D2 (GPIO3) -> in2
 * - D3 (GPIO4) -> in3
 * - D4 (GPIO5) -> in4
 * - 3.3V       -> ult (keep awake)
 * - GND        -> gnd (common ground)
 *
 * Power Supply Connections (3-10V):
 * - (+) -> vcc
 * - (-) -> gnd (shares ground with ESP32)
 *
 * Stepper Motor Connections:
 * - Coil A -> out1 & out2
 * - Coil B -> out3 & out4
 *
 * === DRV8833 vs A4988 Differences ===
 *
 * DRV8833:
 * - 4 input pins (manual stepping sequence)
 * - Lower voltage (2.7V-10.8V)
 * - Lower current (1.5A per channel)
 * - No built-in microstepping
 * - Simpler, cheaper
 *
 * A4988:
 * - 2 input pins (STEP/DIR)
 * - Higher voltage (8V-35V)
 * - Higher current (2A per coil)
 * - Hardware microstepping (1/16 step)
 * - More expensive
 *
 * === C++ Notes ===
 * 1. Must explicitly include <Arduino.h>
 * 2. Full-step sequence: Energizes coils in 4 states
 * 3. This file is compiled as standard C++
 */

#include <Arduino.h>

// Define pin connections for DRV8833
// Using XIAO ESP32C6 D-pin notation (D1=GPIO2, D2=GPIO3, D3=GPIO4, D4=GPIO5)
constexpr int AIN1_PIN = D1;  // -> in1 on DRV8833 (controls out1)
constexpr int AIN2_PIN = D2;  // -> in2 on DRV8833 (controls out2)
constexpr int BIN1_PIN = D3;  // -> in3 on DRV8833 (controls out3)
constexpr int BIN2_PIN = D4;  // -> in4 on DRV8833 (controls out4)

// Motor settings
constexpr int STEPS_PER_REVOLUTION = 200;  // For 1.8° stepper motors

// Full-step sequence for bipolar stepper (4 steps per cycle)
// Each row is: {AIN1, AIN2, BIN1, BIN2}
constexpr int STEP_SEQUENCE[4][4] = {
  {HIGH, LOW,  HIGH, LOW},   // Step 0: Coil A+, Coil B+
  {LOW,  HIGH, HIGH, LOW},   // Step 1: Coil A-, Coil B+
  {LOW,  HIGH, LOW,  HIGH},  // Step 2: Coil A-, Coil B-
  {HIGH, LOW,  LOW,  HIGH}   // Step 3: Coil A+, Coil B-
};

// Current step position
int currentStep = 0;

// Function to execute one step in the given direction
// direction: 1 = clockwise, -1 = counter-clockwise
void stepMotor(int direction) {
  // Update step position based on direction
  currentStep += direction;

  // Wrap around the step sequence (0-3)
  if (currentStep > 3) {
    currentStep = 0;
  } else if (currentStep < 0) {
    currentStep = 3;
  }

  // Apply the current step pattern to the motor pins
  digitalWrite(AIN1_PIN, STEP_SEQUENCE[currentStep][0]);
  digitalWrite(AIN2_PIN, STEP_SEQUENCE[currentStep][1]);
  digitalWrite(BIN1_PIN, STEP_SEQUENCE[currentStep][2]);
  digitalWrite(BIN2_PIN, STEP_SEQUENCE[currentStep][3]);
}

// Function to stop the motor (de-energize all coils)
void stopMotor() {
  digitalWrite(AIN1_PIN, LOW);
  digitalWrite(AIN2_PIN, LOW);
  digitalWrite(BIN1_PIN, LOW);
  digitalWrite(BIN2_PIN, LOW);
}

void setup() {
  // Declare all motor control pins as outputs
  pinMode(AIN1_PIN, OUTPUT);
  pinMode(AIN2_PIN, OUTPUT);
  pinMode(BIN1_PIN, OUTPUT);
  pinMode(BIN2_PIN, OUTPUT);

  // Initialize all pins to LOW (motor off)
  stopMotor();

  Serial.begin(115200);  // ESP32-C6 standard baud rate
  Serial.println("=================================");
  Serial.println("DRV8833 Stepper Motor Test");
  Serial.println("=================================");
  Serial.println("Motor Control Ready!");
  Serial.println();
}

void loop() {
  // Clockwise rotation - slow speed
  Serial.println(">>> Spinning CLOCKWISE (slow)...");

  for (int i = 0; i < STEPS_PER_REVOLUTION; i++) {
    stepMotor(1);  // 1 = clockwise
    delayMicroseconds(4000);  // 4ms between steps = ~4 seconds per revolution
  }

  Serial.println("    Completed 1 revolution CW");
  delay(1000);  // Pause between direction changes

  // Counter-clockwise rotation - medium speed
  Serial.println(">>> Spinning COUNTER-CLOCKWISE (medium)...");

  for (int i = 0; i < STEPS_PER_REVOLUTION; i++) {
    stepMotor(-1);  // -1 = counter-clockwise
    delayMicroseconds(2000);  // 2ms between steps = ~2 seconds per revolution
  }

  Serial.println("    Completed 1 revolution CCW");
  Serial.println();
  delay(1000);  // Pause before next cycle

  // Optional: Uncomment to stop motor between cycles (reduces heat/power)
  // stopMotor();
  // delay(2000);
}
