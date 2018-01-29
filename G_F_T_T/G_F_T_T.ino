/***************************************************
  This is an example sketch for our optical Fingerprint sensor

  Designed specifically to work with the Adafruit BMP085 Breakout
  ----> http://www.adafruit.com/products/751

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Adafruit_Fingerprint.h>
#include <ArduinoJson.h>
#include <Wire.h>
// On Leonardo/Micro or others with hardware serial, use those! #0 is green wire, #1 is white
// uncomment this line:
// #define mySerial Serial1

// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// comment these two lines if using hardware serial
#include <SoftwareSerial.h>
SoftwareSerial mySerial(6, 5);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;
uint8_t mode = 0;
unsigned long previousMillis = 0;
void setup()
{
  Wire.begin();
  Serial.begin(9600);
  Serial.setTimeout(100);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  //  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  // set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }
}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}
int se_s = 0;
int de_1 = 0;
int de_2 = 0;
int de_3 = 0;
int de_4 = 0;
void loop()                     // run over and over again
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 1000) {

    previousMillis = currentMillis;
    Wire.requestFrom(1, 1);
    while (Wire.available()) {
      de_1 = Wire.read();
    }
    Wire.requestFrom(1, 2);
    while (Wire.available()) {
      de_2 = Wire.read();
    }
    Wire.requestFrom(1, 3);
    while (Wire.available()) {
      de_3 = Wire.read();
    }
    Wire.requestFrom(1, 4);
    while (Wire.available()) {
      de_4 = Wire.read();
    }
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["de_1"] = de_1;
    root["de_2"] = de_2;
    root["de_3"] = de_3;
    root["de_4"] = de_1;
    root.printTo(Serial);
    Serial.println();
    Serial.print(de_1); Serial.print(" "); Serial.print(de_2); Serial.print(" "); Serial.print(de_3); Serial.print(" "); Serial.print(de_4); Serial.println();
  }
  if (Serial.available()) {

    String c = Serial.readString();
    StaticJsonBuffer<200> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject(c);

    // Test if parsing succeeds.
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }
    se_s = root["data"][0];
    id = root["data"][1];
    //{"data":[1,10]}
    // Print values.
    Serial.println(se_s);
    Serial.println(id);
    Serial.flush();
    jsonBuffer = StaticJsonBuffer<200>();
  }

  if (se_s == 0) {
    getFingerprintIDez();
    //    Serial.println(se_s);
    //    Serial.println(id);
  }
  if (se_s == 1) {
    Serial.print("Enrolling ID #");
    Serial.println(id);
    mode = 1;
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["mode"] = mode;
    root.printTo(Serial);
    Serial.println();
    //Serial.print(mode);
    while (!  getFingerprintEnroll() && se_s == 1 );
  }
}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        //Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        //Serial.println(". ");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        //Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        //Serial.println("Imaging error");
        break;
      default:
        //Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
      return p;
    default:
      //Serial.println("Unknown error");
      return p;
  }
  mode = 2;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["mode"] = mode;
  root.printTo(Serial);
  Serial.println();
  //Serial.println("Remove finger");
  //Serial.println(mode);
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  //Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        //Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        //Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        //Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        //Serial.println("Imaging error");
        break;
      default:
        //Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
      return p;
    default:
      //Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  //Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    //Serial.println("Fingerprints did not match");
    return p;
  } else {
    //Serial.println("Unknown error");
    return p;
  }

  //Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    //Serial.println("Stored! = ");
    mode = 3;
    se_s = 0;
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["mode"] = mode;
    root.printTo(Serial);
    Serial.println();
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    //Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    //Serial.println("Error writing to flash");
    return p;
  } else {
    //Serial.println("Unknown error");
    return p;
  }
}
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["ID"] = finger.fingerID;
  root["confidence"] = finger.confidence;
  root.printTo(Serial);
  Serial.println();

  //  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  //  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}
