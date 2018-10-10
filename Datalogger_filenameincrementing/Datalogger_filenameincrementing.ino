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

const int CHIPSELECT = 4; // same pin referred to as "CS" in the pin mapping above
int startupVal; // variable will store the number of times this Arduino has powered on
char filename[15]; // string to store the file name

unsigned long timer; // variable used in timing function below to allow for intermittent events
const unsigned long WAIT = 500; // milliseconds between data writing events

void setup() {

  Serial.begin(9600);

  initializeEEPROM(); // this function appears below the loop()

Serial.print(filenameAndHeaderMessage());
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(CHIPSELECT)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  else  Serial.println("card initialized.");

  // if the file is available, write to it:
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    dataFile.println(filenameAndHeaderMessage());
    dataFile.close();

    // print to the serial port too:
    Serial.println(filenameAndHeaderMessage());
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.print("error writing startupVal to ");
    Serial.println(filename);
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
    Serial.print("error opening ");
    Serial.println(filename);
    return false; // return false on error
  }
}

void initializeEEPROM() {
  // first-time initialization: if start of EEPROM is all 255's (factory default), zero it all out
  if (255 == EEPROM.read(3) == EEPROM.read(2) == EEPROM.read(1) == EEPROM.read(0)) {
    Serial.println("EEPROM appears uninitialized; overwriting all of it with zeros");
    for (int i = 0; i < 1024; i++) EEPROM.write(i, 0);
  }

  // look up the number of times this Arduino has started up, using the EEPROM record at address 0
  startupVal = EEPROM.get(0, startupVal);
  Serial.print("number of times this Arduino switched on (i.e. startupVal) = ");
  Serial.println(startupVal);
  // increment the record for the next time the Arduino starts
  int nextStartupVal = startupVal + 1;
  EEPROM.put(0, nextStartupVal);
}



char filenameAndHeaderMessage() {
  // assemble the filename into a char array called "filename"
  // with format data_123.txt, where 123 is startupVal

  strcat(filename, "data_");
  // itoa() needed to turn an integer (startupVal) into an equivalant char array
  char num[4];
  strcat(filename, itoa(startupVal, num, 10));
  strcat(filename, ".txt");

  char sessionData[50] = "this session file: ";
  strcat(sessionData, filename);
  Serial.println(sessionData);

  char fileHeadMsg[200] = "file name: ";
  strcat(fileHeadMsg, filename); // add the file name
  char afterFileNameHeadMsg[] =
    "record format follows on next line: \n\
    millis(),data\n\
    *******************";
  strcat(fileHeadMsg, afterFileNameHeadMsg); // add the text that follows the file name
  return fileHeadMsg;
}
