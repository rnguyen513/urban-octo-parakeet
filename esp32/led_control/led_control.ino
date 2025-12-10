#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE UUIDs
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BATTERY_CHARACTERISTIC_UUID "a1b2c3d4-5678-90ab-cdef-1234567890ab"

// Built-in LED pin for XIAO ESP32C6
#define LED_PIN 15

// Battery monitoring - External 200k+200k voltage divider (2:1) for LiPo battery
// A0 pin connected to voltage divider midpoint
#define BATTERY_UPDATE_INTERVAL 5000  // Update battery level every 5 seconds

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pBatteryCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long lastBatteryUpdate = 0;

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
        Serial.print("Received value: ");

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

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE LED Control");

  // Initialize LED pin (inverted: HIGH = OFF)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Initialize ADC pin for battery monitoring
  pinMode(A0, INPUT);

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
