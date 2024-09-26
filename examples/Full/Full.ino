#include <STM32SD.h>

// If SD card slot has no detect pin then define it as SD_DETECT_NONE
// to ignore it. One other option is to call 'SD.begin()' without parameter.
#ifndef SD_DETECT_PIN
#define SD_DETECT_PIN SD_DETECT_NONE
#endif

#define COUNTOF(__BUFFER__) (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
#define BUFFERSIZE (COUNTOF(wtext) - 1)

uint32_t file_size = 0, seek_val = false, peek_val = 0;
uint32_t byteswritten, bytesread = 0;
/* File write buffer */
const char wtext[] = "This is the Arduino SD Test working with FatFs.";
/* File read buffer including extra "\n\0" */
char rtext[BUFFERSIZE + 2] = { '\0' };
uint32_t i = 0;
bool isdir = false;
File MyFile;

typedef struct {
  const char* tst;
  const char* err;
  bool res;
} result_t;


result_t aResult[] = {
  { .tst = "Creating 'STM32' directory...", .err = NULL, .res = false },
  { .tst = "Creating 'ARDUINO' directory...", .err = NULL, .res = false },
  { .tst = "Creating 'ARDUINO/SD' directory...", .err = NULL, .res = false },
  { .tst = "Test bool operator...", .err = "Error MyFile should not be initialized!", .res = false },
  { .tst = "Opening 'STM32/Toremove.txt' file...", .err = "Error to open 'STM32/Toremove.txt' file.", .res = false },
  { .tst = "Opening 'ARDUINO/SD/ARDUINO_SD_TEXT.txt' file...", .err = "Error to open 'ARDUINO/SD/ARDUINO_SD_TEXT.txt' file", .res = false },
  { .tst = "  Writing `This is the Arduino SD Test working with FatFs.` into ARDUINO_SD_TEXT.txt file...", .err = NULL, .res = false },
  { .tst = "Opening 'ARDUINO/SD/ARDUINO_SD_TEXT.txt' file...", .err = "Error to open 'ARDUINO/SD/ARDUINO_SD_TEXT.txt' file", .res = false },
  { .tst = "  Reading ARDUINO/SD/ARDUINO_SD_TEXT.txt file...", .err = NULL, .res = false },
  { .tst = "Opening 'ARDUINO/SD/TEXT.txt' file...", .err = "Error to open 'ARDUINO/SD/TEXT.txt' file", .res = false },
  { .tst = "  Writing and flush `This is the Arduino SD Test working with FatFs.` into ARDUINO/SD/TEXT.txt file...", .err = NULL, .res = false },
  { .tst = "Opening 'ARDUINO/SD/TEXT.txt' file...", .err = "Error to open 'ARDUINO/SD/TEXT.txt' file", .res = false },
  { .tst = "  TEXT.txt size:", .err = NULL, .res = false },
  { .tst = "  TEXT.txt position value:", .err = NULL, .res = false },
  { .tst = "  TEXT.txt seek value over size:", .err = NULL, .res = false },
  { .tst = "  TEXT.txt seek value to size:", .err = NULL, .res = false },
  { .tst = "  TEXT.txt position value: ", .err = NULL, .res = false },
  { .tst = "  TEXT.txt seek value to 0: ", .err = NULL, .res = false },
  { .tst = "  TEXT.txt position value: ", .err = NULL, .res = false },
  { .tst = "  TEXT.txt peek (10 times): ", .err = NULL, .res = false },
  { .tst = "  TEXT.txt content read byte per byte: ", .err = NULL, .res = false },
  { .tst = "Opening 'STM32' dir...", .err = "Error to open 'STM32' dir", .res = false },
  { .tst = "Is 'STM32' is a dir...", .err = NULL, .res = false },
  { .tst = "Opening 'STM32/Toremove.txt' file...", .err = "Error to open 'STM32/Toremove.txt' file", .res = false },
  { .tst = "  Is 'STM32/Toremove.txt' is a file: ", .err = NULL, .res = false },
  { .tst = "Removing 'STM32/Toremove.txt' file...", .err = NULL, .res = false },
  { .tst = "Removing 'STM32' dir...", .err = NULL, .res = false },
  { .tst = "Opening 'ARDUINO/SD/PRINT.txt' file...", .err = "Error to open 'ARDUINO/SD/PRINT.txt' file", .res = false },
  { .tst = "  Printing to 'ARDUINO/SD/PRINT.txt' file...", .err = NULL, .res = false },
  { .tst = "Opening 'ARDUINO/SD/WRITE.txt' file...", .err = "Error to open 'ARDUINO/SD/WRITE.txt' file", .res = false },
  { .tst = "  Writing 'ARDUINO/SD/WRITE.txt' file: ", .err = NULL, .res = false },
  { .tst = "Opening 'ARDUINO/SD/WRITE.txt' file...", .err = "Error to open 'ARDUINO/SD/WRITE.txt' file", .res = false },
  { .tst = "  Reading 'ARDUINO/SD/WRITE.txt' file: ", .err = NULL, .res = false },
  { .tst = "Opening 'ARDUINO/SD/WRITE.txt' file...", .err = "Error to open 'ARDUINO/SD/WRITE.txt' file", .res = false },
  { .tst = "  Reading 'ARDUINO/SD/WRITE.txt' file: ", .err = NULL, .res = false },
  { .tst = "End SD access...", .err = NULL, .res = false },
};

void setup() {
  uint32_t idx = 0;

  Serial.begin(9600);
  while (!Serial)
    ;

  /* Test begin() method */
  while (!SD.begin(SD_DETECT_PIN)) {
    delay(10);
  }
  delay(100);

  /* Test mkdir() method */
  aResult[idx++].res = SD.mkdir("STM32");
  aResult[idx++].res = SD.mkdir("ARDUINO");
  aResult[idx++].res = SD.mkdir("ARDUINO/SD");

  /* Test bool operator method */
  aResult[idx++].res = !MyFile;

  /* Test open() method */
  MyFile = SD.open("STM32/Toremove.txt", FILE_WRITE);
  if (MyFile) {
    aResult[idx++].res = true;
    MyFile.close();
  } else {
    idx += 1;
  }

  MyFile = SD.open("ARDUINO/SD/ARDUINO_SD_TEXT.txt", FILE_WRITE);
  if (MyFile) {
    aResult[idx++].res = true;
    /* Test print() method */
    byteswritten = MyFile.print(wtext);
    byteswritten += MyFile.print("\n");
    aResult[idx++].res = (byteswritten == (BUFFERSIZE + 1));
    MyFile.close();
  } else {
    idx += 2;
  }

  MyFile = SD.open("ARDUINO/SD/ARDUINO_SD_TEXT.txt");
  if (MyFile) {
    aResult[idx++].res = true;
    bytesread = MyFile.read(rtext, MyFile.size());
    aResult[idx++].res = (bytesread == (BUFFERSIZE + 1));
    MyFile.close();
  } else {
    idx += 2;
  }

  MyFile = SD.open("ARDUINO/SD/TEXT.txt", FILE_WRITE);
  if (MyFile) {
    aResult[idx++].res = true;
    byteswritten = MyFile.print(rtext);
    MyFile.flush();
    aResult[idx++].res = (byteswritten == (BUFFERSIZE + 1));
    MyFile.close();
  } else {
    idx += 2;
  }

  MyFile = SD.open("ARDUINO/SD/TEXT.txt");
  if (MyFile) {
    aResult[idx++].res = true;
    /* Test size() method */
    file_size = MyFile.size();
    aResult[idx++].res = (file_size == (BUFFERSIZE + 1));
    /* Test position and seek method */
    aResult[idx++].res = (MyFile.position() == 0);
    aResult[idx++].res = !MyFile.seek(MyFile.size() + 1);
    aResult[idx++].res = MyFile.seek(MyFile.size());
    aResult[idx++].res = (MyFile.position() == (BUFFERSIZE + 1));
    aResult[idx++].res = MyFile.seek(0);
    aResult[idx++].res = (MyFile.position() == 0);

    /* Test peek() method */
    aResult[idx].res = true;
    for (i = 0; i < 10; i++) {
      if (MyFile.peek() != wtext[0]) {
        aResult[idx].res = false;
      }
    }
    i = 0;
    idx++;

    /* Test available() and read() methods */
    /* skip '\n' */
    aResult[idx].res = true;
    while (MyFile.available() - 1) {
      if (MyFile.read() != wtext[i++]) {
        aResult[idx].res = false;
      }
    }
    idx++;
    MyFile.close();
  } else {
    idx += 10;
  }

  /* Test isDirectory() method */
  MyFile = SD.open("STM32");
  if (MyFile) {
    aResult[idx++].res = true;
    aResult[idx++].res = MyFile.isDirectory();
    MyFile.close();
  } else {
    idx += 2;
  }

  MyFile = SD.open("STM32/Toremove.txt");
  if (MyFile) {
    aResult[idx++].res = true;
    aResult[idx++].res = !MyFile.isDirectory();
    MyFile.close();
  } else {
    idx += 2;
  }
  /* Test exists(), remove() and rmdir() methods */
  while (SD.exists("STM32/Toremove.txt") == true) {
    SD.remove("STM32/Toremove.txt");
  }
  aResult[idx++].res = true;
  while (SD.exists("STM32") == true) {
    SD.rmdir("STM32");
  }
  aResult[idx++].res = true;

  /* Test println(), println(data) methods */
  MyFile = SD.open("ARDUINO/SD/PRINT.txt", FILE_WRITE);
  if (MyFile) {
    aResult[idx++].res = true;
    String str = String("This is a String object on line 7");
    MyFile.println("This should be line 1");
    MyFile.println();
    MyFile.println("This should be line 3");
    MyFile.println("This should be line 4");
    MyFile.println();
    MyFile.println("This should be line 6");
    MyFile.println(str);
    MyFile.print("This should be line ");
    MyFile.println(8);
    aResult[idx++].res = (MyFile.size() == 154);
    MyFile.close();
  } else {
    idx += 2;
  }

  /* Test write(buf, len) method */
  MyFile = SD.open("ARDUINO/SD/WRITE.txt", FILE_WRITE);
  if (MyFile) {
    aResult[idx++].res = true;
    aResult[idx++].res = (MyFile.write(wtext, BUFFERSIZE) == BUFFERSIZE);
    MyFile.close();
  } else {
    idx += 2;
  }

  /* Test read(buf, len) method */
  MyFile = SD.open("ARDUINO/SD/WRITE.txt");
  if (MyFile) {
    aResult[idx++].res = true;
    bytesread = MyFile.read(rtext, MyFile.size());
    aResult[idx++].res = (MyFile.size() == bytesread);
    MyFile.close();
  } else {
    idx += 2;
  }

  /* Test readBytes(buf, len) method */
  MyFile = SD.open("ARDUINO/SD/WRITE.txt");
  if (MyFile) {
    aResult[idx++].res = true;
    bytesread = MyFile.readBytes(rtext, MyFile.size());
    aResult[idx++].res = (MyFile.size() == bytesread);
    MyFile.close();
  } else {
    idx += 2;
  }
  aResult[idx++].res = SD.end();

  for (uint32_t i = 0; i < idx; i++) {
    Serial.printf("%s %s\n", aResult[i].tst, (aResult[i].res) ? "OK" : (aResult[i].err) ? aResult[i].err
                                                                                        : "KO");
  }

  Serial.println("###### End of the SD tests ######");
}

void loop() {
  // do nothing
}
