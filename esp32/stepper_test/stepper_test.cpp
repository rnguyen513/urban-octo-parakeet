/*
 * Standard C++ equivalent of the Stepper Test
 *
 * === WIRING INSTRUCTIONS (A4988 + Arduino Uno) ===
 *
 * 1. A4988 Logic Power:
 * - VDD   -> Arduino 5V
 * - GND   -> Arduino GND
 *
 * 2. Control Pins:
 * - DIR   -> Arduino Pin 2
 * - STEP  -> Arduino Pin 3
 * - SLEEP -> Connect to RESET (Jumper wire between these two pins on the driver)
 *
 * 3. Motor Power (CRITICAL):
 * - VMOT  -> External Power Supply (+) (8V - 35V)
 * - GND   -> External Power Supply (-)
 * * WARNING: Connect a 100uF capacitor across VMOT and GND to protect the driver.
 * * WARNING: NEVER connect/disconnect the motor while VMOT is powered.
 *
 * 4. Motor Coils:
 * - 1A & 1B -> Coil 1
 * - 2A & 2B -> Coil 2
 *
 * === C++ Notes ===
 * 1. Must explicitly include <Arduino.h>
 * 2. Should use function prototypes (if you add helper functions)
 * 3. This file is compiled as standard C++
 */

#include <Arduino.h>

// Define pin connections
// using 'constexpr' is better C++ practice than #define or const int
constexpr int DIR_PIN = 2;
constexpr int STEP_PIN = 3;

// Motor settings
constexpr int STEPS_PER_REVOLUTION = 200;

// In standard C++, setup() and loop() are just regular functions
// that the Arduino core's main() function calls.
void setup() {
  // Declare pins as Outputs
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  Serial.begin(9600);
  Serial.println("Stepper Test Starting (C++ Mode)...");
}

void loop() {
  // Set the spinning direction clockwise
  digitalWrite(DIR_PIN, HIGH);
  Serial.println("Spinning Clockwise...");

  // Spin the motor one revolution slowly
  for (int i = 0; i < STEPS_PER_REVOLUTION; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000); 
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000);
  }

  delay(1000); // Wait a second

  // Set the spinning direction counter-clockwise
  digitalWrite(DIR_PIN, LOW);
  Serial.println("Spinning Counter-Clockwise...");

  // Spin the motor one revolution quickly
  for (int i = 0; i < STEPS_PER_REVOLUTION; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(500); 
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(500);
  }

  delay(1000); // Wait a second
}
