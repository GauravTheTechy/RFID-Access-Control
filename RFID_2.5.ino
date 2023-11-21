#include <MFRC522.h>
#include <SPI.h>
#include <Servo.h>
#include <TimeLib.h> // Include the TimeLib library for date and time

constexpr uint8_t RST_PIN = 9;
constexpr uint8_t SS_PIN = 10;

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo Servo1;

// Define the allowed RFID tags as an array of structures
struct RFIDTag {
  String uid;
  String name;
};

RFIDTag masterTags[3] = {
  {"854ED383", "Tag_1"}, // Replace with your first master tag and its name
  {"19D968D3", "Tag_2"}, // Replace with your second master tag and its name
  {"57EF3835", "Tag_3"}  // Replace with your third master tag and its name
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
}

void loop() {
  while (getID()) {
    bool granted = false;
    String name = "";

    for (int i = 0; i < 3; i++) {
      if (tagID == masterTags[i].uid) {
        granted = true;
        name = masterTags[i].name;
        break;
      }
    }

    // Get the current date and time
    time_t now = now();
    char dateTime[20];
    sprintf(dateTime, "%04d-%02d-%02d %02d:%02d:%02d", year(now), month(now), day(now), hour(now), minute(now), second(now));

    // Print the result
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
    delay(2000);
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