/*
  Incorporating code from Tom Igoe's "SD card datalogger"


   SD card attached to Arduino pins as follows:
   MOSI - pin 11
   MISO - pin 12
   SCK - pin 13
   CS - pin 4

*/

#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>

const int CHIPSELECT = 4;
int startupVal;
String filename;

unsigned long timer;
const unsigned long WAIT = 500; // milliseconds between data writing events

void setup() {

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
  File dataFile = SD.open(filename, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    String hey = String("hey") + String("you");
    String startupMessage = String("*****************************") + "file: " + filename;
    dataFile.println("*****************************");
    dataFile.println("file: " + filename);
    dataFile.println("record format follows on next line:");
    dataFile.println("millis(),data");
    dataFile.println("*****************************");
    dataFile.close();
    // print to the serial port too:
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error writing startupVal to " + filename);
  }
}

void loop() {
  int dataVal = analogRead(A0);

  // twice per second
  if (millis() - timer >= WAIT) {
    writeData(dataVal);
    timer = millis();
  }
}

int writeData(long dataIn) {
  File dataFile = SD.open(filename, FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    String singleRecord = String(millis()) + "," + String(dataIn);
    dataFile.println(singleRecord);
    dataFile.close();
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
