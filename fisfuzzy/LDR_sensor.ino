byte pin_LDR1 = A0; // atas
byte pin_LDR2 = A1; // bawah
byte pin_LDR3 = A2; // kanan
byte pin_LDR4 = A3; // kiri

// setup pin LDR
void LDR_setup() {
  pinMode(pin_LDR1, OUTPUT);
  pinMode(pin_LDR2, OUTPUT);
  pinMode(pin_LDR3, OUTPUT);
  pinMode(pin_LDR4, OUTPUT);
}
