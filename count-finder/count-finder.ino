/*
   Count finder: assumes a square-wave digital signal on an input pin
   (pin 2 specifically) and reports the number of rising transitions
   (i.e. low->high) on that pin in a specified period of time.

   Because it's using the Arduino's interrupt capability, this counter is
   able to run fairly quickly, and should be able to be used for signals
   in the ~kilohertz range. (This is not tested as of this writing!)

   How it works:
   An "interrupt" pin (a pin with the ability to detect electrical changes at a
   high rate, even while other code is running) is assigned. Whenever pin 2's
   signal goes from 0V to 5V, the "Interrupt Service Routine" (ISR) is immediately
   called, which here runs a function called readPulse(). That function simply
   increments a counter.

   If the global variable "SERIALFEEDBACK" is declared as "true" then every
   "SERIALWAIT" milliseconds a line of data will be written to the serial monitor:

   1234,6789

   Where 1234 is the current time in milliseconds since the Arduino powered on, and
   6789 is the value of count at the time of reporting.

   by Robert Zacharias, rzachari@andrew.cmu.edu
   Carnegie Mellon University, Pittsburgh, Pennsylvania
   released to the public domain by the author, 2018
*/

// set SERIALFEEDBACK to true to return serial feedback (optional)
const bool SERIALFEEDBACK = true;
const int SERIALWAIT = 100; // milliseconds between serial prints

// do not reassign this pin casually; it needs to be an interrupt-capable pin
const byte READPIN = 2;

// volatile data type needed for the count because its value will be affected by the ISR
volatile unsigned long count;

void setup() {

  //  assign pin 2 as an "interrupt" pin. In this case, every time a "rising"
  //  signal is seen on pin 2 (i.e. going from 0V to 5V), the function called
  //  readPulse() will immediately run. That function is defined below the loop().
  attachInterrupt(digitalPinToInterrupt(READPIN), readPulse, RISING);

  // start serial data transmission at 115,200 bits per second
  Serial.begin(115200);
}

void loop() {
  // optional serial feedback will print the averageFreq every SERIALWAIT milliseconds
  if (SERIALFEEDBACK) {
    static unsigned long lastSerialPrint = 0;
    if (millis() - lastSerialPrint >= SERIALWAIT) {
      Serial.print(millis());
      Serial.print(",");
      Serial.println(count);
      lastSerialPrint = millis();
    }
  }
}

// the Interrupt Service Routine (ISR) that is called whenever pin 2 transitions 0V -> 5V
void readPulse() {
  count++;
}
