#include <Servo.h>
#include <EEPROM.h>
#include <HX711_ADC.h>              /* https://downloads.arduino.cc/libraries/github.com/olkal/HX711_ADC-1.2.12.zip */           
#include <LiquidCrystal_I2C.h>      /* https://downloads.arduino.cc/libraries/github.com/johnrickman/LiquidCrystal_I2C-1.1.2.zip */

#define HX711_DOUT 3
#define HX711_SCK 2
#define pinServo1 5
#define pinServo2 6
#define pinServo3 7
#define buttonStart 8
#define buttonStop 9
#define Relay 4

#define diTekan 1
#define ON true
#define OFF false

/* Jeda Servo2 dan Servo 3 Setelah Blok Benda */
#define delayServo2 3000
#define delayServo3 5000

/* Variabel Pembanding Berat Buah (gr) */
#define beratBuah_Besar 200
#define beratBuah_Kecil 180
#define buahKecil    (stableValue > 2 && stableValue <= 20)
#define buahSedangg  (stableValue > 20)
#define buahSedang   (stableValue > 20 && stableValue <= 50)
#define buahBesar    (stableValue > 50)

/* Perhalus Gerakan Servo dengan Mengecilkan Nilai */
#define stepServo23 10
#define stepServo1 10

HX711_ADC LoadCell(HX711_DOUT, HX711_SCK);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo Servo1,
      Servo2,
      Servo3;

const int addr1 = 1,
          addr2 = 2,
          addr3 = 3;

int dataSudut1 = 0,
    dataSudut2 = 0,
    dataSudut3 = 0;

const byte sudutDorong = 0,
           sudutKembali = 80;

const byte sudutBuka23 = 0,
           sudutTutup23 = 70;

float calibrationValue = -995.34;
float sensorValue;

boolean Konveyor = OFF;

int previousValue = 0;  // Simpan Nilai Sensor Sebelumnya
int stableValue = 0;    // Menyimpan Nilai Stabil
int tolerance = 2;      // Toleransi Selisih
int stableCounter = 0;  // Counter Deteksi Kestabilan
const int thresholdStableCount = 5; // Jumlah pembacaan stabil yang diperlukan

void Dorong() {
  for (dataSudut1; dataSudut1 >= sudutDorong; dataSudut1--) {
    Servo1.write(dataSudut1);
    delay(stepServo1);
    EEPROM.write(addr1, dataSudut1);
    readButton();
    readSensor();
  }
  delay(1000);
  for (dataSudut1; dataSudut1 <= sudutKembali; dataSudut1++) {
    Servo1.write(dataSudut1);
    delay(stepServo1);
    EEPROM.write(addr1, dataSudut1);
    readButton();
    readSensor();
  }
}

void Servo_Tutup2() {
  for (dataSudut2; dataSudut2 <= sudutTutup23; dataSudut2++) {
    Servo2.write(dataSudut2);
    delay(stepServo23);
    EEPROM.write(addr2, dataSudut2);
    readButton();
    readSensor();
  }
}

void Servo_Buka2() {
  for (dataSudut2; dataSudut2 >= sudutBuka23; dataSudut2--) {
    Servo2.write(dataSudut2);
    delay(stepServo23);
    EEPROM.write(addr2, dataSudut2);
    readButton();
    readSensor();
  }
}

void Servo_Tutup3() {
  for (dataSudut3; dataSudut3 <= sudutTutup23; dataSudut3++) {
    Servo3.write(dataSudut3);
    delay(stepServo23);
    EEPROM.write(addr3, dataSudut3);
    readButton();
    readSensor();
  }
}

void Servo_Buka3() {
  for (dataSudut3; dataSudut3 >= sudutBuka23; dataSudut3--) {
    Servo3.write(dataSudut3);
    delay(stepServo23);
    EEPROM.write(addr3, dataSudut3);
    readButton();
    readSensor();
  }
}

void cekKondisi_Awal() {
  dataSudut1 = EEPROM.read(addr1);
  dataSudut2 = EEPROM.read(addr2);
  dataSudut3 = EEPROM.read(addr3);

  //  Serial.println(dataSudut1);
  //  Serial.println(dataSudut2);
  //  Serial.println(dataSudut3);

  if (dataSudut1 >= sudutKembali) {
    for (dataSudut1; dataSudut1 >= sudutKembali; dataSudut1--) {
      Servo1.write(dataSudut1);
      EEPROM.write(addr1, dataSudut1);
      delay(stepServo1);
    }
  }
  if (dataSudut2 >= sudutBuka23) {
    for (dataSudut2; dataSudut2 >= sudutBuka23; dataSudut2--) {
      Servo2.write(dataSudut2);
      EEPROM.write(addr2, dataSudut2);
      delay(stepServo23);
    }
  }
  if (dataSudut3 >= sudutBuka23) {
    for (dataSudut3; dataSudut3 >= sudutBuka23; dataSudut3--) {
      Servo3.write(dataSudut3);
      EEPROM.write(addr3, dataSudut3);
      delay(stepServo23);
    }
  }
}

void Display() {
  lcd.setCursor(0, 0);
  lcd.print("Konveyor: ");

  lcd.setCursor(0, 1);
  lcd.print("Berat: ");


  if (stableValue < 10) {
    lcd.setCursor(7, 1);
    lcd.print(String(stableValue) + "    gr");
  }
  else if (stableValue >= 10 && stableValue < 100) {
    lcd.setCursor(7, 1);
    lcd.print(String(stableValue) + "   gr ");
  }
  else if (stableValue >= 100 && stableValue < 1000) {
    lcd.setCursor(7, 1);
    lcd.print(String(stableValue) + "  gr  ");
  }
  else if (stableValue >= 1000) {
    lcd.setCursor(7, 1);
    lcd.print(String(stableValue) + " gr   ");
  }
}

void readButton() {
  if (digitalRead(buttonStart) == diTekan) {
    digitalWrite(Relay, HIGH);
    Konveyor = ON;
    lcd.setCursor(10, 0);
    lcd.print("ON ");
  }
  else if (digitalRead(buttonStop) == diTekan) {
    digitalWrite(Relay, LOW);
    Konveyor = OFF ;
    lcd.setCursor(10, 0);
    lcd.print("OFF");
  }
}

void readSensor() {
  LoadCell.update();
  sensorValue = LoadCell.getData();
  //  Serial.print("LoadCell Data: ");
  //  Serial.println(sensorValue);

  if (sensorValue <= 0) {
    sensorValue = 0;
  }
}

void setup() {
  Serial.begin(9600);
  EEPROM.begin();
  lcd.init();
  lcd.backlight();
  LoadCell.begin();

  Servo1.attach(pinServo1);
  Servo2.attach(pinServo2);
  Servo3.attach(pinServo3);

  pinMode(buttonStart, INPUT);
  pinMode(buttonStop, INPUT);
  pinMode(Relay, OUTPUT);

  digitalWrite(Relay, LOW);
  Display();
  lcd.setCursor(10, 0);
  lcd.print("OFF ");

  cekKondisi_Awal();

  //  Servo1.write(80);
  //  Servo2.write(0);
  //  Servo3.write(0);

  unsigned long stabilizingtime = 500;
  boolean _tare = true;
  LoadCell.start(stabilizingtime, _tare);

  if (LoadCell.getTareTimeoutFlag()) {
    // Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    // Serial.println("Startup is complete");
  }

}

void loop() {
  readButton();
  readSensor();

  /* Periksa Nilai Sensor (Stabil || !Stabil) */
  if (abs(sensorValue - previousValue) <= tolerance) {
    stableCounter++;
  } else {
    stableCounter = 0;
  }

  /* Jika Nilai Sensor Stabil */
  if (stableCounter >= thresholdStableCount) {
    stableValue = sensorValue;
    Display();

    // Serial.println("Nilai Stabil Terdeteksi: " + String(stableValue));

    /* ------------------------------- VERSI 1 ----------------------------------- */
    /*
      if (stableValue >= beratBuah_Besar && Konveyor == ON) {
      Servo_Tutup2();
      Dorong();
      delay(delayServo2);
      Servo_Buka2();
      stableCounter = 0;
      }
      else if (stableValue > 2 && stableValue < beratBuah_Besar  && Konveyor == ON) {
      Servo_Tutup3();
      Dorong();
      delay(delayServo3);
      Servo_Buka3();
      stableCounter = 0;
      }
      else {
      stableCounter = 0;
      }
    */

    /* ------------------------------- VERSI 2 ----------------------------------- */
    /*
      if (buahKecil && Konveyor == ON) {
      Servo_Tutup2();
      Dorong();
      delay(delayServo2);
      Servo_Buka2();
      stableCounter = 0;
      }
      else if (buahSedangg && Konveyor == ON) {
      Servo_Tutup3();
      Dorong();
      delay(delayServo3);
      Servo_Buka3();
      stableCounter = 0;
      }
      else {
      stableCounter = 0;
      }
    */


    /* ------------------------------- VERSI 3 ----------------------------------- */
    if (buahKecil && Konveyor == ON) {
      Servo_Tutup2();
      Dorong();
      delay(delayServo2);
      Servo_Buka2();
      stableCounter = 0;
    }
    else if (buahSedang && Konveyor == ON) {
      Servo_Tutup3();
      Dorong();
      delay(delayServo3);
      Servo_Buka3();
      stableCounter = 0;
    }
    else if (buahBesar && Konveyor == ON) {
      delay(500);
      Dorong();
      stableCounter = 0;
    }
    else {
      stableCounter = 0;
    }

  }
  previousValue = sensorValue;
  delay(100);
}
