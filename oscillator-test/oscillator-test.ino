const int OUT = 2;

void setup() {
  pinMode(OUT, OUTPUT);
  
  toneVersion();
}

void loop() {
  //timerVersion();
  //delayVersion();
}

void toneVersion() {
  tone(OUT, 1000);
}

void timerVersion() {
  static bool outState;
  static unsigned long timer;

  if (millis() - timer >= 1) {
    outState = !outState;
    timer = millis();
  }

  digitalWrite(OUT, outState);
}

void delayVersion() {
  digitalWrite(OUT, HIGH);
  delay(1);
  digitalWrite(OUT, LOW);
  delay(1);
}
