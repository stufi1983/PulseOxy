#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "B0401";
const char* password = "44444444";

const char* server = "stufimedia.com";  // Server URL
const char* url = "/posttest.php";  // Server URL

WiFiClientSecure client;

//Make sure you get the root's certificate (save file to .crt)
//https://randomnerdtutorials.com/esp32-https-requests

/// Root certificate (replace with the provided certificate)
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n"
"MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB\n"
"iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n"
"cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n"
"BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw\n"
"MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV\n"
"BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU\n"
"aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy\n"
"dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK\n"
"AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B\n"
"3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY\n"
"tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/\n"
"Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2\n"
"VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT\n"
"79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6\n"
"c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT\n"
"Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l\n"
"c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee\n"
"UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE\n"
"Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd\n"
"BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G\n"
"A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF\n"
"Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO\n"
"VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3\n"
"ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs\n"
"8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR\n"
"iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze\n"
"Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ\n"
"XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/\n"
"qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB\n"
"VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB\n"
"L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG\n"
"jjxDah2nGN59PRbxYvnKkKj9\n"
"-----END CERTIFICATE-----\n";

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>

#include <esp_task_wdt.h>


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
        else stat =       "Kurang  Sehat";
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

  Serial2.begin(1200, SERIAL_8N1, RXD2, TXD2);

  BLEDevice::init("");
  scanBLEDev();
  if (deviceFound) {
    connectToDevice();
    doReconnect = false;
  }
esp_pause_bluetooth();
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Attempt to connect to WiFi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000); // Wait 1 second before retrying
  }

  Serial.println("\nConnected to WiFi!");

  //client.setInsecure(); // For now, ignore SSL validation (insecure)
client.setCACert(root_ca);  // Set the root certificate for SSL

  Serial.println("Starting connection to server...");
  if (!client.connect(server, 443)) {
    Serial.println("Connection failed!");
    return;
  } else {
    Serial.println("Connected to server!");

    // Create JSON data to send in POST request
    DynamicJsonDocument doc(1024);

    // Add values to JSON document
    doc["key"] = "kunci";
    doc["sensor"] = "gps";
    doc["time"] = 1351824120;

    // Add array data
    JsonArray data = doc.createNestedArray("data");
    data.add(48.756080);
    data.add(2.302038);

    // Convert JSON document to string
    String postData;
    serializeJson(doc, postData);

    // Start a POST request
    client.println("POST "+String(url)+" HTTP/1.1");
    client.println("Host: " + String(server));
    //client.println("Content-Type: application/json"); //Doesn't work with $_POST, but work with: $data = json_decode(file_get_contents('php://input'), true);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: close");
    client.println("Content-Length: " + String(postData.length()));
    client.println();
    client.println(postData);

    // Wait for the response and print the headers
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("Headers received.");
        break;
      }
    }

    // Read and print the response
    String response;
    while (client.available()) {
      response = client.readString();
      Serial.println("Response: " + response);
    }

    // Close the connection
    client.stop();
  }
  esp_unpause_bluetooth();
  /*
  // Initialize the watchdog timer
  // Create the watchdog configuration
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 120000,  // 60 seconds timeout
    .idle_core_mask = 0,   // No idle core is masked
    .trigger_panic = true  // Trigger a panic and reset on timeout
  };

  // Initialize the watchdog timer with the configuration
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);  // Add current thread to the watchdog timer*/
}

unsigned long previousOLMillis = 0;

void loop() {
  unsigned long currentOLMillis = millis();

  // Check if 3 second has passed
  if (currentOLMillis - previousOLMillis >= 3000) {
    previousOLMillis = currentOLMillis;
    tickkMark = !tickkMark;
    Serial.println("BP:" + String(curSistole) + "/" + String(curDiastole) + " HR:" + String(curHeartRate) + " O2:" + String(curSPO2));
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
 // esp_task_wdt_reset();
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
