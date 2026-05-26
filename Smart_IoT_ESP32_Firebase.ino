/*
  ============================================================
  SMART IOT SYSTEM - ESP32 + FIREBASE
  ESP32 role:
  - Connects to WiFi
  - Reads data from Arduino Uno through Serial2
  - Sends sensor data to Firebase Realtime Database
  - Reads commands from Firebase
  - Sends commands back to Arduino Uno

  Required library:
  - Firebase ESP Client by Mobizt

  Author: Smart IoT Project
  ============================================================
*/

#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// ===================== WIFI CONFIG =====================
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// ===================== FIREBASE CONFIG =====================
#define API_KEY "YOUR_FIREBASE_API_KEY"
#define DATABASE_URL "https://your-project-id-default-rtdb.firebaseio.com/"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Serial2 pins for ESP32
// Arduino TX -> ESP32 RX2 GPIO16
// Arduino RX -> ESP32 TX2 GPIO17
#define ESP32_RX_PIN 16
#define ESP32_TX_PIN 17

String lastCommand = "NONE";
unsigned long lastCommandCheck = 0;
const unsigned long COMMAND_INTERVAL = 1000;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, ESP32_RX_PIN, ESP32_TX_PIN);

  connectToWiFi();
  connectToFirebase();
}

void loop() {
  readDataFromArduino();

  if (millis() - lastCommandCheck >= COMMAND_INTERVAL) {
    readCommandFromFirebase();
    lastCommandCheck = millis();
  }
}

// ============================================================
// WIFI
// ============================================================

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// ============================================================
// FIREBASE
// ============================================================

void connectToFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase sign up successful");
  } else {
    Serial.print("Firebase sign up failed: ");
    Serial.println(config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

// ============================================================
// ARDUINO DATA
// ============================================================

void readDataFromArduino() {
  if (!Serial2.available()) {
    return;
  }

  String line = Serial2.readStringUntil('\n');
  line.trim();

  if (line.length() == 0) {
    return;
  }

  Serial.println("Arduino: " + line);

  if (line.startsWith("TEMP:")) {
    uploadSensorData(line);
  }

  if (line.startsWith("EVENT:")) {
    uploadEvent(line);
  }
}

void uploadSensorData(String data) {
  float temperature = getValue(data, "TEMP:", ",HUM:").toFloat();
  float humidity = getValue(data, "HUM:", ",GAS:").toFloat();
  int gas = getValue(data, "GAS:", ",MOTION:").toInt();
  int motion = getValue(data, "MOTION:", ",PARK:").toInt();
  int parkingPlaces = getValue(data, "PARK:", ",DIST:").toInt();
  int distance = getValue(data, "DIST:", "").toInt();

  Firebase.RTDB.setFloat(&fbdo, "/system/temperature", temperature);
  Firebase.RTDB.setFloat(&fbdo, "/system/humidity", humidity);
  Firebase.RTDB.setInt(&fbdo, "/system/gas", gas);
  Firebase.RTDB.setInt(&fbdo, "/system/motion", motion);
  Firebase.RTDB.setInt(&fbdo, "/system/parkingPlaces", parkingPlaces);
  Firebase.RTDB.setInt(&fbdo, "/system/distance", distance);
  Firebase.RTDB.setString(&fbdo, "/system/rawData", data);

  String safetyStatus = "NORMAL";

  if (gas > 400) {
    safetyStatus = "GAS_ALERT";
  }

  if (motion == 1) {
    Firebase.RTDB.setString(&fbdo, "/system/motionStatus", "MOTION_DETECTED");
  } else {
    Firebase.RTDB.setString(&fbdo, "/system/motionStatus", "NO_MOTION");
  }

  Firebase.RTDB.setString(&fbdo, "/system/safetyStatus", safetyStatus);
}

void uploadEvent(String eventLine) {
  String eventName = eventLine.substring(6);

  Firebase.RTDB.setString(&fbdo, "/system/lastEvent", eventName);
  Firebase.RTDB.setInt(&fbdo, "/system/lastEventTime", millis());

  Serial.println("Uploaded event: " + eventName);
}

// ============================================================
// FIREBASE COMMANDS
// ============================================================

void readCommandFromFirebase() {
  if (!Firebase.RTDB.getString(&fbdo, "/commands/action")) {
    return;
  }

  String command = fbdo.stringData();
  command.trim();

  if (command == "" || command == "NONE") {
    return;
  }

  if (command != lastCommand) {
    sendCommandToArduino(command);
    lastCommand = command;

    Firebase.RTDB.setString(&fbdo, "/commands/action", "NONE");
  }
}

void sendCommandToArduino(String command) {
  Serial2.println(command);
  Serial.println("Command sent to Arduino: " + command);
}

// ============================================================
// UTILS
// ============================================================

String getValue(String data, String startKey, String endKey) {
  int startIndex = data.indexOf(startKey);

  if (startIndex == -1) {
    return "";
  }

  startIndex += startKey.length();

  int endIndex;

  if (endKey == "") {
    endIndex = data.length();
  } else {
    endIndex = data.indexOf(endKey, startIndex);
  }

  if (endIndex == -1) {
    return "";
  }

  return data.substring(startIndex, endIndex);
}
