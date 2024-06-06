byte pin_up = 11;
byte pin_down = 10;
byte pin_pwm = 9;

// Setup pin actuator
void aktuator_setup(){
  pinMode(pin_up, OUTPUT);
  pinMode(pin_down, OUTPUT);
  pinMode(pin_pwm, OUTPUT);
}

// Move actuator to UP
void up_aktuator(int pwm){
  // protect psu from high Ampere when H-bridge on low voltage
  if (pwm < 100){
    pwm = 100;
  }
  digitalWrite(pin_up, HIGH);
  digitalWrite(pin_down, LOW);
  analogWrite(pin_pwm, pwm);
  Serial.println("naik");
}

// Move actuator to down
void down_aktuator(int pwm){
  // protect psu from high Ampere when H-bridge on low voltage
  if (abs(pwm) < 100){
    pwm = 100;
  }
  digitalWrite(pin_up, LOW);
  digitalWrite(pin_down, HIGH);
  analogWrite(pin_pwm, abs(pwm));
  Serial.println("down");
}

// Off actuator
void off_aktuator(){
  digitalWrite(pin_up, LOW);
  digitalWrite(pin_down, LOW);
  Serial.println("off");
}
