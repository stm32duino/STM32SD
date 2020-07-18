/*
  SD card datalogger

 This example shows how to log data from three analog sensors
 to an SD card using the SD library.

 The circuit:
 * analog sensors on analog ins A0, A1, and A2
 * SD card

 */

#include <STM32SD.h>

// If SD card slot has no detect pin then define it as SD_DETECT_NONE
// to ignore it. One other option is to call 'SD.begin()' without parameter.
#ifndef SD_DETECT_PIN
#define SD_DETECT_PIN SD_DETECT_NONE
#endif

uint32_t A[] = { A0, A1, A2};

File dataFile;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  while (!SD.begin(SD_DETECT_PIN))
  {
    delay(10);
  }
  delay(100);
  Serial.println("card initialized.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  // if the file is available, seek to last position
  if (dataFile) {
    dataFile.seek(dataFile.size());
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
}

void loop()
{
  // make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(A[analogPin]);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";
    }
  }


  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.flush(); // use flush to ensure the data written
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error on datalog.txt file handle");
  }
  delay(100);
}
