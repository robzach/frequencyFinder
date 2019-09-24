/*
  Distance logger: records ultrasonic ranger distances on an SD card. Each 
  new Arduino power-on a new file is created in the format DATA_123.TXT, 
  where 123 is the startup number.

  Each line of the body of the data file contains two comma-separated values, e.g.:

    1234,6789

  Where 1234 is the number of milliseconds since the Arduino powered on and
  6789 is the centimeter range recorded at that moment.

  To reset the sequential startup counter to 1, turn the boolean value RESETSTARTUPCOUNTER
  to true and upload the code; then turn the boolean back to false and upload again.

  Wiring:

  ultrasonic ranger pin | Arduino pin
    trigger             | pin 7
    echo                | pin 6
  
  LED: pin 2 (optional; blinks when data is being written to SD card)

  SD card reader pin  | Arduino pin
      MOSI            | pin 11
      MISO            | pin 12
      SCK             | pin 13
      CS              | pin 4

  by Robert Zacharias, rzachari@andrew.cmu.edu
  Carnegie Mellon University, Pittsburgh, Pennsylvania
  released to the public domain by the author, 2019
  incorporates code from Tom Igoe's "SD card datalogger" example sketch
*/

#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>
#include <NewPing.h>

const int TRIGGER_PIN = 7;
const int ECHO_PIN = 6;
const int MAX_DISTANCE = 200;

const int LEDPIN = 2;

const int CHIPSELECT = 4;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.


File dataFile;
int startupVal;
String filename;

unsigned long timer;
const unsigned long WRITEWAIT = 100; // milliseconds between data writing events

// set this boolean to true to restart the sequential startup counter
const bool RESETSTARTUPCOUNTER = false;

void setup() {

  Serial.begin(9600);

  initializeEEPROM(); // checks nonvolatile memory for sequential startup number

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
  dataFile = SD.open(filename, FILE_WRITE); // open the file once; it won't be closed

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println("*****************************");
    dataFile.println("file: " + filename);
    dataFile.println("record format follows on next line:");
    dataFile.println("milliseconds elapsed from startup,centimeter distance recorded");
    dataFile.println("*****************************");
    dataFile.flush();


    // print the same message to the serial monitor too:
    Serial.println("*****************************");
    Serial.println("file: " + filename);
    Serial.println("record format follows on next line:");
    Serial.println("milliseconds elapsed from startup,centimeter distance recorded");
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
    if (writeRecord(sonar.ping_cm())) digitalWrite(LEDPIN, HIGH); // blink LED on to show write event happened
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
    if (counter % 10 == 0) dataFile.flush(); // only flush buffer (push data to card) every tenth time
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

void initializeEEPROM() {
  
  // if start of EEPROM is 255's (factory default), or if RESETSTARTUPCOUNTER is
  // set to true, then zero out the sequential counter
  if ((255 == EEPROM.read(1) == EEPROM.read(0)) || RESETSTARTUPCOUNTER) {
    if (RESETSTARTUPCOUNTER) Serial.println("resetting startup counter to 0");
    else Serial.println("EEPROM appears uninitialized; resetting startup counter to 0");
    startupVal = 0;
    EEPROM.put(0, startupVal);
  }

  // look up the unique sequential value to use for this operation
  startupVal = EEPROM.get(0, startupVal);
  // update the value for the next time the Arduino starts
  int nextStartupVal = startupVal + 1;
  EEPROM.put(0, nextStartupVal);
}
