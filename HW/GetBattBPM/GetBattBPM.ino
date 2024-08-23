#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#define READ_INTERVAL 10000
bool isConnected = false;
// Replace with your BLE device's address
#define DEVICE_ADDRESS "F5:BC:47:6B:0B:34"

// Battery Level Characteristic UUID
#define BATTERY_LEVEL_UUID "2A19"
#define HEART_RATE_MEASUREMENT_UUID "2A37"
// Scan time in seconds
#define SCAN_TIME 5

BLEClient* pClient;
BLERemoteCharacteristic* pBatteryLevelCharacteristic;
BLERemoteCharacteristic* pHeartRateCharacteristic;

bool deviceFound = false;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == "ZL02D") {
      Serial.println("Found target BLE device.");
      BLEDevice::getScan()->stop();
      deviceFound = true;
    }
  }
};

void notifyCallback(BLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  if (length > 0) {
    uint8_t heartRate = pData[1];  // Heart rate is usually the second byte
    Serial.printf("Heart Rate: %d bpm\n", heartRate);
  }
}

void connectingBLE() {
  Serial.println("Starting BLE scan...");

  BLEScan* pScan = BLEDevice::getScan();  // Create BLE scan object
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true);      // Active scan uses more power, but gets results faster
  pScan->start(SCAN_TIME, false);  // Scan for SCAN_TIME seconds

  if (deviceFound) {
    // Connect to the BLE device
    BLEAddress address(DEVICE_ADDRESS);
    pClient = BLEDevice::createClient();
    if (pClient->connect(address)) {
      isConnected = true;
      Serial.println("Connected to BLE device.");

      // Discover services and characteristics
      BLERemoteService* pService = pClient->getService(BLEUUID("180F"));  // Battery Service UUID
      if (pService != nullptr) {
        pBatteryLevelCharacteristic = pService->getCharacteristic(BATTERY_LEVEL_UUID);
        if (pBatteryLevelCharacteristic != nullptr) {
          Serial.println("Battery Level Characteristic found.");
        } else {
          Serial.println("Battery Level Characteristic not found.");
        }
      } else {
        Serial.println("Battery Service not found.");
      }

      pService = pClient->getService(BLEUUID("180D"));  // Heart Rate Service UUID
      if (pService != nullptr) {
        pHeartRateCharacteristic = pService->getCharacteristic(HEART_RATE_MEASUREMENT_UUID);
        if (pHeartRateCharacteristic != nullptr) {
          isConnected = true;
          Serial.println("Heart Rate Measurement Characteristic found.");
          pHeartRateCharacteristic->registerForNotify(notifyCallback);  // Register for notifications
        } else {
          Serial.println("Heart Rate Measurement Characteristic not found.");
        }
        //pClient->disconnect();
      } else {
        Serial.println("Heart Rate service not found.");
      }
    }
  }
}
void setup() {
  Serial.begin(115200);
  BLEDevice::init("");  // Initialize BLE
  connectingBLE();
}
unsigned long previousMillis = 0;  // Stores the last time battery level was read

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= READ_INTERVAL) {
    previousMillis = currentMillis;
    if (!isConnected) {
      connectingBLE();
    }
    readBatteryLevel();
  }
}
void readBatteryLevel() {
  {

    uint8_t batteryLevel = pBatteryLevelCharacteristic->readUInt8();
    if (batteryLevel <= 100)
      Serial.printf("Battery Level: %d%%\n", batteryLevel);
    else
      Serial.printf("Battery is being charged\n");
  }
}