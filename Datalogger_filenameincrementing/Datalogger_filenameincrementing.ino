/*
  Count recorder: assumes a square-wave digital signal on an input pin
  (pin 2 specifically) and reports the number of rising transitions
  (i.e. low->high) on that pin in a specified period of time. These are
  recorded to an attached SD card; each new Arduino power-on a new record
  is created in the format DATA_123.TXT, where 123 is the startup number.
  A startup number higher than 999 may cause file writing to fail because of
  naming limitations in the FAT32 file system.

  Each line of the body of the record contains two comma-separated values, e.g.:

    1234,6789

  Where 1234 is the number of milliseconds since the Arduino powered on and
  6789 is the number of rising pulses counted since startup.

  How it works:
  An "interrupt" pin (a pin with the ability to detect electrical changes at a
  high rate, even while other code is running) is assigned. Whenever pin 2's
  signal goes from 0V to 5V, the "Interrupt Service Routine" (ISR) is immediately
  called, which calls a function called readPulse(); that function simply
  increments a counter. This code running on an Arduino Uno appears to add or drop
  ~3 counts/second when clocking a 10kHz signal. It is more accurate at lower
  frequencies.

  SD card reader pin  | Arduino pin
      MOSI            | pin 11
      MISO            | pin 12
      SCK             | pin 13
      CS              | pin 4

  input signal: pin 2
  LED:          pin 8 (optional; blinks when data is being written)

  by Robert Zacharias, rzachari@andrew.cmu.edu
  Carnegie Mellon University, Pittsburgh, Pennsylvania
  released to the public domain by the author, 2018
  incorporates code from Tom Igoe's "SD card datalogger" example sketch
*/

#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>

const int CHIPSELECT = 4;
const int LEDPIN = 8;

// do not reassign this pin casually; it needs to be an interrupt-capable pin
const byte READPIN = 2;

File dataFile;
int startupVal;
String filename;

unsigned long timer;
const unsigned long WRITEWAIT = 100; // milliseconds between data writing events

// volatile data type needed for the count because its value will be affected by the ISR
volatile unsigned long count;

void setup() {

  //  assign pin 2 as an "interrupt" pin. In this case, every time a "rising"
  //  signal is seen on pin 2 (i.e. going from 0V to 5V), the function called
  //  readPulse() will immediately run. That function is defined below the loop().
  attachInterrupt(digitalPinToInterrupt(READPIN), readPulse, RISING);

  Serial.begin(9600);

  initializeEEPROM(); // checks nonvolatile memory for serial startup number

  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(CHIPSELECT)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  else  Serial.println("card initialized.");

  filename = "DATA_" + String(startupVal) + ".TXT";
  Serial.println("this session file: " + filename);
  dataFile = SD.open(filename, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println("*****************************");
    dataFile.println("file: " + filename);
    dataFile.println("record format follows on next line:");
    dataFile.println("milliseconds elapsed from startup,pulses counted from startup");
    dataFile.println("*****************************");
    dataFile.flush();


    // print the same message to the serial monitor too:
    Serial.println("*****************************");
    Serial.println("file: " + filename);
    Serial.println("record format follows on next line:");
    Serial.println("milliseconds elapsed from startup,pulses counted from startup");
    Serial.println("*****************************");
  }
  // if the file isn't open, print an error:
  else {
    Serial.println("error writing to " + filename);
  }

  // use an external LED as visual indicator of data being recorded to card
  pinMode(LEDPIN, OUTPUT);
}

void loop() {
  // every WRITEWAIT milliseconds
  if (millis() - timer >= WRITEWAIT) {
    timer = millis(); // reset timer
    if (writeRecord(count)) digitalWrite(LEDPIN, HIGH); // blink LED on to show write event happened
  }

  // turn off LED after 10 milliseconds
  if (millis() - timer > 10) digitalWrite(LEDPIN, LOW);
}

int writeRecord(unsigned long dataIn) {

  // if the file is available, write to it:
  if (dataFile) {
    // build a char array called singleRecord, consisting of "millis(),dataIn" to write to file
    // this is to avoid using the String data type in the loop(), which can lead to memory faults
    char singleRecord[20] = ""; // reserve space for 19 characters total in the string
    char millisChar[10];
    strcat(singleRecord, ltoa(millis(), millisChar, 10));
    strcat(singleRecord, ",");
    char dataChar[10];
    strcat(singleRecord, ltoa(dataIn, dataChar, 10));

    dataFile.println(singleRecord); // write the record (a single line) to the SD card
    
    static unsigned long counter;
    if (counter % 10 == 0) dataFile.flush(); // only flush buffer every tenth time
    counter++;

    // print to the serial port too:
    Serial.println(singleRecord);
    return true; // return true on success
  }
  // if the file isn't open or openable, report an error:
  else {
    Serial.println("error opening " + filename);
    return false; // return false on error
  }
}

void   initializeEEPROM() {
  // first-time initialization: if start of EEPROM is 255's (factory default), zero it out
  if (255 == EEPROM.read(1) == EEPROM.read(0)) {
    Serial.println("EEPROM appears uninitialized; resetting startupVal to 0");
    startupVal = 0;
    EEPROM.put(0, startupVal);
  }

  // look up the unique serial value to use for this operation's event
  startupVal = EEPROM.get(0, startupVal);
  // update the record for the next time the Arduino starts
  int nextStartupVal = startupVal + 1;
  EEPROM.put(0, nextStartupVal);
}

// the Interrupt Service Routine (ISR) that is called whenever pin 2 transitions 0V -> 5V
void readPulse() {
  count++;
}
