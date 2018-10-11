/*
  Incorporating code from Tom Igoe's "SD card datalogger"


  SD card attached to Arduino pins as follows:
  MOSI: pin 11
  MISO: pin 12
  SCK:  pin 13
  CS:   pin 4

  sensor: pin 2
  (sketch assumes square waves alternating between 0V and 5V)

*/

#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>

const int CHIPSELECT = 4;
const int LEDPIN = 8;

int startupVal;
String filename;

unsigned long timer;
const unsigned long WAIT = 100; // milliseconds between data writing events

File dataFile;

void setup() {

  attachInterrupt(digitalPinToInterrupt(2), count, RISING);

  Serial.begin(9600);

  initializeEEPROM();

  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(CHIPSELECT)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  else  Serial.println("card initialized.");

  filename = "data_" + String(startupVal) + ".txt";
  Serial.println("this session file: " + filename);
  dataFile = SD.open(filename, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println("*****************************");
    dataFile.println("file: " + filename);
    dataFile.println("record format follows on next line:");
    dataFile.println("millis(),data");
    dataFile.println("*****************************");

    //    dataFile.close();
    dataFile.flush();


    // print the same message to the serial monitor too:
    Serial.println("*****************************");
    Serial.println("file: " + filename);
    Serial.println("record format follows on next line:");
    Serial.println("millis(),data");
    Serial.println("*****************************");
  }
  // if the file isn't open, print an error:
  else {
    Serial.println("error writing startupVal to " + filename);
  }

  // use an external LED as visual indicator of data being recorded to card
  pinMode(LEDPIN, OUTPUT);
}

void loop() {
  int dataVal = analogRead(A0);

  // every WAIT milliseconds
  if (millis() - timer >= WAIT) {
    timer = millis(); // reset timer
    writeRecord(dataVal);
    digitalWrite(LEDPIN, HIGH); // blink LED on
  }

  // turn off LED after 10 milliseconds
  if (millis() - timer > 10) digitalWrite(LEDPIN, LOW);
}

int writeRecord(long dataIn) {

  static unsigned long counter;

  unsigned long now = micros();
  //  dataFile = SD.open(filename, FILE_WRITE);
  //  dataFile = SD.open(filename, O_WRITE);

  unsigned long openDiff = micros() - now;
  Serial.print("openDiff w/O_WRITE = ");
  Serial.println(openDiff);

  // if the file is available, write to it:
  if (dataFile) {
    // build a char array called singleRecord, consisting of "millis(),dataIn" to write to file
    char singleRecord[20] = ""; // reserve space for 19 characters total in the string
    char millisChar[10];
    strcat(singleRecord, ltoa(millis(), millisChar, 10));
    strcat(singleRecord, ",");
    char dataChar[10];
    strcat(singleRecord, itoa(dataIn, dataChar, 10));

    unsigned long writeNow = micros();
    dataFile.println(singleRecord);
    unsigned long writeDiff = micros() - writeNow;
    Serial.print("writeDiff = ");
    Serial.println(writeDiff);


    unsigned long laterNow = micros();

    if (true || (counter % 10 == 0)) dataFile.flush(); // only every tenth
    counter++;
    //    dataFile.close();

    unsigned long flushDiff = micros() - laterNow;
    Serial.print("flushDiff = ");
    Serial.println(flushDiff);
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
  // first-time initialization: if start of EEPROM is all 255's (factory default), zero it all out
  if (255 == EEPROM.read(3) == EEPROM.read(2) == EEPROM.read(1) == EEPROM.read(0)) {
    Serial.println("EEPROM appears uninitialized; overwriting all of it with zeros");
    for (int i = 0; i < 1024; i++) EEPROM.write(i, 0);
  }

  // look up the unique serial value to use for this operation's event
  startupVal = EEPROM.get(0, startupVal);
  // update the record for the next time the Arduino starts
  int nextStartupVal = startupVal + 1;
  EEPROM.put(0, nextStartupVal);
}

void count(){
}
