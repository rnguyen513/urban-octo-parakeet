/*
 * Stepper Motor Test for DRV8833 Driver
 *
 * === WIRING INSTRUCTIONS (DRV8833 + ESP32-C6) ===
 *
 * 1. DRV8833 Logic Power:
 * - VCC   -> ESP32 3.3V
 * - GND   -> ESP32 GND
 *
 * 2. Control Pins (4-wire control for bipolar stepper):
 * - AIN1  -> ESP32 GPIO 2
 * - AIN2  -> ESP32 GPIO 3
 * - BIN1  -> ESP32 GPIO 4
 * - BIN2  -> ESP32 GPIO 5
 *
 * 3. Optional Control Pins:
 * - SLP   -> ESP32 3.3V (or GPIO pin for sleep control)
 *           * Connect to 3.3V to keep driver awake
 *           * Pull LOW to put driver in sleep mode (saves power)
 *
 * 4. Motor Power (CRITICAL - READ CAREFULLY):
 * - VM    -> External Power Supply (+) (2.7V - 10.8V max!)
 * - GND   -> External Power Supply (-) AND ESP32 GND (common ground!)
 *
 * === SAFETY PRECAUTIONS ===
 *
 * VOLTAGE LIMITS:
 * - ABSOLUTE MAX: 10.8V on VM pin
 * - RECOMMENDED: 5V-9V for most small steppers
 * - NEVER exceed 10.8V or you WILL destroy the chip!
 *
 * CURRENT LIMITS:
 * - Max continuous: 1.5A per motor channel
 * - Peak: 2A (brief pulses only)
 * - Check your stepper motor's rated current
 * - If motor draws >1.5A, you NEED a different driver (like A4988 or DRV8825)
 *
 * CAPACITOR (REQUIRED):
 * - Place 100uF electrolytic capacitor across VM and GND
 * - Place as close as possible to DRV8833 VM/GND pins
 * - This prevents voltage spikes that can destroy the driver
 * - Observe polarity: (-) to GND, (+) to VM
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
 * 5. Motor Connections (Bipolar Stepper):
 * - AOUT1 & AOUT2 -> Coil A (e.g., Red & Green wires)
 * - BOUT1 & BOUT2 -> Coil B (e.g., Blue & Yellow wires)
 *
 * How to identify coils with multimeter:
 * 1. Measure resistance between all wire pairs
 * 2. Coil A: Two wires with ~few ohms resistance
 * 3. Coil B: The other two wires with ~few ohms resistance
 * 4. Between coils: Infinite/very high resistance
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
constexpr int AIN1_PIN = 2;  // Control H-Bridge A, Input 1
constexpr int AIN2_PIN = 3;  // Control H-Bridge A, Input 2
constexpr int BIN1_PIN = 4;  // Control H-Bridge B, Input 1
constexpr int BIN2_PIN = 5;  // Control H-Bridge B, Input 2

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
    delayMicroseconds(2000);  // 2ms between steps = ~2 seconds per revolution
  }

  Serial.println("    Completed 1 revolution CW");
  delay(1000);  // Pause between direction changes

  // Counter-clockwise rotation - fast speed
  Serial.println(">>> Spinning COUNTER-CLOCKWISE (fast)...");

  for (int i = 0; i < STEPS_PER_REVOLUTION; i++) {
    stepMotor(-1);  // -1 = counter-clockwise
    delayMicroseconds(1000);  // 1ms between steps = ~1 second per revolution
  }

  Serial.println("    Completed 1 revolution CCW");
  Serial.println();
  delay(1000);  // Pause before next cycle

  // Optional: Uncomment to stop motor between cycles (reduces heat/power)
  // stopMotor();
  // delay(2000);
}
