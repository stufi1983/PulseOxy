#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#define READ_INTERVAL 10000
bool isConnected = false;
// Replace with your BLE device's address
#define DEVICE_ADDRESS "F5:BC:47:6B:0B:34"

// Battery Level Characteristic UUID
#define BATTERY_LEVEL_UUID "FEE1"  //read
#define HEART_RATE_MEASUREMENT_UUID "FEE3"  //notify
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
    Serial.printf("nOTIFY: %x bpm\n", pData[0]);
    Serial.printf("nOTIFY: %x bpm\n", pData[1]);
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
      BLERemoteService* pService = pClient->getService(BLEUUID("FEEA"));  // Battery Service UUID
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

      pService = pClient->getService(BLEUUID("FEEA"));  // Heart Rate Service UUID
      if (pService != nullptr) {
        pHeartRateCharacteristic = pService->getCharacteristic(HEART_RATE_MEASUREMENT_UUID);
        if (pHeartRateCharacteristic != nullptr) {
          isConnected = true;
          Serial.println("Heart Rate Measurement Characteristic found.");
          pHeartRateCharacteristic->registerForNotify(notifyCallback);  // Register for notifications
          //pHeartRateCharacteristic->registerForIndications(indicateCallback);  // Register for notifications
          
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
if(pBatteryLevelCharacteristic==NULL) return;
    uint8_t batteryLevel = pBatteryLevelCharacteristic->readUInt8();
    
      Serial.printf("rEAD: %d%%\n", batteryLevel);
   
  }
}

/*
 : 
A4CF12FF8DBD:
Loket1:A000

nOTIFY: fe bpm
nOTIFY: ea bpm

3ffe1e9c
109/71

3ffe1e9c
112/70

feea20086900 6e 44
110/68

feea20086900 70 46
112/70

feea20086900 6d 49
109/73


Blood Pressure
feea20 08 704b 112/75 70--> 112, 4b-->75

feea20 08 74 47 116/71 74--> 116, 47-->71


SPO2
feea20 06 6b 62   62-->98
feea20 06 6b 61   61-->97

BPM
feea20 06 6d 51   51-->81

*/
