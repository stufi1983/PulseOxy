#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#define READ_INTERVAL 15000
bool isConnected = false;
// Replace with your BLE device's address
#define DEVICE_ADDRESS "F5:BC:47:6B:0B:34"

// Battery Level Characteristic UUID
#define BATTERY_LEVEL_UUID "2A19"
#define HEART_RATE_MEASUREMENT_UUID "2A37"
#define UNKNOWN_UUID "FEE3"
// Scan time in seconds
#define SCAN_TIME 5

BLEClient* pClient;
BLERemoteCharacteristic* pBatteryLevelCharacteristic;
BLERemoteCharacteristic* pHeartRateCharacteristic;

bool deviceFound = false;
bool doReconnect = false;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == "ZL02D") {
      Serial.println("Found target BLE device.");
      BLEDevice::getScan()->stop();
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
    doReconnect = true;  // Set a flag to attempt reconnection in the loop
    deviceFound = false;
    // Optionally start reconnection attempts
    while (!isConnected) {
      Serial.println("Attempting to reconnect...");
      scanBLEDev();
      if (deviceFound) {
        Serial.println("Dev found");
        connectToDevice();
      } else {
        Serial.println("Dev not found");
      }
      delay(5000);  // Wait before trying to reconnect
      Serial.println("Wait....");
    }
  }
};


void connectToDevice() {


  if (deviceFound) {
    // Connect to the BLE device
    Serial.println("Connecting to BLE device.");
    BLEAddress address(DEVICE_ADDRESS);
    pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());

    if (pClient->connect(address)) {

      //isConnected = true;
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

      BLERemoteService* pService1 = pClient->getService(BLEUUID("FEEA"));  // Device Notify Service UUID
      if (pService1 != nullptr) {
        pHeartRateCharacteristic = pService1->getCharacteristic(UNKNOWN_UUID);
        if (pHeartRateCharacteristic != nullptr) {
          isConnected = true;
          Serial.println("Measurement Characteristic found.");
          pHeartRateCharacteristic->registerForNotify(notifyCallback);  // Register for notifications
        } else {
          Serial.println("Measurement Characteristic not found.");
        }
        //pClient->disconnect();
      } else {
        Serial.println("Measurement service not found.");
      }
    } else {
      Serial.println("Failed to connect.");
    }
  } else {
    Serial.println("Target device not found.");
    isConnected = false;
    doReconnect = true;
  }
}




void notifyCallback(BLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  if (length > 0) {
    unsigned long ttime = millis() / 1000;
    Serial.print(ttime);
    uint8_t code1 = pData[3];  // Heart rate is usually the second byte
    uint8_t code2 = pData[4];  // Heart rate is usually the second byte
    uint8_t value = pData[5];  // Heart rate is usually the second byte

    if (code1 == 0x08) {  //Blood pressure
      uint8_t sistole = pData[6];
      uint8_t diastole = pData[7];
      Serial.printf("Blood pressure: %d/%d\n", sistole, diastole);
    } else if (code1 == 0x06) {  //SPO2 & BPM
      if (code2 == 0x6b) {       //spo2
        Serial.printf("SPO2: %d %%\n", value);

      } else if (code2 == 0x6d) {  //bpm
        Serial.printf("Heart Rate: %d bpm\n", value);
      }
    }
  }
}

void scanBLEDev() {
  Serial.println("Starting BLE scan...");
  BLEScan* pScan = BLEDevice::getScan();  // Create BLE scan object
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true);      // Active scan uses more power, but gets results faster
  pScan->start(SCAN_TIME, false);  // Scan for SCAN_TIME seconds
}

void setup() {
  Serial.begin(115200);
  BLEDevice::init("");  // Initialize BLE

  scanBLEDev();

  connectToDevice();
}
unsigned long previousMillis = 0;  // Stores the last time battery level was read

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= READ_INTERVAL) {
    previousMillis = currentMillis;
    // if (!isConnected) {
    //   connectToDevice();
    // }
    if (isConnected) { readBatteryLevel(); }
  }

  if (deviceFound) {
    if (doReconnect && !isConnected) {
      Serial.println("Reconnecting...");

      // Delete and recreate BLE client
      //delete pClient;
      //connectToDevice();

      doReconnect = false;  // Reset the reconnection flag
    }
  } else {
    Serial.println("Devi not found");
     scanBLEDev();
     delay(5000);
  }
}
void readBatteryLevel() {
  {
    unsigned long ttime = millis() / 1000;
    Serial.print(ttime);
    uint8_t batteryLevel = pBatteryLevelCharacteristic->readUInt8();
    if (batteryLevel <= 100)
      Serial.printf("Battery Level: %d%%\n", batteryLevel);
    else
      Serial.printf("Battery is being charged\n");
  }
}