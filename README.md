# Smart IoT System Code Package

Acest pachet conține codul complet pentru proiectul:

**Smart Campus + Smart Home + Smart Parking**

## Structură

```text
Smart_IoT_Project_Code/
│
├── 01_Arduino_Uno/
│   └── Smart_IoT_Arduino.ino
│
├── 02_ESP32_Firebase/
│   └── Smart_IoT_ESP32_Firebase.ino
│
├── 03_Web_Dashboard/
│   ├── index.html
│   ├── style.css
│   └── script.js
│
└── 04_Documentation/
    └── README.md
```

## 1. Arduino Uno

Arduino Uno citește datele de la:

- DHT11
- MQ-2
- PIR
- HC-SR04
- RFID RC522

Și controlează:

- servomotoare
- buzzer
- LED-uri
- releu
- LCD

## 2. ESP32

ESP32 face conexiunea WiFi și comunică cu Firebase.

În fișierul `Smart_IoT_ESP32_Firebase.ino`, modifică:

```cpp
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define API_KEY "YOUR_FIREBASE_API_KEY"
#define DATABASE_URL "https://your-project-id-default-rtdb.firebaseio.com/"
```

## 3. Dashboard Web

În fișierul `script.js`, modifică datele Firebase:

```js
const firebaseConfig = {
  apiKey: "YOUR_FIREBASE_API_KEY",
  authDomain: "YOUR_PROJECT_ID.firebaseapp.com",
  databaseURL: "https://YOUR_PROJECT_ID-default-rtdb.firebaseio.com",
  projectId: "YOUR_PROJECT_ID",
  storageBucket: "YOUR_PROJECT_ID.appspot.com",
  messagingSenderId: "YOUR_SENDER_ID",
  appId: "YOUR_APP_ID"
};
```

## 4. Conectare Arduino Uno - ESP32

Recomandat:

```text
Arduino TX  -> ESP32 RX GPIO16
Arduino RX  -> ESP32 TX GPIO17
Arduino GND -> ESP32 GND
```

Important: ESP32 funcționează la 3.3V logic, iar Arduino Uno la 5V. Pentru TX Arduino către RX ESP32, folosește un divizor de tensiune sau level shifter.

## 5. Librării necesare Arduino IDE

Pentru Arduino Uno:

- Servo
- DHT sensor library
- MFRC522
- LiquidCrystal

Pentru ESP32:

- Firebase ESP Client by Mobizt
- WiFi

## 6. Observație importantă despre LCD

Dacă folosești LCD 16x2 normal, ai nevoie de mai mulți pini. Cel mai simplu este să folosești un modul LCD I2C.

Pentru o maquetă mai curată, recomand LCD 16x2 I2C.
