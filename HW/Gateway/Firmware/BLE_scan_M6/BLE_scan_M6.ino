/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 5;  //In seconds
BLEScan* pBLEScan;

//Address of the peripheral device. Address will be found during scanning...
static BLEAddress* pServerAddress;

//BLE Server name (the other ESP32 name running the server sketch)
#define bleServerName "M6"

/* UUID's of the service, characteristic that we want to read*/
// BLE Service
static BLEUUID bmeServiceUUID(BLEUUID((uint16_t)0x180f));


static boolean doConnect = false;
static boolean connected = false;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
     Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    if (advertisedDevice.getName().substr(0,sizeof(bleServerName)-1)==bleServerName) {                 //Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop();                              //Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());  //Address of advertiser is the one we need
      doConnect = true;                                                //Set indicator, stating that we are ready to connect
      Serial.println("Device found, stop scan then connecting!");
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
}

//Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) {
  BLEClient* pClient = BLEDevice::createClient();

  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");

  // // Obtain a reference to the service we are after in the remote BLE server.
   BLERemoteService* pRemoteService = pClient->getService(bmeServiceUUID);
  // if (pRemoteService == nullptr) {
  //   Serial.print("Failed to find our service UUID: ");
  //   //Serial.println(bmeServiceUUID.toString().c_str());
  //   return (false);
  // }

  // // Obtain a reference to the characteristics in the service of the remote BLE server.
  // temperatureCharacteristic = pRemoteService->getCharacteristic(temperatureCharacteristicUUID);
  // humidityCharacteristic = pRemoteService->getCharacteristic(humidityCharacteristicUUID);

  // if (temperatureCharacteristic == nullptr || humidityCharacteristic == nullptr) {
  //   Serial.print("Failed to find our characteristic UUID");
  //   return false;
  // }
  // Serial.println(" - Found our characteristics");

  // //Assign callback functions for the Characteristics
  // temperatureCharacteristic->registerForNotify(temperatureNotifyCallback);
  // humidityCharacteristic->registerForNotify(humidityNotifyCallback);
  return true;
}

void loop() {

  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      //Activate the Notify property of each Characteristic
      // temperatureCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      // humidityCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
  Serial.println("Scan done!");
  // pBLEScan->clearResults();  // delete results fromBLEScan buffer to release memory
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }
  // put your main code here, to run repeatedly:
  // BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  // Serial.print("Devices found: ");
  // Serial.println(foundDevices.getCount());
  // Serial.println("Scan done!");
  // pBLEScan->clearResults();  // delete results fromBLEScan buffer to release memory
   delay(2000);
}