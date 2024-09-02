//#ESP32
//#include <WiFi.h>

//example: {"dev": "FF:AA",  "bat": 99,  "sis": 100,  "dia": 70,  "hrr": 80,  "spo": 96,  "kon": true,  "con": true}

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <EEPROM.h>

#define EEPROM_SIZE 64                   // Define EEPROM size, adjust based on actual data size
#define DEFAULT_MAC "00:00:00:00:00:00"  // Default value

WiFiManager wifiManager;
char deviceAddr[20] = DEFAULT_MAC;  // Default device name
WiFiManagerParameter custom_device_addr("device_addr", "Device Address", deviceAddr, 20);

char hostAddr[40] = "stufimedia.com";  // Default device name
WiFiManagerParameter custom_host_addr("host_addr", "Server Address", hostAddr, 40);

const char* ssid = "TP-Link_26F8";
const char* password = "admin_elcons";

const char* server = "stufimedia.com";  // Server URL
const char* url = "/vhsm/index.php";    // Server URL

WiFiClientSecure client;

String inputString = "";      // A String to hold incoming data
bool stringComplete = false;  // Whether the string is complete

//Make sure you get the root's certificate (save file to .crt)
//https://randomnerdtutorials.com/esp32-https-requests

/// Root certificate (replace with the provided certificate)
const char* root_ca =
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
/*
Go to the website whose SSL certificate you need.
Click on the padlock icon to the left of the URL in the address bar.
Click on "Connection is secure" (or similar text) to view the certificate information.
Click on "Certificate (Valid)" to open the certificate details.
In the window that opens, go to the "Details" tab.
Scroll down to "SHA-1 Fingerprint" or "SHA-256 Fingerprint". This is the fingerprint you can use in your code.
*/
//Make sure to insert a space between each pair of hexadecimal characters (e.g., AA BB CC DD...).
const char* fingerprint = "e7 93 c9 b0 2f d8 aa 13 e2 1c 31 22 8a cc b0 81 19 64 3b 74 9c 89 89 64 b1 74 6d 46 c3 d4 cb d2";

unsigned long lastCheck = 0;
unsigned long reconnectAttemptTime = 0;
const unsigned long checkInterval = 15000;       // 15 seconds check interval
const unsigned long reconnectTimeout = 20000;    // 20 seconds reconnect timeout
const unsigned long wifiManagerTimeout = 60000;  // 60 seconds WiFiManager timeout
#define MAX_TRYING_TO_RECONNECT 3
bool shouldReconnect = false;

const int resetPin = 14;                   // GPIO14 (D5 on some ESP8266 boards)
const unsigned long resetDuration = 4000;  // 4 seconds in milliseconds
unsigned long buttonPressedTime = 0;
bool buttonPressed = false;

int tryingToReconnect = 0;
void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  Serial.println();
  delay(100);

  inputString.reserve(200);  // Reserve memory for the input string

  // Serial.print("Attempting to connect to SSID: ");
  // Serial.println(ssid);
  // WiFi.begin(ssid, password);

  // // Attempt to connect to WiFi network:
  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.print(".");
  //   delay(1000);  // Wait 1 second before retrying
  // }

  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  // Read data from EEPROM at boot
  String deviceAddrStr = readFromEEPROM();

  // Check if stored value is valid, else set to default
  if (deviceAddrStr.length() == 0 || deviceAddrStr.indexOf(':') < 2) {
    Serial.println("No data found in EEPROM, setting default value.");
    strcpy(deviceAddr, custom_device_addr.getValue());  // Save the custom value to `deviceName`
    writeToEEPROM(DEFAULT_MAC);
    deviceAddrStr = String(deviceAddr);
  } else {
    Serial.println("Data found in EEPROM: " + deviceAddrStr);
  }


  // Start WiFi Manager
  //wifiManager.setTimeout(wifiManagerTimeout / 1000);  // Timeout for WiFi Manager in seconds
  wifiManager.setConfigPortalTimeout(wifiManagerTimeout / 1000);
  // Add custom parameter to WiFiManager
  wifiManager.addParameter(&custom_device_addr);
  wifiManager.addParameter(&custom_host_addr);
  //wifiManager.setSaveConfigCallback(saveCustomConfigCallback);
  wifiManager.setSaveParamsCallback(saveCustomConfigCallback);
  if (!wifiManager.autoConnect("VHSM_01")) {  // Create an access point for configuration
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.restart();  // Restart the device if it fails to connect
  }
  // Read and print the custom parameter

  Serial.println("Device Addr: ");
  Serial.println("{" + deviceAddrStr + "}");      //notofy to ESP32
  strcpy(hostAddr, custom_host_addr.getValue());  // Save the custom value to `deviceName`
  Serial.print("Host Addr: ");
  Serial.println(String(hostAddr) + String(url));

  Serial.println("Connected to WiFi!");
  tryingToReconnect = 0;
  //ESP32
  //client.setCACert(root_ca);  // Set the root certificate for SSL

  //ESP8266
  //client.setFingerprint(fingerprint);  // Validate certificate by fingerprint

  client.setInsecure();  // For now, ignore SSL validation (insecure)
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW); //turn led ON
}

void saveCustomConfigCallback() {
  // Get the custom value from the WiFiManager parameter
  String newCustomValue = custom_device_addr.getValue();

  Serial.println("{" + newCustomValue + "}");
  writeToEEPROM(newCustomValue);
}

void loop() {
  // Check if the reset pin is connected to ground
  if (digitalRead(resetPin) == LOW) {
    if (!buttonPressed) {            // When the button is pressed for the first time
      buttonPressedTime = millis();  // Record the time when the button is first pressed
      buttonPressed = true;
    }

    // Check if the button has been pressed for the required duration
    if (buttonPressed && (millis() - buttonPressedTime >= resetDuration)) {
      Serial.println("Button pressed for 4 seconds, resetting WiFi settings...");

      // Reset WiFi settings
      wifiManager.resetSettings();  // Clear saved WiFi credentials
      ESP.restart();                // Restart the device to trigger WiFiManager again
    }
  } else {
    buttonPressed = false;  // Reset the flag when the button is released
  }


  unsigned long currentMillis = millis();

  // Check WiFi status every 15 seconds
  if (currentMillis - lastCheck >= checkInterval) {
    lastCheck = currentMillis;

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected. Attempting to reconnect...");
        digitalWrite(2, HIGH); //turn led OFF
      shouldReconnect = true;
      reconnectAttemptTime = currentMillis;  // Mark the time of the first reconnect attempt
      tryingToReconnect++;
      if (tryingToReconnect > MAX_TRYING_TO_RECONNECT) {
        ESP.restart();
      }
    }
  }

  // If reconnection is needed, wait for 20 seconds and reconnect
  if (shouldReconnect && currentMillis - reconnectAttemptTime >= reconnectTimeout) {
    shouldReconnect = false;

    // Reset WiFiManager if reconnection fails after timeout
    if (!WiFi.reconnect()) {
      Serial.println("Reconnect failed. Starting WiFiManager...");
      //wifiManager.resetSettings();  // Optional: Reset saved WiFi credentials
      wifiManager.startConfigPortal("ESP_Access_Point");  // Start WiFi Manager portal

      // After 30 seconds of no user input, try reconnecting to the last known SSID
      if (!wifiManager.autoConnect("ESP_Access_Point")) {
        Serial.println("Failed to connect after WiFiManager. Reconnecting to previous SSID...");
        WiFi.reconnect();
      }
    } else {
      Serial.println("Reconnected to WiFi successfully.");
      tryingToReconnect = 0;
        digitalWrite(2, LOW); //turn led ON
    }
  }


  // Do nothing in loop
  if (stringComplete) {
    Serial.print("Received: ");
    //Serial.println(inputString);  // Print the received string

    Serial.println("Starting connection to server...");
    if (!client.connect(server, 443)) {
      Serial.println("Connection failed!");
      // Clear the string after processing
      inputString = "";
      delay(1000);
      stringComplete = false;
      return;
    } else {
      Serial.println("Connected to server!");
      /*
    // Create JSON data to send in POST request
    DynamicJsonDocument doc(1024);

    // Add values to JSON document
    doc["dev"] = "AA:AA";
    doc["bat"] = 99;
    doc["sht"] = true;

    // Add array data
    JsonArray data = doc.createNestedArray("data");
    data.add(48.756080);
    data.add(2.302038);

    // Convert JSON document to string
    String postData;
    serializeJson(doc, postData);
*/
      // Start a POST request
      client.println("POST " + String(url) + " HTTP/1.1");
      // client.println("Host: " + String(server));
      client.println("Host: " + String(hostAddr));
      //client.println("Content-Type: application/json"); //Doesn't work with $_POST, but work with: $data = json_decode(file_get_contents('php://input'), true);
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.println("Connection: close");
      client.println("Content-Length: " + String(inputString.length()));
      client.println();
      client.println(inputString);

      // Wait for the response and print the headers
      Serial.println("Header:");
      while (client.connected()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
        if (line == "\r") {
          Serial.println("Headers received.");
          break;
        }
      }

      // Read and print the response
      String response;
      while (client.available()) {
        response = client.readString();
        Serial.println("Response:");
        Serial.println(response);
      }

      // Close the connection
      client.stop();
    }

    // Clear the string after processing
    inputString = "";
    stringComplete = false;
  }
}

// This function is called automatically when new serial data arrives
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();  // Read a character from the serial input

    // Add the character to the input string
    inputString += inChar;

    // Check if the incoming character is a newline ('\n')
    if (inChar == '\n') {
      stringComplete = true;  // Mark the string as complete
    }
  }
}

// Function to write data to EEPROM
void writeToEEPROM(String data) {
  for (int i = 0; i < data.length(); i++) {
    EEPROM.write(i, data[i]);
  }
  EEPROM.write(data.length(),0xFF); //add esc charraacter  string for last
  EEPROM.commit();
  Serial.println("Data written to EEPROM: " + data);
}

// Function to read data from EEPROM
String readFromEEPROM() {
  String data = "";
  for (int i = 0; i < EEPROM_SIZE; i++) {
    char value = EEPROM.read(i);
    if (value == 0xFF) break;  // Stop if empty
    data += value;
  }
  return data;
}
