uint8_t hujan = 7;
bool nilai3 = 0; 

// Setup pin from rain sensor
void rain_setup() {
  pinMode(hujan, INPUT);
}

// Get rain data from pin
bool get_rain() {
  nilai3 = digitalRead(hujan);
  return !nilai3;
}