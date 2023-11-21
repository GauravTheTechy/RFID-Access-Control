#include <MFRC522.h>
#include <SPI.h>
#include <Servo.h>
#include <TimeLib.h>

constexpr uint8_t RST_PIN = 9;
constexpr uint8_t SS_PIN = 10;

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo Servo1;

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
  Servo1.attach(6);
  Servo1.write(0);
  Serial.begin(9600);
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  // Set the time manually to October 30, 2023, 00:00:00 (UTC+5:30)
  setTime(10, 02, 00, 04, 11, 2023);
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

    time_t currentTime = now();
    char dateTime[20];
    sprintf(dateTime, "%04d-%02d-%02d %02d:%02d:%02d", year(currentTime), month(currentTime), day(currentTime), hour(currentTime), minute(currentTime), second(currentTime));

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
  Serial.println("Access Granted! Opening the door...");
  for (int i = 0; i <= 100; i++) {
    Servo1.write(i);
    delay(20);
  }
  delay(5000);
  Servo1.write(0);
  delay(5000);
}
