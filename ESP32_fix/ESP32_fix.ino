#include <WiFi.h>
#include <ModbusMasterPzem017.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

static uint8_t pzemSlaveAddr = 0x01;    //PZem Address
static uint16_t NewshuntAddr = 0x0001;  // Declare your external shunt value. Default is 100A, replace to "0x0001" if using 50A shunt, 0x0002 is for 200A, 0x0003 is for 300A
static uint8_t resetCommand = 0x42;     // reset energy command

ModbusMaster node;
float PZEMVoltage = 0;
float PZEMCurrent = 0;
float PZEMPower = 0;
float PZEMEnergy = 0;

float raw_voltage, raw_current, raw_power;
float voltage, current, power;

// Deklarasi Variable dan Konstanta
const char* wifiSSID = "NusantaraTracker";
const char* wifiPassword = "1234567890";

WiFiClient client;

//setup counter periode
int raw_counter = 1;
int counter;
byte periode = 60;  // banyak nya data yang diambil

//setup read Smart System Tracker
String slope = "2.67";
int is_rain = 0;
String id = "move";

String data;

//setup millis()
uint32_t mulai1, mulai2, mulai3, mulai4;

TaskHandle_t Task1;

void setup() {
  // put your setup code here, to run once:
  connectWifi();
  delay(1000);
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N2);  // Pzem
  Serial2.begin(9600);              // SST
  // setShunt(pzemSlaveAddr);
  node.begin(pzemSlaveAddr, Serial1);
  delay(1000);

  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */

  //  Connect to WiFi access point.
}

void loop() {

  if (millis() - mulai1 > 1000) {
    mulai1 = millis();
    uint8_t result;
    result = node.readInputRegisters(0x0000, 6);
    if (result == node.ku8MBSuccess) {
      uint32_t tempdouble = 0x00000000;
      PZEMVoltage = node.getResponseBuffer(0x0000) / 100.0;
      PZEMCurrent = node.getResponseBuffer(0x0001) / 100.0;
      tempdouble = (node.getResponseBuffer(0x0003) << 16) + node.getResponseBuffer(0x0002);  // get the power value. Power value is consists of 2 parts (2 digits of 16 bits in front and 2 digits of 16 bits at the back) and combine them to an unsigned 32bit
      PZEMPower = tempdouble / 10.0;                                                         //Divide the value by 10 to get actual power value (as per manual)
      tempdouble = (node.getResponseBuffer(0x0005) << 16) + node.getResponseBuffer(0x0004);  //get the energy value. Energy value is consists of 2 parts (2 digits of 16 bits in front and 2 digits of 16 bits at the back) and combine them to an unsigned 32bit
      PZEMEnergy = tempdouble;

      raw_voltage += PZEMVoltage;
      raw_current += PZEMCurrent;
      raw_power += PZEMPower;

      // Tambahkan nilai raw_counter
      raw_counter += 1;
    } else {
      Serial.println("Failed to read modbus");
    }
  }

  if (raw_counter > periode) {
    voltage = raw_voltage / periode;
    current = raw_current / periode;
    power = raw_power / periode;

    raw_counter = 1;  //Reset raw_counter
    raw_voltage = 0;
    raw_current = 0;
    raw_power = 0;

    postHTTP(voltage, current, power);
  }
}

void Task1code(void* pvParameters) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    while (Serial2.available() > 0) {
      delay(10);
      char c = Serial2.read();
      data += c;
    }

    if (data.length() > 0) {
      Serial.println(data);
      slope = data;
      data = "";
    }
  }
}


void postHTTP(float v, float c, float p) {
  Serial.println("Posting...");
  String url = // your API here;
  HTTPClient http;
  String response;

  StaticJsonDocument<200> buff;
  String jsonParams;

  buff["voltage"] = v;
  buff["current"] = c;
  buff["is_rain"] = is_rain;
  buff["slope"] = slope;
  buff["power"] = p;
  buff["module"] = id;

  serializeJson(buff, jsonParams);
  Serial.println(jsonParams);

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(jsonParams);
  if (httpCode > 0) {
    response = http.getString();
    Serial.println(response);
  } else {
    Serial.println("HTTP POST failed!");
    WiFi.disconnect();
    WiFi.reconnect();
  }
  http.end();
}

void connectWifi() {
  Serial.println("Connecting To Wifi");
  WiFi.begin(wifiSSID, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("Wifi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setShunt(uint8_t slaveAddr) {
  static uint8_t SlaveParameter = 0x06;     /* Write command code to PZEM */
  static uint16_t registerAddress = 0x0003; /* change shunt register address command code */

  uint16_t u16CRC = 0xFFFF;                  /* declare CRC check 16 bits*/
  u16CRC = crc16_update(u16CRC, slaveAddr);  // Calculate the crc16 over the 6bytes to be send
  u16CRC = crc16_update(u16CRC, SlaveParameter);
  u16CRC = crc16_update(u16CRC, highByte(registerAddress));
  u16CRC = crc16_update(u16CRC, lowByte(registerAddress));
  u16CRC = crc16_update(u16CRC, highByte(NewshuntAddr));
  u16CRC = crc16_update(u16CRC, lowByte(NewshuntAddr));

  Serial.println("Change shunt address");
  Serial1.write(slaveAddr);  //these whole process code sequence refer to manual
  Serial1.write(SlaveParameter);
  Serial1.write(highByte(registerAddress));
  Serial1.write(lowByte(registerAddress));
  Serial1.write(highByte(NewshuntAddr));
  Serial1.write(lowByte(NewshuntAddr));
  Serial1.write(lowByte(u16CRC));
  Serial1.write(highByte(u16CRC));
  delay(10);
  delay(100);
  while (Serial1.available()) {
    Serial.print(char(Serial1.read()), HEX);  //Prints the response and display on Serial Monitor (Serial)
    Serial.print(" ");
  }
}  //setShunt Ends
