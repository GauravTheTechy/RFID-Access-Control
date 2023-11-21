#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>

constexpr uint8_t RST_PIN = 9;
constexpr uint8_t SS_PIN = 10;
constexpr uint8_t RELAY_CONTROL_PIN = 6; // Pin to control the relay module

MFRC522 mfrc522(SS_PIN, RST_PIN);
RTC_DS3231 rtc;

struct RFIDTag {
  String uid;
  String name;
};

RFIDTag masterTags[3] = {
  {"854ED383", "Tag_1"},
  {"19D968D3", "Tag_2"},
  {"57EF3835", "Tag_3"}
};

String tagID = "";

void setup() {
  Serial.begin(9600);
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(RELAY_CONTROL_PIN, OUTPUT);
  digitalWrite(RELAY_CONTROL_PIN, HIGH); // Ensure the relay is initially in the off state
}

void loop() {
  if (getID()) {
    bool granted = false;
    String name = "";

    for (int i = 0; i < 3; i++) {
      if (tagID == masterTags[i].uid) {
        granted = true;
        name = masterTags[i].name;
        break;
      }
    }

    DateTime now = rtc.now();
    char dateTime[20];
    sprintf(dateTime, "%04d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

    Serial.print("Date & Time: ");
    Serial.println(dateTime);
    Serial.print("UID: ");
    Serial.println(tagID);
    Serial.print("Name: ");
    Serial.println(name);
    Serial.println(granted ? "Access Granted!" : "Access Denied!");

    if (granted) {
      openDoor();
    }

    // Reset RFID module for the next scan
    mfrc522.PCD_Init();

    // Add a delay between scans
    delay(1000); // Adjust as needed
  }
}

boolean getID() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }
  tagID = "";
  for (uint8_t i = 0; i < 4; i++) {
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA();
  return true;
}

void openDoor() {
  Serial.println("Access Granted! Activating the relay...");

  digitalWrite(RELAY_CONTROL_PIN, LOW); // Turn on the relay

  delay(5000); // Keep the relay on for 5 seconds (adjust as needed)

  Serial.println("Deactivating the relay...");
  digitalWrite(RELAY_CONTROL_PIN, HIGH); // Turn off the relay

  delay(2000); // Additional delay after deactivating the relay
}
