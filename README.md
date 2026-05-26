# Smart IoT System Code Package

**Smart Campus**

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



```cpp
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define API_KEY "YOUR_FIREBASE_API_KEY"
#define DATABASE_URL "https://your-project-id-default-rtdb.firebaseio.com/"
```

## 3. Dashboard Web


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

```text
Arduino TX  -> ESP32 RX GPIO16
Arduino RX  -> ESP32 TX GPIO17
Arduino GND -> ESP32 GND
```

## 5. Librării necesare Arduino IDE

Pentru Arduino Uno:

- Servo
- DHT sensor library
- MFRC522
- LiquidCrystal

Pentru ESP32:

- Firebase ESP Client by Mobizt
- WiFi
