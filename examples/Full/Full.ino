#include <STM32SD.h>

// If SD card slot has no detect pin then define it as SD_DETECT_NONE
// to ignore it. One other option is to call 'SD.begin()' without parameter.
#ifndef SD_DETECT_PIN
#define SD_DETECT_PIN SD_DETECT_NONE
#endif

#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
#define BUFFERSIZE                       (COUNTOF(wtext) -1)

uint32_t file_size = 0, seek_val = false, peek_val = 0;
uint32_t byteswritten, bytesread = 0;
/* File write buffer */
uint8_t wtext[] =  "This is the Arduino SD Test working with FatFs.";
/* File read buffer */
uint8_t rtext[BUFFERSIZE];
uint32_t i = 0;
bool isdir = false;
File MyFile;

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  /* Test begin() method */
  while (!SD.begin(SD_DETECT_PIN))
  {
    delay(10);
  }
  delay(100);

  /* Test mkdir() method */
  Serial.print("Creating 'STM32' directory...");
  if (SD.mkdir("STM32")) {
    Serial.println("OK");
  } else {
    Serial.println("KO");
  }
  Serial.print("Creating 'ARDUINO' directory...");
  if (SD.mkdir("ARDUINO")) {
    Serial.println("OK");
  } else {
    Serial.println("KO");
  }
  Serial.print("Creating 'ARDUINO/SD' directory...");
  if (SD.mkdir("ARDUINO/SD")) {
    Serial.println("OK");
  } else {
    Serial.println("KO");
  }

  /* Test bool operator method */
  Serial.print("Test bool operator...");
  if (!MyFile) {
    Serial.println("OK");
  } else {
    Serial.println("KO --> Error MyFile should not be initialized!");
  }

  /* Test open() method */
  Serial.print("Opening 'STM32/Toremove.txt' file...");
  MyFile = SD.open("STM32/Toremove.txt", FILE_WRITE);
  if (MyFile) {
    Serial.println("OK");
    Serial.print("Closing 'STM32/Toremove.txt' file...");
    MyFile.close();
    Serial.println("OK");
  } else {
    Serial.println("KO --> Error to open 'STM32/Toremove.txt' file");
  }
  Serial.print("Opening 'ARDUINO/SD/ARDUINO_SD_TEXT.txt' file...");
  MyFile = SD.open("ARDUINO/SD/ARDUINO_SD_TEXT.txt", FILE_WRITE);
  if (MyFile) {
    Serial.println("OK");
    /* Test print() method */
    Serial.print("  writing \"");
    Serial.print((const char*)wtext);
    Serial.print("\" into ARDUINO_SD_TEXT.txt file...");
    byteswritten = MyFile.print((const char*)wtext);
    byteswritten += MyFile.print("\n");
    Serial.print(byteswritten, DEC);
    Serial.println(" bytes written.");
    Serial.print("Closing 'ARDUINO/SD/ARDUINO_SD_TEXT.txt' file...");
    MyFile.close();
    Serial.println("OK");
  } else {
    Serial.println("KO --> Error to open 'ARDUINO/SD/ARDUINO_SD_TEXT.txt' file");
  }

  Serial.print("Opening 'ARDUINO/SD/ARDUINO_SD_TEXT.txt' file...");
  MyFile = SD.open("ARDUINO/SD/ARDUINO_SD_TEXT.txt");
  if (MyFile) {
    Serial.println("OK");
    Serial.print("  reading ARDUINO/SD/ARDUINO_SD_TEXT.txt file...");
    bytesread = MyFile.read(rtext, MyFile.size());
    Serial.print(bytesread, DEC);
    Serial.println(" bytes read.");
    Serial.print("Closing 'ARDUINO/SD/ARDUINO_SD_TEXT.txt' file...");
    MyFile.close();
    Serial.println("OK");
  } else {
    Serial.println("KO -- > Error to open 'ARDUINO/SD/ARDUINO_SD_TEXT.txt' file");
  }

  Serial.print("Opening 'ARDUINO/SD/TEXT.txt' file...");
  MyFile = SD.open("ARDUINO/SD/TEXT.txt", FILE_WRITE);
  if (MyFile) {
    Serial.println("OK");
    Serial.print("  writing and flush \"");
    Serial.print((const char*)rtext);
    Serial.print("\" into ARDUINO/SD/TEXT.txt file...");
    byteswritten = MyFile.print((const char*)rtext);
    MyFile.flush();
    Serial.print(byteswritten, DEC);
    Serial.println(" bytes written.");
    Serial.print("Closing 'ARDUINO/SD/TEXT.txt' file...");
    MyFile.close();
    Serial.println("OK");
  } else {
    Serial.println("KO --> Error to open 'ARDUINO/SD/TEXT.txt' file");
  }

  Serial.print("Opening 'ARDUINO/SD/TEXT.txt' file...");
  MyFile = SD.open("ARDUINO/SD/TEXT.txt");
  if (MyFile) {
    Serial.println("OK");
    /* Test size() method */
    file_size = MyFile.size();
    Serial.print("  TEXT.txt size: ");
    Serial.println(file_size);

    /* Test position and seek method */
    Serial.print("  TEXT.txt position value: ");
    Serial.println(MyFile.position());
    Serial.print("  TEXT.txt seek value over size: ");
    if (!MyFile.seek(MyFile.size() + 1)) {
      Serial.println("OK");
    } else {
      Serial.println("KO");
    }
    Serial.print("  TEXT.txt seek value to size: ");
    if (MyFile.seek(MyFile.size())) {
      Serial.println("OK");
    } else {
      Serial.println("KO");
    }
    Serial.print("  TEXT.txt position value: ");
    Serial.println(MyFile.position());
    Serial.print("  TEXT.txt seek value to 0: ");
    if (MyFile.seek(0)) {
      Serial.println("OK");
    } else {
      Serial.println("KO");
    }
    Serial.print("  TEXT.txt position value: ");
    Serial.println(MyFile.position());

    /* Test peek() method */
    Serial.println("  TEXT.txt peek (10 times): ");
    for (i = 0; i < 10; i++)
    {
      peek_val = MyFile.peek();
      Serial.print("    ");
      Serial.print(peek_val);
      Serial.print(" '");
      Serial.write(peek_val);
      Serial.println("'");
    }
    i = 0;

    /* Test available() and read() methods */
    Serial.println("  TEXT.txt content read byte per byte: ");
    while (MyFile.available())
    {
      rtext[i] = (uint8_t)MyFile.read();
      Serial.print("    ");
      Serial.print(rtext[i]);
      Serial.print(" '");
      Serial.write(rtext[i]);
      Serial.println("'");
      i++;
    }
    /* Test close method */
    Serial.print("Closing 'ARDUINO/SD/TEXT.txt' file...");
    MyFile.close();
    Serial.println("OK");
  } else {
    Serial.println("KO --> Error to open 'ARDUINO/SD/TEXT.txt' file");
  }

  /* Test isDirectory() method */
  Serial.print("Opening 'STM32' dir...");
  MyFile = SD.open("STM32");
  if (MyFile) {
    Serial.println("OK");
    Serial.print("Is 'STM32' is a dir...");
    if (MyFile.isDirectory()) {
      Serial.println("OK");
    } else {
      Serial.println("KO");
    }
    MyFile.close();
  } else {
    Serial.println("KO --> Error to open 'STM32' dir");
  }

  Serial.print("Opening 'STM32/Toremove.txt' file...");
  MyFile = SD.open("STM32/Toremove.txt");
  if (MyFile) {
    Serial.println("OK");
    Serial.print("  Is 'STM32/Toremove.txt' is a file: ");
    if (MyFile.isDirectory()) {
      Serial.println("KO");
    } else {
      Serial.println("OK");
    }
    Serial.print("Closing 'STM32/Toremove.txt' file...");
    MyFile.close();
    Serial.println("OK");
  } else {
    Serial.println("KO --> Error to open 'STM32/Toremove.txt' file");
  }
  /* Test exists(), remove() and rmdir() methods */
  Serial.print("Removing 'STM32/Toremove.txt' file...");
  while (SD.exists("STM32/Toremove.txt") == true)
  {
    SD.remove("STM32/Toremove.txt");
  }
  Serial.println("OK");

  Serial.print("Removing 'STM32' dir...");
  while (SD.exists("STM32") == true)
  {
    SD.rmdir("STM32");
  }
  Serial.println("OK");

  /* Test println(), println(data) methods */
  Serial.print("Opening 'ARDUINO/SD/PRINT.txt' file...");
  MyFile = SD.open("ARDUINO/SD/PRINT.txt", FILE_WRITE);
  if (MyFile) {
    Serial.println("OK");
    String str = String("This is a String object on line 7");
    Serial.print("  Printing to 'ARDUINO/SD/PRINT.txt' file...");
    MyFile.println("This should be line 1");
    MyFile.println();
    MyFile.println("This should be line 3");
    MyFile.println("This should be line 4");
    MyFile.println();
    MyFile.println("This should be line 6");
    MyFile.println(str);
    Serial.println("OK");
    Serial.print("Closing 'ARDUINO/SD/PRINT.txt' file");
    Serial.println("OK");
    MyFile.close();
  } else {
    Serial.println("KO --> Error to open 'ARDUINO/SD/PRINT.txt' file");
  }

  /* Test write(buf, len) method */
  Serial.print("Opening 'ARDUINO/SD/WRITE.txt' file...");
  MyFile = SD.open("ARDUINO/SD/WRITE.txt", FILE_WRITE);
  if (MyFile) {
    Serial.println("OK");
    Serial.print("  Writing 'ARDUINO/SD/WRITE.txt' file: ");
    byteswritten = MyFile.write(wtext, BUFFERSIZE);
    Serial.print(byteswritten);
    Serial.println(" bytes written");
    Serial.print("Closing 'ARDUINO/SD/WRITE.txt' file");
    MyFile.close();
    Serial.println("OK");
  } else {
    Serial.println("KO --> Error to open 'ARDUINO/SD/WRITE.txt' file");
  }

  /* Test read(buf, len) method */
  Serial.print("Opening 'ARDUINO/SD/WRITE.txt' file...");
  MyFile = SD.open("ARDUINO/SD/WRITE.txt");
  if (MyFile) {
    Serial.println("OK");
    Serial.print("  Reading 'ARDUINO/SD/WRITE.txt' file: ");
    bytesread = MyFile.read(rtext, MyFile.size());
    Serial.print(bytesread);
    Serial.println(" bytes read");
    Serial.println((const char*)rtext);
    Serial.print("Closing 'ARDUINO/SD/WRITE.txt' file...");
    MyFile.close();
    Serial.println("OK");
  } else {
    Serial.println("KO --> Error to open 'ARDUINO/SD/WRITE.txt' file");
  }
  Serial.println("###### End of the SD tests ######");
}

void loop()
{
  // do nothing
}
