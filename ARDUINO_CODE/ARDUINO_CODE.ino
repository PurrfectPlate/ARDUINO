#include <SPI.h>
#include <MFRC522.h>
#include <CheapStepper.h>
#include <HX711_ADC.h>

#define SS_PIN 10  // RFID SDA Pin
#define RST_PIN 9  // RFID Reset Pin

#define STEPS 2048  // Change this value based on your stepper motor


const int motorPins[] = {4, 5, 6, 7}; //STEPPER MOTOR: IN1 = D4, IN2 = D5, IN3 = D6, IN4 = D7
const int weightPins[] = {2, 3}; //WEIGHT SENSOR: DT = 2, SCK = 3
//RFID Pins: RST = D9, SDA = D10, MOSI = D11, MISO = D12, SCK = D13

CheapStepper stepper = CheapStepper(motorPins[0], motorPins[1], motorPins[2], motorPins[3]);
HX711_ADC LoadCell(weightPins[0], weightPins[1]);
MFRC522 mfrc522(SS_PIN, RST_PIN);

unsigned long t = 0;

void setup() {
  Serial.begin(9600);  // Initialize serial communications
  SPI.begin();         // Initialize SPI bus
  mfrc522.PCD_Init();  // Initialize MFRC522 card reader
  float calibrationValue = 696.0;

  LoadCell.begin();
  unsigned long stabilizingtime = 2000;
  boolean _tare = true;
  LoadCell.start(stabilizingtime, _tare); if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration factor (float)
    Serial.println("Startup is complete");
  }
  //while (!LoadCell.update());

  Serial.println("Ready to read RFID cards and control stepper motor!");
}

bool getWeight = true;
bool getRFID = false;

void loop() {
  spinStepper();

  if (Serial.available() > 0) {
    String instruction = Serial.readString();
    Serial.println(instruction);
  }

  if (!getWeight && !getRFID) {
    return;
  }

  if (getRFID) {

    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      // Print UID to serial
      Serial.print("UID: ");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
      }
      Serial.println();

    }

  }

  if (getWeight) {
    static boolean newDataReady = 0;
    const int serialPrintInterval = 500;

    if (LoadCell.update()) newDataReady = true;

    if (newDataReady) {
      
      if (millis() > t + serialPrintInterval) {
        float i = LoadCell.getData();
        Serial.print("Load_cell output val: ");
        Serial.println(i);
        newDataReady = 0;
        t = millis();
      }
      
    }
    
  }


}

void spinStepper() {
  // Rotate the stepper motor
  stepper.setRpm(10);  // Set the speed (adjust as needed)
  stepper.moveDegrees(true, 240);
  stepper.moveDegrees(true, 240);
  stepper.moveDegrees(true, 240);
  stepper.moveDegrees(true, 240);
  stepper.moveDegrees(true, 240);
  stepper.moveDegrees(true, 240);
}
