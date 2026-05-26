/*
  ============================================================
  SMART IOT SYSTEM - ARDUINO UNO
  Modules:
  - Smart Campus: RFID, PIR, MQ-2, buzzer, LEDs, door servo
  - Smart Home: DHT11, relay light, optional fan
  - Smart Parking: HC-SR04, barrier servo, LCD, LEDs

  Arduino role:
  - Reads sensors
  - Controls actuators
  - Sends data to ESP32 through Serial
  - Receives commands from ESP32 through Serial

  Author: Smart IoT Project
  ============================================================
*/

#include <Servo.h>
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

// ===================== DHT11 =====================
#define DHT_PIN 2
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// ===================== CAMPUS SECURITY =====================
#define PIR_PIN 3
#define MQ2_PIN A0
#define BUZZER_PIN 6

#define RFID_SS_PIN 10
#define RFID_RST_PIN A4
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

// Change this UID with your own card UID from Serial Monitor
String allowedUID = "DE AD BE EF";

// ===================== SERVOS =====================
#define DOOR_SERVO_PIN 7
#define PARKING_SERVO_PIN 8

Servo doorServo;
Servo parkingServo;

// ===================== SMART HOME =====================
#define RELAY_LIGHT_PIN 9

// ===================== SMART PARKING =====================
#define TRIG_PIN 4
#define ECHO_PIN 5

int parkingPlaces = 3;
bool carAlreadyDetected = false;

// ===================== LEDS =====================
#define LED_GREEN_PIN A1
#define LED_RED_PIN A2
#define LED_YELLOW_PIN A3

// ===================== LCD 16x2 =====================
// Recommended: use a normal Arduino Uno-compatible pin mapping.
// LCD RS, E, D4, D5, D6, D7
LiquidCrystal lcd(A5, 11, 12, 13, A0, A1);

// NOTE:
// If you use the LCD pins above, avoid using MQ2 on A0 and LED_GREEN on A1.
// For a real build, the best solution is an I2C LCD module.
// This code is written as a complete prototype. Adjust pins depending on your wiring.

// ===================== CONFIG =====================
const int GAS_THRESHOLD = 400;
const int CAR_DISTANCE_THRESHOLD_CM = 10;

unsigned long lastDataSend = 0;
const unsigned long DATA_INTERVAL = 1500;

void setup() {
  Serial.begin(9600);

  dht.begin();

  SPI.begin();
  rfid.PCD_Init();

  doorServo.attach(DOOR_SERVO_PIN);
  parkingServo.attach(PARKING_SERVO_PIN);

  doorServo.write(0);
  parkingServo.write(0);

  pinMode(PIR_PIN, INPUT);
  pinMode(MQ2_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(RELAY_LIGHT_PIN, OUTPUT);
  digitalWrite(RELAY_LIGHT_PIN, LOW);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Smart IoT");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");

  delay(2000);
}

void loop() {
  checkRFIDAccess();
  handleSafetySensors();
  handleParking();
  handleCommandsFromESP32();

  if (millis() - lastDataSend >= DATA_INTERVAL) {
    sendSystemDataToESP32();
    updateLCD();
    lastDataSend = millis();
  }
}

// ============================================================
// SENSOR FUNCTIONS
// ============================================================

float readTemperature() {
  float value = dht.readTemperature();

  if (isnan(value)) {
    return -1;
  }

  return value;
}

float readHumidity() {
  float value = dht.readHumidity();

  if (isnan(value)) {
    return -1;
  }

  return value;
}

int readGasValue() {
  return analogRead(MQ2_PIN);
}

int readMotionValue() {
  return digitalRead(PIR_PIN);
}

int readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return 999;
  }

  int distance = duration * 0.034 / 2;
  return distance;
}

// ============================================================
// CAMPUS MODULE
// ============================================================

void checkRFIDAccess() {
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  String uid = getCardUID();

  if (uid == allowedUID) {
    grantAccess();
  } else {
    denyAccess();
  }

  rfid.PICC_HaltA();
}

String getCardUID() {
  String uid = "";

  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      uid += "0";
    }

    uid += String(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) {
      uid += " ";
    }
  }

  uid.toUpperCase();
  return uid;
}

void grantAccess() {
  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_RED_PIN, LOW);

  Serial.println("EVENT:ACCESS_GRANTED");

  openDoor();

  digitalWrite(LED_GREEN_PIN, LOW);
}

void denyAccess() {
  digitalWrite(LED_RED_PIN, HIGH);
  digitalWrite(LED_GREEN_PIN, LOW);

  Serial.println("EVENT:ACCESS_DENIED");

  tone(BUZZER_PIN, 1000);
  delay(1000);
  noTone(BUZZER_PIN);

  digitalWrite(LED_RED_PIN, LOW);
}

void openDoor() {
  doorServo.write(90);
  delay(3000);
  doorServo.write(0);
}

// ============================================================
// SAFETY MODULE
// ============================================================

void handleSafetySensors() {
  int gasValue = readGasValue();
  int motionValue = readMotionValue();

  if (gasValue > GAS_THRESHOLD) {
    activateGasAlarm();
  } else {
    deactivateGasAlarm();
  }

  if (motionValue == HIGH) {
    digitalWrite(LED_YELLOW_PIN, HIGH);
  } else {
    digitalWrite(LED_YELLOW_PIN, LOW);
  }
}

void activateGasAlarm() {
  digitalWrite(LED_RED_PIN, HIGH);
  tone(BUZZER_PIN, 1500);
}

void deactivateGasAlarm() {
  noTone(BUZZER_PIN);
  digitalWrite(LED_RED_PIN, LOW);
}

// ============================================================
// SMART PARKING MODULE
// ============================================================

void handleParking() {
  int distance = readDistanceCM();

  if (distance < CAR_DISTANCE_THRESHOLD_CM && !carAlreadyDetected) {
    carAlreadyDetected = true;

    if (parkingPlaces > 0) {
      openParkingBarrier();
      parkingPlaces--;
    }
  }

  if (distance > 20) {
    carAlreadyDetected = false;
  }

  if (parkingPlaces <= 0) {
    digitalWrite(LED_RED_PIN, HIGH);
    digitalWrite(LED_GREEN_PIN, LOW);
  } else {
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(LED_RED_PIN, LOW);
  }
}

void openParkingBarrier() {
  parkingServo.write(90);
  delay(3000);
  parkingServo.write(0);

  Serial.println("EVENT:BARRIER_OPENED");
}

// ============================================================
// ESP32 COMMUNICATION
// ============================================================

void sendSystemDataToESP32() {
  float temperature = readTemperature();
  float humidity = readHumidity();
  int gas = readGasValue();
  int motion = readMotionValue();
  int distance = readDistanceCM();

  Serial.print("TEMP:");
  Serial.print(temperature);
  Serial.print(",HUM:");
  Serial.print(humidity);
  Serial.print(",GAS:");
  Serial.print(gas);
  Serial.print(",MOTION:");
  Serial.print(motion);
  Serial.print(",PARK:");
  Serial.print(parkingPlaces);
  Serial.print(",DIST:");
  Serial.println(distance);
}

void handleCommandsFromESP32() {
  if (!Serial.available()) {
    return;
  }

  String command = Serial.readStringUntil('\n');
  command.trim();

  if (command == "LIGHT_ON") {
    digitalWrite(RELAY_LIGHT_PIN, HIGH);
    Serial.println("EVENT:LIGHT_ON");
  }

  if (command == "LIGHT_OFF") {
    digitalWrite(RELAY_LIGHT_PIN, LOW);
    Serial.println("EVENT:LIGHT_OFF");
  }

  if (command == "OPEN_DOOR") {
    openDoor();
    Serial.println("EVENT:DOOR_OPENED");
  }

  if (command == "OPEN_BARRIER") {
    openParkingBarrier();
  }

  if (command == "RESET_PARKING") {
    parkingPlaces = 3;
    Serial.println("EVENT:PARKING_RESET");
  }
}

// ============================================================
// LCD
// ============================================================

void updateLCD() {
  float temperature = readTemperature();
  int gas = readGasValue();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature);
  lcd.print("C ");

  lcd.print("G:");
  lcd.print(gas);

  lcd.setCursor(0, 1);
  lcd.print("Places:");
  lcd.print(parkingPlaces);
}
