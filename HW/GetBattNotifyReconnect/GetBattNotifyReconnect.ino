#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>

#define READ_INTERVAL 15000                 // Read interval for battery level in ms
#define DEVICE_ADDRESS "F5:BC:47:6B:0B:34"  // Replace with your BLE device's address
#define BATTERY_LEVEL_UUID "2A19"           // Battery Level Characteristic UUID
#define HEART_RATE_MEASUREMENT_UUID "2A37"
#define UNKNOWN_UUID "FEE3"
#define SCAN_TIME 5  // Scan time in seconds

BLEClient* pClient;
BLERemoteCharacteristic* pBatteryLevelCharacteristic;
BLERemoteCharacteristic* pHeartRateCharacteristic;

bool deviceFound = false;
bool doReconnect = false;
bool isConnected = false;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == "ZL02D") {  // Replace with your device's name
      Serial.println("Found target BLE device.");
      BLEDevice::getScan()->stop();  // Stop scanning
      deviceFound = true;
    }
  }
};

void connectToDevice();
void scanBLEDev();

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pClient) {
    Serial.println("Connected to server.");
    isConnected = true;
  }

  void onDisconnect(BLEClient* pClient) {
    Serial.println("Disconnected from server.");
    isConnected = false;
    doReconnect = true;   // Set a flag to attempt reconnection
    deviceFound = false;  // Device must be scanned again

    delay(1000);  // Allow time before attempting reconnection
  }
};

void notifyCallback(BLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  if (length > 0) {
    uint8_t code1 = pData[3];
    uint8_t code2 = pData[4];
    uint8_t value = pData[5];

    if (code1 == 0x08) {  // Blood pressure
      uint8_t sistole = pData[6];
      uint8_t diastole = pData[7];
      Serial.printf("Blood pressure: %d/%d\n", sistole, diastole);
    } else if (code1 == 0x06) {  // SpO2 & BPM
      if (code2 == 0x6b) {       // SpO2
        Serial.printf("SpO2: %d %%\n", value);
      } else if (code2 == 0x6d) {  // BPM
        Serial.printf("Heart Rate: %d bpm\n", value);
      }
    }
  }
}

void connectToDevice() {
  if (deviceFound) {
    Serial.println("Connecting to BLE device...");
    BLEAddress address(DEVICE_ADDRESS);
    pClient = BLEDevice::createClient();  // Create a new client
    pClient->setClientCallbacks(new MyClientCallback());

    if (pClient->connect(address)) {
      Serial.println("Connected to BLE device.");

      // Discover services and characteristics
      BLERemoteService* pService = pClient->getService(BLEUUID("180F"));  // Battery Service UUID
      if (pService != nullptr) {
        pBatteryLevelCharacteristic = pService->getCharacteristic(BATTERY_LEVEL_UUID);
        if (pBatteryLevelCharacteristic != nullptr) {
          Serial.println("Battery Level Characteristic found.");
        }
      }

      BLERemoteService* pService1 = pClient->getService(BLEUUID("FEEA"));  // Measurement service UUID
      if (pService1 != nullptr) {
        pHeartRateCharacteristic = pService1->getCharacteristic(UNKNOWN_UUID);
        if (pHeartRateCharacteristic != nullptr) {
          Serial.println("Measurement Characteristic found.");
          pHeartRateCharacteristic->registerForNotify(notifyCallback);  // Register for notifications
          isConnected = true;
        }
      }
    } else {
      Serial.println("Failed to connect.");
      pClient->disconnect();  // Cleanup on failed connection
      delete pClient;
      delay(5000);
      Serial.println("reinit.");
      scanBLEDev();
      connectToDevice();
    }
  } else {
    Serial.println("Target device not found.");
    delay(5000);
      Serial.println("reinit.");
      scanBLEDev();
      connectToDevice();
  }
}

void scanBLEDev() {
  Serial.println("Starting BLE scan...");
  BLEScan* pScan = BLEDevice::getScan();  // Create a BLE scan object
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true);      // Active scan uses more power but gets results faster
  pScan->start(SCAN_TIME, false);  // Start scanning for SCAN_TIME seconds
}

void setup() {
  Serial.begin(115200);
  BLEDevice::init("");  // Initialize BLE

  scanBLEDev();
  connectToDevice();
}

unsigned long previousMillis = 0;

void loop() {
  unsigned long currentMillis = millis();

  // Read battery level periodically
  if (currentMillis - previousMillis >= READ_INTERVAL) {
    previousMillis = currentMillis;
    if (isConnected) { readBatteryLevel(); }
  }

  // Reconnect logic
  if (doReconnect && !isConnected) {
    Serial.println("Reconnecting...");
    scanBLEDev();  // Scan for devices before attempting reconnection

    if (deviceFound) {
      connectToDevice();
      doReconnect = false;
    } else {
      Serial.println("Device not found. Will retry...");
      delay(5000);  // Delay before retrying
    }
  }
}

void readBatteryLevel() {
  if (isConnected && pBatteryLevelCharacteristic != nullptr) {
    uint8_t batteryLevel = pBatteryLevelCharacteristic->readUInt8();
    if (batteryLevel <= 100) {
      Serial.printf("Battery Level: %d%%\n", batteryLevel);
    } else {
      Serial.printf("Battery is being charged\n");
    }
  }
}
