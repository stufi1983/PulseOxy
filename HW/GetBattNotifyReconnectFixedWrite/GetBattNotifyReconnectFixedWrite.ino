#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>

#define READ_INTERVAL 60000  //more than 30000 (time for measurement)

#define DEVICE_ADDRESS "f5:bc:47:6b:0b:34"  //f5:bc:47:6b:0b:34 --> convert to string became lowercase

#define BATTERY_SERVICE_UUID "180F"
#define BATTERY_LEVEL_UUID "2A19"

#define MEASUREMENT_SERVICE_UUID "FEEA"
#define MEASUREMENT_NOTIFICATION_UUID "FEE3"

#define HEART_RATE_MEASUREMENT_UUID "2A37"

#define SCAN_TIME 5
#define SCAN_TIMEOUT 15000      // Maximum scan duration in milliseconds (15 seconds)
#define MEASUREMENT_INTERVAL 3  // in minutes

#define REQUEST_SERVICE_UUID "FEEA"
#define REQUEST_CHARACTERISTIC_UUID "FEE2"

BLEClient* pClient;
BLERemoteCharacteristic* pCharacteristic;
BLERemoteCharacteristic* pBatteryLevelCharacteristic;
BLERemoteCharacteristic* pHeartRateCharacteristic;
BLERemoteCharacteristic* pHeartRateNotify;

bool deviceFound = false;
bool doReconnect = false;
bool isConnected = false;

bool inbody = false;
uint8_t MICounter = 0;  //MEASUREMENT_INTERVAL counter

unsigned long previousMillis = 0;
uint8_t curBatteryLevel = 0;
uint8_t curSistole = 0;
uint8_t curDiastole = 0;
uint8_t curHeartRate = 0;
uint8_t curSPO2 = 0;
uint8_t type = 0;

bool isScanning = false;
unsigned long scanStartTime = 0;
void connectToDevice();
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.println(advertisedDevice.getName() + " " + advertisedDevice.getAddress().toString());
    if (advertisedDevice.getAddress().toString() == DEVICE_ADDRESS) {
      deviceFound = true;
      isScanning = false;
      BLEDevice::getScan()->stop();
      String DevAddress = advertisedDevice.getAddress().toString();
      Serial.println("Found target BLE device.");
      Serial.println(DevAddress);
      //connectToDevice();
    }
  }
};

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pClient) {
    Serial.println("Connected to server.");
    isConnected = true;
    isScanning = false;
  }

  void onDisconnect(BLEClient* pClient) {
    Serial.println("Disconnected from server.");
    isConnected = false;
    doReconnect = true;
    deviceFound = false;
    if (pClient != NULL) {
      pClient->disconnect();
      delete pClient;
    }
    delay(1000);  // Delay before reconnect attempt
    /*
pClient->disconnect();
    delete pClient;
    doReconnect = true;
    isConnected = false;
    deviceFound = false;
        */
  }
};

void notifyCallback(BLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  if (length > 0) {
    MICounter = 0;  //reset ?
    uint8_t code1 = pData[3];
    uint8_t code2 = pData[4];
    uint8_t value = pData[5];

    if (code1 == 0x08) {
      uint8_t sistole = pData[6];
      uint8_t diastole = pData[7];
      if (sistole == 0 || sistole == 255) {
        inbody = false;
        type = 0;
      } else {
        inbody = true;
        Serial.printf("Blood pressure: %d/%d\n", sistole, diastole);
        curSistole = sistole;
        curDiastole = diastole;
      }
    } else if (code1 == 0x06) {
      if (code2 == 0x6b) {
        Serial.printf("SpO2: %d %%\n", value);
        curSPO2 = value;
      } else if (code2 == 0x6d) {
        Serial.printf("Heart Rate: %d bpm\n", value);
        curHeartRate = value;
      }
    }
  }
}

void connectToDevice() {
  Serial.println("Connecting to BLE device...");
  BLEAddress address(DEVICE_ADDRESS);
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  if (pClient->connect(address)) {
    Serial.println("Connected to BLE device.");

    BLERemoteService* pService = pClient->getService(BLEUUID(BATTERY_SERVICE_UUID));
    if (pService != nullptr) {
      pBatteryLevelCharacteristic = pService->getCharacteristic(BATTERY_LEVEL_UUID);
      if (pBatteryLevelCharacteristic != nullptr) {
        Serial.println("Battery Level Characteristic found.");
      }
      readBatteryLevel();
    }

    BLERemoteService* pService1 = pClient->getService(BLEUUID(MEASUREMENT_SERVICE_UUID));
    if (pService1 != nullptr) {
      pHeartRateCharacteristic = pService1->getCharacteristic(MEASUREMENT_NOTIFICATION_UUID);
      if (pHeartRateCharacteristic != nullptr) {
        Serial.println("Measurement Characteristic found.");
        if (pHeartRateCharacteristic->canNotify()) {
          pHeartRateCharacteristic->registerForNotify(notifyCallback);
        }
      } else {
        Serial.println("Failed to find Measurement characteristic.");
      }
    } else {
      Serial.println("Failed to find Measurement service.");
    }

    // Discover the proprietary service
    BLERemoteService* pService0 = pClient->getService(BLEUUID(REQUEST_SERVICE_UUID));
    if (pService != nullptr) {
      Serial.println("Request Service found.");

      // Discover the characteristic within the service
      pCharacteristic = pService0->getCharacteristic(BLEUUID(REQUEST_CHARACTERISTIC_UUID));
      if (pCharacteristic != nullptr) {
        Serial.println("Characteristic found.");
      } else {
        Serial.println("Failed to find characteristic.");
      }
    } else {
      Serial.println("Failed to find service.");
    }

    BLERemoteService* pService3 = pClient->getService(BLEUUID("180D"));  // Device Notify Service UUID
    if (pService3 != nullptr) {
      pHeartRateNotify = pService3->getCharacteristic(HEART_RATE_MEASUREMENT_UUID);
      if (pHeartRateNotify != nullptr) {
        isConnected = true;
        Serial.println("Heart Rate Characteristic found.");
        if (pHeartRateCharacteristic->canNotify()) {
          pHeartRateNotify->registerForNotify(pHeartRateNotify_notifyCallback);  // Register for notifications
        }
      } else {
        Serial.println("Heart Rate Characteristic not found.");
      }
      //pClient->disconnect();
    } else {
      Serial.println("Heart Rate service not found.");
    }

    isConnected = true;

  } else {
    Serial.println("Failed to connect.");
    pClient->disconnect();
    delete pClient;
    doReconnect = true;
    isConnected = false;
    deviceFound = false;
  }
}

void pHeartRateNotify_notifyCallback(BLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  if (length > 0) {
    curHeartRate = pData[1];  // Heart rate is usually the second byte
    Serial.printf("Heart Rate: %d bpm\n", curHeartRate);
  }
}

void scanBLEDev() {
  if (!isScanning) {
    Serial.println("Starting BLE scan...");
    BLEScan* pScan = BLEDevice::getScan();
    pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pScan->setActiveScan(true);
    pScan->start(SCAN_TIME, false);
    // Set the scan start time
    scanStartTime = millis();
    isScanning = true;
  }
}
/*
Opcode: Write Command (0x52)
        0... .... = Authentication Signature: False, meaning no authentication signature is included.
        .1.. .... = Command: True, indicating that this is a command message, not a request-response.
        ..01 0010 = Method: Write Request (0x12) using the ATT protocol.
Handle: 0x0020 (Swirl Networks, Inc.: Anki, Inc.)
        [Service UUID: Swirl Networks, Inc. (0xfeea)] identifies the custom service in which this attribute resides.
        [UUID: Anki, Inc. (0xfee2)]  which refers to the specific characteristic being written to.
    Value: feea20066b00 is the actual data being written to the characteristic. 
*/
void writeValueToCharacteristic(uint8_t type) {
  if (isConnected && pCharacteristic != nullptr) {
    uint8_t value1[] = { 0xfe, 0xea, 0x20, 0x06, 0x6b, 0x00 };  // Hex representation of feea20066b00 SPO2
    uint8_t value2[] = { 0xfe, 0xea, 0x20, 0x08, 0x69, 0x00 };  // Hex representation of fe ea 20 08 69 00 00 00 Blood Pressure
    uint8_t value3[] = { 0xfe, 0xea, 0x20, 0x06, 0x6d, 0x00 };  // Hex representation of feea20066d00 Heart Rate

    if (pCharacteristic->canWrite()) {
      //Serial.println("Characteristic can write.");
    } else if (pCharacteristic->canWriteNoResponse()) {
      //Serial.println("Characteristic can write no response.");
    } else {
      Serial.println("Characteristic unwritable.");
      return;
    }
    if (type == 0) {
      pCharacteristic->writeValue(value1, sizeof(value1), false);  // Write without response
    } else if (type == 1) {
      pCharacteristic->writeValue(value2, sizeof(value2), false);  // Write without response
      //curSistole = 0;                                                                  //reset value
    }
    //else if (type == 2) pCharacteristic->writeValue(value3, sizeof(value3), false);  // Write without response, heart rate service avail, get notift form hr service, nodot need request
    else
      return;

  } else {
    Serial.println("Not connected to the characteristic.");
  }
}

void setup() {
  Serial.begin(115200);
  BLEDevice::init("");
  scanBLEDev();
  if (deviceFound) {
    connectToDevice();
    doReconnect = false;
  }
}

unsigned long previousOLMillis = 0;

void loop() {
  unsigned long currentOLMillis = millis();

  // Check if 1 second has passed
  if (currentOLMillis - previousOLMillis >= 1000) {
    previousOLMillis = currentOLMillis;
    Serial.println("BP:" + String(curSistole) + "/" + String(curDiastole) + " HR:" + String(curHeartRate) + " O2:" + String(curSPO2));
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 30000) {  //first measurement, 15 s time to response
    if (isConnected) {
      //no measuring yet
      if (curSPO2 == 0) {
        previousMillis = currentMillis;
        Serial.println("First SPO value");
        writeValueToCharacteristic(0);
      } else if (curSistole == 0) {
        previousMillis = currentMillis;
        Serial.println("First BP value");
        writeValueToCharacteristic(1);
      } else if (curHeartRate == 0) {
        previousMillis = currentMillis;
        Serial.println("First HR value");
        writeValueToCharacteristic(2);
      }
    }
  }

  if (currentMillis - previousMillis >= READ_INTERVAL) {
    previousMillis = currentMillis;
    MICounter++;
    Serial.println("Loop");
    if (isConnected) {
      readBatteryLevel();
      if (curBatteryLevel > 5) {



        if (MICounter >= MEASUREMENT_INTERVAL) {
          MICounter = 0;  //reset
          if (type > 2) type = 0;
          Serial.println("Read" + String(type));
          writeValueToCharacteristic(type);
          type++;
        }


      } else {
        Serial.println("Low batt, no measurement request.");
      }
    } else {
      esp_restart();
    }
  }

  // Handle scan timeout
  if (isScanning && (millis() - scanStartTime >= SCAN_TIMEOUT) && !isConnected && !deviceFound) {
    Serial.println("Scan timed out.");
    BLEDevice::getScan()->stop();
    isScanning = false;

    // Retry scan or perform other logic
    delay(5000);   // Wait before retrying (optional)
    scanBLEDev();  // Retry scan after timeout
  }
  /*
      isConnected = false;
    doReconnect = true;
    deviceFound = false;
    */
  if (doReconnect && !isConnected) {
    static unsigned long lastReconnectAttempt = 0;
    if (currentMillis - lastReconnectAttempt >= 5000) {
      lastReconnectAttempt = currentMillis;
      Serial.println("Reconnec");
      scanBLEDev();
      if (deviceFound) {
        connectToDevice();
        doReconnect = false;
      }
    }
  }
}

void readBatteryLevel() {
  if (isConnected && pBatteryLevelCharacteristic != nullptr) {
    if (pBatteryLevelCharacteristic->canRead()) {
      uint8_t batteryLevel = pBatteryLevelCharacteristic->readUInt8();
      if (batteryLevel <= 100) {
        Serial.printf("Battery Level: %d%%\n", batteryLevel);
        curBatteryLevel = batteryLevel;
      } else {
        Serial.printf("Battery is being charged\n");
      }
    } else {
      Serial.println("Battery Level Characteristic is not readable.");
    }
  }
}
