#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

#define SS_PIN 22
#define RST_PIN 23
MFRC522 mfrc522(SS_PIN, RST_PIN);
#define light_pin A8

#define TAG_1 0xD0, 0xDD, 0x39, 0x12
//senzor lumina
const int senzor_led = 3;
float resolutionADC = .0049;
float resolutionSensor = .01;

// RGB LED pins
const int LED_R = 15;
const int LED_G = 14;
const int LED_B = 16;

// PIR sensor pin
const int PIR_SENSOR = 2;
const int led_pir = 30;
int state = LOW;
int val = 0;
unsigned long motionStartTime = 0;
const unsigned long motionDuration = 3000;  // 3 sec

int right = 1023;  
int left = 721;   
int sel = 480; 


int touchCount = 0;
int menuOption = 1;

const int pin_RS_LCD = 8;
const int pin_EN_LCD = 9;
const int pin_d4_LCD = 4;
const int pin_d5_LCD = 5;
const int pin_d6_LCD = 6;
const int pin_d7_LCD = 7;
LiquidCrystal lcd(pin_RS_LCD, pin_EN_LCD, pin_d4_LCD, pin_d5_LCD, pin_d6_LCD, pin_d7_LCD);



void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(PIR_SENSOR, INPUT);
  pinMode(light_pin, INPUT);
  pinMode(senzor_led, OUTPUT);

  lcd.begin(16, 2);
  lcd.print("Select option");
  delay(2000);
  lcd.clear();

  Serial.println("RFID reading UID");
}

void loop() {
  checkButtons();

  static int previousOption = 0;

  if (menuOption != previousOption) {
    switch (previousOption) {
      case 1:
        break;
      case 2:
        break;
      case 3:
        break;
    }
    previousOption = menuOption;
  }

  switch (menuOption) {
    case 1:
      handleRGBWithRFID();
      break;
    case 2:
      handleLEDWithPIR();
      break;
    case 3:
      lightSensor();
      break;
  }
}

void printButtonValues() {
  int rightValue = analogRead(A0);
  int leftValue = analogRead(A0);
  int selectValue = analogRead(A0);

  Serial.print("Right Button Value: ");
  Serial.println(rightValue);
  Serial.print("Left Button Value: ");
  Serial.println(leftValue);

  Serial.print("Select Button Value: ");
  Serial.println(selectValue);
  delay(2000);
}


void handleRGBWithRFID() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      Serial.println("Tag UID:");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
      }

      if (compareTags(mfrc522.uid.uidByte)) {
        Serial.println(" - Correct tag!");

        changeLEDColor();

        displayColorOnLCD();

      } else {
        Serial.println(" - Wrong tag scanned!");
      }

      mfrc522.PICC_HaltA();
    }
  }
}

bool compareTags(byte scannedTag[]) {
  byte expectedTag1[] = { TAG_1 };
  return compareArrays(scannedTag, expectedTag1, sizeof(expectedTag1));
}

bool compareArrays(byte arr1[], byte arr2[], int length) {
  for (int i = 0; i < length; i++) {
    if (arr1[i] != arr2[i]) {
      return false;
    }
  }
  return true;
}

void changeLEDColor() {
  touchCount = (touchCount + 1) % 4;
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);

  if (touchCount == 1) {
    digitalWrite(LED_R, HIGH);  // Red
    Serial.println("LED Color: Red");
  } else if (touchCount == 2) {
    digitalWrite(LED_G, HIGH);  // Green
    Serial.println("LED Color: Green");
  } else if (touchCount == 3) {
    digitalWrite(LED_B, HIGH);  // Blue
    Serial.println("LED Color: Blue");
  } else {
    touchCount = 0;
  }

  Serial.print("LED Color: R=");
  Serial.print(analogRead(LED_R));
  Serial.print(", G=");
  Serial.print(analogRead(LED_G));
  Serial.print(", B=");
  Serial.println(analogRead(LED_B));
}

void displayColorOnLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RFID Tag OK");
  lcd.setCursor(0, 1);
  lcd.print("LED Color: ");

  if (touchCount == 1) {
    lcd.print("Red");
  } else if (touchCount == 2) {
    lcd.print("Green");
  } else if (touchCount == 3) {
    lcd.print("Blue");
  } else {
    lcd.print("Reset");
  }
  delay(3000);  // 3 secunde pt mesaj
  lcd.clear();
}

void handleLEDWithPIR() {
  val = digitalRead(PIR_SENSOR);

  if (val == HIGH) {
    digitalWrite(led_pir, HIGH);

    if (state == LOW) {
      Serial.println("Motion detected!");
      motionStartTime = millis();  // tine minte cand a detectat miscarea
      state = HIGH;
    }
  } else {
    digitalWrite(led_pir, LOW);

    if (state == HIGH && (millis() - motionStartTime >= motionDuration)) {
      Serial.println("Motion stopped!");
      state = LOW;
    }
  }
}

void checkButtons() {
  int btnValue = analogRead(A0);

// right
  if (btnValue < 60) {
    menuOption = (menuOption % 3) + 1; 
    displayMenuOption();
    delay(500);
  } else if (btnValue < 600) {
// left
    menuOption = (menuOption - 2 + 3) % 3 + 1;  
    displayMenuOption();
    delay(500);
  } else if (btnValue < 800) {
// select
    delay(500); 
    lcd.clear();
    lcd.print("Selected Option: ");
    lcd.print(menuOption);
    delay(2000);
    lcd.clear();
  }
}

void displayMenuOption() {
  lcd.clear();
  lcd.print("Select option");
  lcd.setCursor(0, 1);
  lcd.print("Option: ");
  lcd.print(menuOption);
}



void lightSensor() {

  int light = analogRead(light_pin);

  float lightVoltage = light * resolutionADC;
  int invertedBrightness = map(light, 5, 1000, 255, 0);
  int brightness = constrain(invertedBrightness, 0, 255);

  analogWrite(senzor_led, brightness);

  Serial.print("Light Level (raw): ");
  Serial.print(light);
  Serial.print(", Light Voltage: ");
  Serial.print(lightVoltage, 3);
  Serial.print(", Brightness: ");
  Serial.println(brightness);

  delay(500);
}
