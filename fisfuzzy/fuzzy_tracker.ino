#include <ArduinoJson.h>
#include <SoftwareSerial.h>

// Definisikan pin Rx dan Tx untuk koneksi SoftwareSerial dengan ESP8266
const int nanoRxPin = 3; 
const int nanoTxPin = 4; 

// Inisialisasi objek SoftwareSerial untuk komunikasi dengan ESP8266
SoftwareSerial s(nanoRxPin, nanoTxPin);

uint32_t mulai1, mulai2, mulai3, mulai4;
float data_mpu = 0;
bool data_rain = 0;
byte data_rtc = 0;
float data_fldr = 0;
float data_fmpu = 0;
int LDR1 = 0;
int LDR2 = 0;

void setup() {
  Serial.begin(9600);
  // Input: MPU6050 Sensor
  mpu_setup();
  // Input: Rain Sensor
  rain_setup();
  // Input: Clock Sensor
  rtc_setup();
  // Input: LDR
  LDR_setup();
  // Output: Aktuator
  aktuator_setup();
  // data: serial
  s.begin(9600);
}

void loop() {
  // Take MPU6050 data with 1 hz Frequency 
  get_mpu();
  if (millis() - mulai1 > 1000) {
    mulai1 = millis();
    data_mpu = get_x();
  }

  // Take rain, rtc and LDR data with 1 Hz Frequency
  if (millis() - mulai2 > 1000) {
    mulai2 = millis();
    data_rain = get_rain();
    data_rtc = get_rtc();
    LDR1 = map(analogRead(A0), 270, 456, 0, 1023);
    LDR2 = map(analogRead(A1), 270, 456, 0, 1023);
    data_fldr = data_fuzzy_LDR(LDR1, LDR2);
    data_fmpu = data_fuzzy_MPU(data_mpu);
  }

  if(millis() - mulai4 > 1000){
    mulai4 = millis();
    sendesp();
  }
  
  // Move the actuator according to the fuzzy value every second
  if (millis() - mulai3 > 1000) {
    mulai3 = millis();
    Serial.print(data_fmpu);
    Serial.print(" ");
    Serial.print(data_mpu);
    Serial.print(" ");
    Serial.print(data_fldr);
    Serial.print(" ");
    Serial.print(LDR1);
    Serial.print(" ");
    Serial.print(LDR2);
    Serial.print(" ");
    Serial.print(analogRead(A0));
    Serial.print(" ");
    Serial.print(analogRead(A1));
    Serial.print(" ");
    Serial.print(data_rain);
    Serial.print(" ");
    Serial.println(data_rtc);
    
    if (data_rtc > 6 && data_rtc < 18 && data_rain == 0) { // LDR move
      Serial.println("LDR");
      if (data_fldr > -122 && data_fldr < 122) {
        off_aktuator();
      } else if (data_fldr < -122) {
        down_aktuator(data_fldr);
      } else if (data_fldr > 122) {
        up_aktuator(data_fldr);
      }

    } else { // MPU6050 move
      Serial.println("MPU");
      if (data_fmpu > -10.0 && data_fmpu < 10.0) {
        off_aktuator();
      } else if (data_fmpu > -10.0) {
        down_aktuator(data_fmpu);
      } else if (data_fmpu < 10.0) {
        up_aktuator(data_fmpu);
      }
    }
  }
}

void sendesp(){
  StaticJsonDocument<32> doc;
  String jsonParam;

  doc["slope"] = data_mpu;
  doc["is_rain"] = data_rain;

  serializeJson(doc, jsonParam);
  s.println(jsonParam);
}