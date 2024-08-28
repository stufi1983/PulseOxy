#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>

#include <esp_task_wdt.h>
#include <ArduinoJson.h>
#define RXD1 12
#define TXD1 13

#define RXD2 16
#define TXD2 17

#define READ_INTERVAL 60000  //more than 30000 (time for watck to do measurement)

#define DEVICE_ADDRESS "f5:bc:47:6b:0b:34"  //f5:bc:47:6b:0b:34 --> convert to string became lowercase

#define BATTERY_SERVICE_UUID "180F"
#define BATTERY_LEVEL_UUID "2A19"

#define MEASUREMENT_SERVICE_UUID "FEEA"
#define MEASUREMENT_NOTIFICATION_UUID "FEE3"

#define HEART_RATE_MEASUREMENT_UUID "2A37"

#define SCAN_TIME 5
#define SCAN_TIMEOUT 15000      // Maximum scan duration in milliseconds (15 seconds)
#define MEASUREMENT_INTERVAL 3  // multipled by READ_INTERVAL

#define REQUEST_SERVICE_UUID "FEEA"
#define REQUEST_CHARACTERISTIC_UUID "FEE2"

uint8_t P10Mode = 0;
uint8_t curP10Mode = 255;

BLEClient* pClient;
BLERemoteCharacteristic* pCharacteristic;
BLERemoteCharacteristic* pBatteryLevelCharacteristic;
BLERemoteCharacteristic* pHeartRateCharacteristic;
BLERemoteCharacteristic* pHeartRateNotify;

bool deviceFound = false;
bool doReconnect = false;
bool isConnected = false;
bool tickkMark = false;
bool isCharged = true;
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

uint8_t dsistole = 0;  //dont diaplay 0

bool sehat = true;

void displayP10() {
  sehat = true;
  if (curSistole != 0)
    dsistole = curSistole;


  String stat = "";

  if (P10Mode == 0) {
    if (curSPO2 == 0) {
      P10Mode++;
    } else {
      if (curP10Mode == P10Mode) return;
      curP10Mode = P10Mode;
      if (curSPO2 > 95) {
        stat = "Kadar Oksigen Normal";
        //sehat = true;
      } else if (curSPO2 > 0) {
        stat = "Kurang oksigen";
        sehat = false;
      }
      Serial2.println(stat + ":" + " O2 " + String(curSPO2) + "%");
      return;
    }
  }
  if (P10Mode == 1) {
    if (dsistole == 0) {
      P10Mode++;
    } else {
      if (curP10Mode == P10Mode) return;
      curP10Mode = P10Mode;
      if (dsistole >= 90 && dsistole <= 120 && curDiastole >= 60 && curDiastole <= 80) {
        stat = "Tekanan Darah Normal";
        //sehat = true;
      } else {
        if (curDiastole < 60) stat = "Tekanan Darah Rendah";
        if (dsistole > 120) stat = "Tekanan Darah Tinggi";
        sehat = false;
      }
      Serial2.println(stat + ":" + " " + String(dsistole) + "/" + String(curDiastole));
      return;
    }
  }
  if (P10Mode == 2) {
    if (curHeartRate == 0) {
      P10Mode++;
    } else {
      if (curP10Mode == P10Mode) return;
      curP10Mode = P10Mode;
      if (curHeartRate > 50 && curHeartRate < 100) {
        stat = "Detak Jantung Normal";
        //sehat = true;
      } else {
        stat = "Detak jantung berbahaya";
        sehat = false;
      }
      Serial2.println(stat + ":" + " " + String(curHeartRate) + " BPM");
      return;
    }
  }
  if (P10Mode == 3) {
    if (curP10Mode == P10Mode) return;
    curP10Mode = P10Mode;

    if (!isConnected) stat = "Menghubungkan...";
    else {
      if (curHeartRate > 0 && curSPO2 > 0) {
        if (sehat) stat = " Sehat Bugar ";
        else stat = "Kurang  Sehat";
      } else {
        stat = "Pengukuran...";
      }
    }
    if (curBatteryLevel > 99) curBatteryLevel = 99;
    Serial2.println(stat + ":" + "Batt " + String(curBatteryLevel) + "%");
    return;
  }
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RXD1, TXD1);
  Serial2.begin(1200, SERIAL_8N1, RXD2, TXD2);

  //Serial1.println("Thisis ates");

  BLEDevice::init("");
  scanBLEDev();
  if (deviceFound) {
    connectToDevice();
    doReconnect = false;
  }

  // Initialize the watchdog timer
  // Create the watchdog configuration
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 120000,  // 60 seconds timeout
    .idle_core_mask = 0,   // No idle core is masked
    .trigger_panic = true  // Trigger a panic and reset on timeout
  };

  // Initialize the watchdog timer with the configuration
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);  // Add current thread to the watchdog timer
}


void sendJsonSerial() {
  const size_t capacity = JSON_OBJECT_SIZE(7);  // Optimize memory allocation for 7 key-value pairs

  // Allocate the JSON document with a defined capacity
  StaticJsonDocument<capacity> doc;


  // Add values in the document
  doc["dev"] = DEVICE_ADDRESS;
  doc["bat"] = curBatteryLevel;
  doc["sis"] = dsistole;
  doc["dia"] = curDiastole;
  doc["hrr"] = curHeartRate;
  doc["spo"] = curSPO2;
  doc["kon"] = sehat;
  doc["con"] = isConnected;

  // // Add an array
  // JsonArray data = doc["data"].to<JsonArray>();
  // data.add(48.756080);
  // data.add(2.302038);

  // Generate the minified JSON and send it to the Serial port
  serializeJson(doc, Serial1);
  Serial1.println();
}

unsigned long previousOLMillis = 0;
unsigned long previousJSONMillis = 0;

void loop() {
  byte RxData;
  // put your main code here, to run repeatedly:
  if (Serial1.available() > 0) {
    RxData = Serial1.read();
    Serial.write(RxData);
  }

  unsigned long currentJSONMillis = millis();
  if (currentJSONMillis - previousJSONMillis >= 60000) {
    previousJSONMillis = currentJSONMillis;
    if (curSPO2 > 0 && curHeartRate > 0 && curSistole > 0) {
      sendJsonSerial();
    }
  }

  unsigned long currentOLMillis = millis();

  // Check if 3 second has passed
  if (currentOLMillis - previousOLMillis >= 3000) {
    previousOLMillis = currentOLMillis;
    tickkMark = !tickkMark;
    //Serial.println("BP:" + String(curSistole) + "/" + String(curDiastole) + " HR:" + String(curHeartRate) + " O2:" + String(curSPO2));
    displayP10();
    P10Mode++;
    if (P10Mode > 4) P10Mode = 0;
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 30000) {  //first measurement, 15 s time to response
    if (isConnected && !isCharged) {
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
    if (isCharged) {
      previousMillis = currentMillis;
      Serial.println("Charging, no measurement!");
      readBatteryLevel();
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

  // Reset the watchdog timer
  esp_task_wdt_reset();
}

void readBatteryLevel() {
  if (isConnected && pBatteryLevelCharacteristic != nullptr) {
    if (pBatteryLevelCharacteristic->canRead()) {
      uint8_t batteryLevel = pBatteryLevelCharacteristic->readUInt8();
      if (batteryLevel <= 100) {
        Serial.printf("Battery Level: %d%%\n", batteryLevel);
        curBatteryLevel = batteryLevel;
        isCharged = false;
      } else {
        isCharged = true;
        Serial.printf("Battery is being charged\n");
      }
    } else {
      Serial.println("Battery Level Characteristic is not readable.");
    }
  }
}
