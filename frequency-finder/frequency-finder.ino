/*
   Frequency finder: assumes a square-wave digital signal on an input pin
   (pin 2 specifically) and computes an exponential running average of the
   frequency of the oscillation of that signal, reported in hertz.

   Because it's using the Arduino's interrupt capability, this counter is
   able to run fairly quickly, and should be able to be used for signals
   in the ~kilohertz range. (This is not tested as of this writing!)

   How it works:
   An "interrupt" pin (a pin with the ability to detect electrical changes at a
   high rate, even while other code is running) is assigned. Whenever pin 2's
   signal goes from 0V to 5V, the "Interrupt Service Routine" (ISR) is immediately
   called, which here runs a function called readPulse(). That function simply
   increments a counter. Every TIMERWAIT milliseconds (the user can change this value),
   the number of increments since the last check is calculated; the instantaneous
   frequency is simply the number of pulses divided by TIMERWAIT in seconds.

   The running average of the frequency of oscillation (in hertz) is stored in
   the variable called "averagedFreq." If the global variable "SERIALFEEDBACK" is
   declared as "true" then the value of "averagedFreq" will be reported to the
   serial monitor every "WAIT" milliseconds, which is useful for debugging. You may
   also use a different piece of software (such as Matlab, Mathematica, etc.) to read
   this value for analysis, storage, control, etc.

   by Robert Zacharias, rzachari@andrew.cmu.edu
   Carnegie Mellon University, Pittsburgh, Pennsylvania
   released to the public domain by the author, 2018
*/

// set SERIALFEEDBACK to true to return serial feedback (optional)
const bool SERIALFEEDBACK = true;

const int SERIALWAIT = 100; // milliseconds between serial prints

// milliseconds between: 1) calculating new instanteous frequency and 2) performing
// exponential smoothing operation including that new data point
const int TIMERWAIT = 100;

// This variable is used to calculate the exponential running average:
// higher OLDWEIGHT means *more* smoothing operates, and lower OLDWEIGHT means
// the newly arrived data points have more weight, i.e. *less* smoothing.
// (0 ≤ OLDWEIGHT ≤ 1)
const float OLDWEIGHT = 0.9;

// do not reassign this pin casually; it needs to be an interrupt-capable pin
const byte READPIN = 2;

// volatile data type needed for the count because its value will be affected by the ISR
volatile unsigned long count;

float averagedFreq, instantaneousFreq; // variables to store frequency data
unsigned long timer; // variable to store last time the frequency calculator ran

void setup() {

  // setup pin 2 as the input for the device
  pinMode(READPIN, INPUT);

  //  assign pin 2 as an "interrupt" pin. In this case, every time a "rising"
  //  signal is seen on pin 2 (i.e. going from 0V to 5V), the function called
  //  readPulse() will immediately run. That function is defined below the loop().
  attachInterrupt(digitalPinToInterrupt(READPIN), readPulse, RISING);

  Serial.begin(115200);
  delay(200);
}

void loop() {

  // every TIMERWAIT milliseconds, calculate new instantaneous frequency
  // and run the exponential smoothing operation with that new data
  if (millis() - timer > TIMERWAIT) {
    static unsigned long lastCount;

    // the variable "count" will be incremented by the ISR function, "readPulse()"
    unsigned long countDiff = count - lastCount;

    // convert from frequency per TIMERWAIT milliseconds to frequency per second
    instantaneousFreq = countDiff * (1000 / TIMERWAIT);

    // the exponential smoothing operation
    averagedFreq = (averagedFreq * OLDWEIGHT) + (instantaneousFreq * (1.0 - OLDWEIGHT));

    // reset counter and timer for next time this if() runs
    lastCount = count;
    timer = millis();
  }

  /*
     User can insert whatever function(s) they'd like based on averagedFreq.
     For instance:

     To calculate a flow rate through a pipe with a hall-effect flow sensor:

        float flowrate = averagedFreq * VALUE;

      where VALUE is an empirically determined constant associated with that particular
      piece of hardware.

      Or:

        if (averagedFreq > 150) {
          something that is triggered at any higher frequency than 150Hz
        }
        else if (averagedFreq > 100) {
          something that's triggered when (100Hz < averagedFreq ≤ 150Hz)
        }
        else {
          something that's triggered when (averagedFreq ≤ 100Hz)
        }

  */

  // optional serial feedback will print the averageFreq every SERIALWAIT milliseconds
  if (SERIALFEEDBACK) {
    static unsigned long lastDebugPrint = 0;
    if (millis() - lastDebugPrint > SERIALWAIT) {
      Serial.println(averagedFreq);
      lastDebugPrint = millis();
    }
  }
}

// the Interrupt Service Routine (ISR) that is called whenever pin 2 transitions 0V -> 5V
void readPulse() {
  count++;
}
