#include <Keypad.h>
#include <SD.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// SoftwareSerial object for communication with the fingerprint sensor
SoftwareSerial mySerial(10, 12);  // RX, TX
// Initialize the fingerprint sensor
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);
File myFile;
const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {30, 31, 32, 33};
byte colPins[COLS] = {34, 35, 36, 37};
char key = ';';

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

int newID = 0;
String newPwd = "NaN";
String newCard = "NaN";

void setup()
{
  // Initialize Serial for debugging
  Serial.begin(9600);
  Serial3.begin(9600);
  // Initialize the fingerprint sensor
  finger.begin(57600);
  delay(5);
  // Check if the fingerprint sensor is connected
  if (finger.verifyPassword())
  {
    Serial.println("Fingerprint sensor found!");
  }
  else
  {
    Serial.println("Fingerprint sensor not found. Check wiring!");
    while (1);
  }
  while (!Serial)
    delay(100);
  if (!SD.begin(53)) {
    Serial.println("SD initialization failed!");
    return;
  }
  Serial.println("SD initialization done.");
}

int readNewID() {
  int num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void printMessage(String message, boolean backline = true) {
  if (backline)
    Serial.println(message);
  else
    Serial.print(message);
}

bool storeInDB() {
  myFile = SD.open("database.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    printMessage("Writing to test.txt...", false);
    myFile.print(newID);
    myFile.print(",");
    myFile.print(newPwd);
    myFile.print(",");
    myFile.println(newCard);
    // close the file:
    myFile.close();
    printMessage("done.");
    newID = 0;
    newPwd = "";
  } else {
    // if the file didn't open, print an error:
    printMessage("error opening test.txt");
  }
}

int newUserID() {
  myFile = SD.open("database.txt", FILE_READ);
  // if the file opened okay, write to it:
  int id = 0;
  if (!myFile)
    return id;
  else {
    String car;
    while (myFile.available()) {
      car = myFile.readStringUntil('\n');
      int i1 = car.indexOf(',');
      id = car.substring(0, i1).toInt();
    }
    myFile.close();
    return id + 1;
  }
}

uint8_t checkCardexist(String id) {
  myFile = SD.open("database.txt", FILE_READ);
  // if the file opened okay, write to it:
  bool founded = false;
  if (!myFile)
    return 0;
  else {
    String car;
    while (myFile.available()) {
      car = myFile.readStringUntil('\n');
      int i1 = car.indexOf(',');
      int i2 = car.indexOf(',', i1 + 1);
      if (car.substring(i2 + 1, car.length() - 1).equalsIgnoreCase(id)) {
        founded = true;
        break;
      }
    }
    myFile.close();
    return founded;
  }
}

uint8_t checkPwdExist(String pwd) {
  myFile = SD.open("database.txt", FILE_READ);
  // if the file opened okay, write to it:
  bool founded = false;
  if (!myFile)
    return 0;
  else {
    String car;
    while (myFile.available()) {
      car = myFile.readStringUntil('\n');
      int i1 = car.indexOf(',');
      int i2 = car.indexOf(',', i1 + 1);
      if (car.substring(i1 + 1, i2).equalsIgnoreCase(pwd)) {
        founded = true;
        break;
      }
    }
    myFile.close();
    return founded;
  }
}

bool checkIDexist(int ID) {
  myFile = SD.open("database.txt", FILE_READ);
  // if the file opened okay, write to it:
  bool founded = false;
  if (!myFile)
    return 0;
  else {
    String car;
    while (myFile.available()) {
      car = myFile.readStringUntil('\n');
      int i1 = car.indexOf(',');
      if (car.substring(0, i1).toInt() == ID)
        founded = true;
    }
    myFile.close();
  }
  return founded;
}

void storeFinger() {
  if (getFingerprintIDez() > 0) {
    printMessage("User already exist");
    return;
  }
  printMessage("Ready to enroll a fingerprint!");
  if (newID == 0) {// ID #0 not allowed, try again!
    return;
  }
  printMessage("Enrolling ID #", false);
  printMessage(String(newID));
  while (!getFingerprintEnroll());
}

int getFingerprintIDez()
{
  uint8_t p = -1;
  printMessage("Pose your finger for vérification");
  printMessage("****************************");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        printMessage("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        //printMessage(".", false);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        printMessage("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        printMessage("Imaging error");
        break;
      default:
        printMessage("Unknown error");
        break;
    }
  }

  if (p != FINGERPRINT_OK)
  {
    return -1;
  }
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
  {
    return -1;
  }
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)
  {
    return -1;
  }
  //#if __Debug
  printMessage("Found ID #", false);
  printMessage(String(finger.fingerID), false);
  printMessage(" with confidence of ", false);
  printMessage(String(finger.confidence));
  //#endif

  return finger.fingerID;
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  printMessage("Waiting for valid finger to enroll as #", false);
  printMessage(String(newID));
  printMessage("****************************");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        printMessage("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        //printMessage(".", false);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        printMessage("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        printMessage("Imaging error");
        break;
      default:
        printMessage("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      printMessage("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      printMessage("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      printMessage("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      printMessage("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      printMessage("Could not find fingerprint features");
      return p;
    default:
      printMessage("Unknown error");
      return p;
  }

  printMessage("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  printMessage("ID ", false);
  printMessage(String(newID));
  p = -1;

  printMessage("Place same finger again");
  printMessage("****************************");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        printMessage("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        //printMessage(".", false);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        printMessage("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        printMessage("Imaging error");
        break;
      default:
        printMessage("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      printMessage("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      printMessage("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      printMessage("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      printMessage("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      printMessage("Could not find fingerprint features");
      return p;
    default:
      printMessage("Unknown error");
      return p;
  }

  // OK converted!
  printMessage("Creating model for #", false);
  printMessage(String(newID));
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    printMessage("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    printMessage("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    printMessage("Fingerprints did not match");
    return p;
  } else {
    printMessage("Unknown error");
    return p;
  }

  printMessage("ID ", false);
  printMessage(String(newID));
  p = finger.storeModel(newID);
  if (p == FINGERPRINT_OK) {
    printMessage("Stored!");
    storeInDB();
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    printMessage("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    printMessage("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    printMessage("Error writing to flash");
    return p;
  } else {
    printMessage("Unknown error");
    return p;
  }
  return true;
}

void storeCard() {
  if (newID == 0) {// ID #0 not allowed, try again!
    return;
  }
  printMessage("Ready to enroll a Card !");
  printMessage("Enrolling ID #", false);
  printMessage(String(newID));
  while (!getCardEnroll());
}

uint8_t getCardEnroll() {
  newCard = "";
  char carac;
  printMessage("Please Batch the card");
  printMessage("************************");
  while (newCard.length() < 8) {
    if (Serial3.available() > 0) {
      newCard = Serial3.readStringUntil('\n');
    }
  }
  printMessage("Card : ", false);
  printMessage(String(newCard.length()));
  printMessage(String(checkCardexist(newCard)));
  if (checkCardexist(newCard) == true)
  {
    printMessage("User already exist");
    return true;
  }
  printMessage("Hit # to save password or C to cancel");
  while (carac != '#' && carac != 'C') {
    carac = customKeypad.getKey();
    if (carac != NO_KEY && (carac != '#' && carac != 'C'))
      printMessage("Hit # to save password or C to cancel");
  }
  if (carac == '#') {
    storeInDB();
    return true;
  } else {
    cancel();
    return true;
  }
}

void storePassword() {
  if (newID == 0) {// ID #0 not allowed, try again!
    return;
  }
  printMessage("Ready to enroll a password !");
  printMessage("Enrolling ID #", false);
  printMessage(String(newID));
  while (!getPasswordEnroll());
}

uint8_t getPasswordEnroll() {
  newPwd = "";
  char carac;
  printMessage("Enter the password 4 caracs");
  while (newPwd.length() < 4) {
    carac = customKeypad.getKey();
    if (carac && carac != NO_KEY) {
      newPwd += String(carac);
      printMessage(String(carac), false);
    }
  }
  printMessage("");
  printMessage("Hit # to save password or C to cancel");
  while (carac != '#' && carac != 'C') {
    carac = customKeypad.getKey();
    if (carac != NO_KEY && (carac != '#' && carac != 'C'))
      printMessage("Hit # to save password or C to cancel");
  }
  if (carac == '#') {
    storeInDB();
    return true;
  } else {
    cancel();
    return true;
  }
}

void cancel() {
  newID = 0;
  newPwd = "NaN";
  String newCard = "NaN";
  printMessage("Process cancelled");
}

void registerNew() {
  printMessage("Adding new USER.....");
  while (newID == 0) {
    newID = newUserID();
  }
  printMessage("ID : ", false);
  printMessage(String(newID));
  printMessage("Authentification Methode : ");
  printMessage("1. Fingerprint");
  printMessage("2. Card");
  printMessage("3. Password");
  char opt = ',';
  while (opt != '1' && opt != '2' && opt != '3') {
    opt = customKeypad.getKey();
    if (opt != NO_KEY && opt != '1' && opt != '2' && opt != '3')
      printMessage("Please type 1, 2 or 3 !");
  }
  switch (opt) {
    case '1':
      storeFinger();
      break;
    case '2':
      storeCard();
      break;
    default:
      storePassword();
  }
}

void passAuth() {
  String pwd = "";
  char carac;
  printMessage("Enter your password");
  while (pwd.length() < 4) {
    carac = customKeypad.getKey();
    if (carac && carac != NO_KEY) {
      pwd += String(carac);
      printMessage("*", false);
    }
  }
  printMessage("");
  printMessage("Hit # for OK or C to cancel");
  while (carac != '#' && carac != 'C') {
    carac = customKeypad.getKey();
    if (carac != NO_KEY && (carac != '#' && carac != 'C'))
      printMessage("Hit # for OK or C to cancel");
  }
  if (carac == '#') {
    if (checkPwdExist(pwd))
      printMessage("You can open the door");
    else
      printMessage("You are not allowed");
    return;
  }
}

void checkFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK){
    printMessage("Please try again!");
    return;
  }

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)
    printMessage("Access Denied");
  else
    printMessage("Access Allowed");
}

void loop()
{
  key = customKeypad.getKey();

  checkFingerprintIDez();

  if (key && key != NO_KEY) {
    printMessage(String(key));
    switch (key) {
      case 'A':
        registerNew();
        break;
      case 'D':
        printMessage(String(checkIDexist(1)));
        break;
      case '*':
        passAuth();
        break;
      default:
        break;
    }
  }
  if (Serial3.available() > 0) {
    String str = Serial3.readStringUntil('\n');
    if (checkCardexist(str) == true)
      printMessage("Access Allowed");
    else
      printMessage("Access Denied");
  }
  delay(200);
}
