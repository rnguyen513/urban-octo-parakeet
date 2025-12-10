#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE UUIDs
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BATTERY_CHARACTERISTIC_UUID "a1b2c3d4-5678-90ab-cdef-1234567890ab"
#define STEPPER_CHARACTERISTIC_UUID "c1d2e3f4-5678-90ab-cdef-1234567890cd"

// Built-in LED pin for XIAO ESP32C6
#define LED_PIN 15

// Battery monitoring - External 200k+200k voltage divider (2:1) for LiPo battery
// A0 pin connected to voltage divider midpoint
#define BATTERY_UPDATE_INTERVAL 5000  // Update battery level every 5 seconds

// Stepper motor pins (DRV8833 driver)
// D1-D4 map to GPIO 2-5 on XIAO ESP32C6
constexpr int AIN1_PIN = D1;  // -> in1 on DRV8833 (controls out1)
constexpr int AIN2_PIN = D2;  // -> in2 on DRV8833 (controls out2)
constexpr int BIN1_PIN = D3;  // -> in3 on DRV8833 (controls out3)
constexpr int BIN2_PIN = D4;  // -> in4 on DRV8833 (controls out4)

// Stepper motor settings
constexpr int STEPS_PER_REVOLUTION = 200;  // For 1.8° stepper motors
constexpr float DEGREES_PER_STEP = 360.0 / STEPS_PER_REVOLUTION;  // 1.8°

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

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pBatteryCharacteristic = NULL;
BLECharacteristic* pStepperCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long lastBatteryUpdate = 0;

// Stepper motor control functions
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

void stopMotor() {
  digitalWrite(AIN1_PIN, LOW);
  digitalWrite(AIN2_PIN, LOW);
  digitalWrite(BIN1_PIN, LOW);
  digitalWrite(BIN2_PIN, LOW);
}

void rotateDegrees(float degrees, int direction) {
  // Convert degrees to steps
  int steps = (int)((degrees / DEGREES_PER_STEP) + 0.5);  // Round to nearest step

  Serial.print("Rotating ");
  Serial.print(degrees);
  Serial.print("° (");
  Serial.print(steps);
  Serial.print(" steps) ");
  Serial.println(direction == 1 ? "CLOCKWISE" : "COUNTER-CLOCKWISE");

  // Execute the rotation
  for (int i = 0; i < steps; i++) {
    stepMotor(direction);
    delayMicroseconds(4000);  // 4ms between steps for smooth operation
  }

  // De-energize coils to reduce heat and power consumption
  stopMotor();

  Serial.println("Rotation complete");
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.print("Received LED value: ");

        // Read the first byte to determine LED state
        uint8_t ledState = value[0];

        if (ledState == 1) {
          digitalWrite(LED_BUILTIN, LOW);  // Inverted: LOW = ON
          Serial.println("LED ON");
        } else if (ledState == 0) {
          digitalWrite(LED_BUILTIN, HIGH);  // Inverted: HIGH = OFF
          Serial.println("LED OFF");
        }
      }
    }
};

class StepperCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String value = pCharacteristic->getValue();

      // Expected format: 3 bytes
      // Byte 0-1: degrees (uint16_t) in little-endian
      // Byte 2: direction (1 = CW, 0 = CCW)
      if (value.length() >= 3) {
        uint16_t degrees = (uint8_t)value[0] | ((uint8_t)value[1] << 8);
        uint8_t direction = (uint8_t)value[2];

        Serial.print("Received stepper command: ");
        Serial.print(degrees);
        Serial.print("° in direction ");
        Serial.println(direction);

        // Clamp degrees to valid range (0-360)
        if (degrees > 360) {
          degrees = 360;
        }

        // Convert direction byte to motor direction
        int motorDirection = (direction == 1) ? 1 : -1;

        // Execute rotation
        rotateDegrees(degrees, motorDirection);
      } else {
        Serial.println("Invalid stepper command format");
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE LED & Stepper Control");

  // Initialize LED pin (inverted: HIGH = OFF)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Initialize ADC pin for battery monitoring
  pinMode(A0, INPUT);

  // Initialize stepper motor pins
  pinMode(AIN1_PIN, OUTPUT);
  pinMode(AIN2_PIN, OUTPUT);
  pinMode(BIN1_PIN, OUTPUT);
  pinMode(BIN2_PIN, OUTPUT);
  stopMotor();  // Start with motor de-energized

  // Create the BLE Device
  BLEDevice::init("ad99df9");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Add a descriptor for notifications
  pCharacteristic->addDescriptor(new BLE2902());

  // Set the callback
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Create battery characteristic (read + notify)
  pBatteryCharacteristic = pService->createCharacteristic(
                             BATTERY_CHARACTERISTIC_UUID,
                             BLECharacteristic::PROPERTY_READ |
                             BLECharacteristic::PROPERTY_NOTIFY
                           );
  pBatteryCharacteristic->addDescriptor(new BLE2902());

  // Create stepper motor characteristic (write only)
  pStepperCharacteristic = pService->createCharacteristic(
                             STEPPER_CHARACTERISTIC_UUID,
                             BLECharacteristic::PROPERTY_WRITE
                           );
  pStepperCharacteristic->setCallbacks(new StepperCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();

  Serial.println("BLE advertising started");
  Serial.println("Waiting for a client connection...");
}

float readBatteryVoltage() {
  // Take multiple readings and average for accuracy (removes spike errors)
  uint32_t Vbatt = 0;
  for(int i = 0; i < 16; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(A0); // ADC with factory calibration
  }

  // Calculate voltage: average mV reading, compensate for 2:1 divider, convert to V
  float Vbattf = 2 * Vbatt / 16 / 1000.0;
  return Vbattf;
}

void loop() {
  // Handle disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }

  // Handle connecting
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  // Update battery level periodically when connected
  if (deviceConnected && (millis() - lastBatteryUpdate >= BATTERY_UPDATE_INTERVAL)) {
    float batteryVoltage = readBatteryVoltage();

    // Send voltage as string (e.g., "3.85")
    char batteryStr[8];
    snprintf(batteryStr, sizeof(batteryStr), "%.2f", batteryVoltage);

    pBatteryCharacteristic->setValue(batteryStr);
    pBatteryCharacteristic->notify();

    Serial.print("Battery voltage: ");
    Serial.print(batteryVoltage);
    Serial.println("V");

    lastBatteryUpdate = millis();
  }

  delay(100);
}
