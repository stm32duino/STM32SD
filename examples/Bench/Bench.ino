#include <STM32SD.h>

#define TEST_FILE_NAME "/bench.txt"
#define TEST_BLOCK_SIZE 4096  // Size of each data block in bytes
#define TEST_BLOCK_COUNT 400  // Number of blocks to write and read

File testFile;

void setup() {
  delay(2000);

  Serial.begin(115200);
  while (!Serial) {
    delay(10);  // Wait for Serial to initialize
  }

  Serial.println("Initializing SD card...");
  if (!SD.begin()) {
    Serial.println("SD initialization failed!");
    while (1)
      ;
  }
  Serial.println("SD initialization successful.");
  Serial.flush();

  // Perform write speed test
  if (writeSpeedTest()) {
    // Perform read speed test
    readSpeedTest();
  }
}

bool writeSpeedTest() {
  bool status = false;
  Serial.println("Starting write speed test...");
  Serial.flush();
  // Prepare test data
  uint8_t buffer[TEST_BLOCK_SIZE];
  for (uint32_t i = 0; i < TEST_BLOCK_SIZE; i++) {
    buffer[i] = i % 256;
  }

  // Open file for writing
  testFile = SD.open(TEST_FILE_NAME, FILE_WRITE);
  if (!testFile) {
    Serial.println("Failed to open file for writing.");
  } else {

    unsigned long startTime = micros();
    for (int i = 0; i < TEST_BLOCK_COUNT; i++) {
      testFile.write(buffer, TEST_BLOCK_SIZE);
    }
    testFile.close();

    float speed = (TEST_BLOCK_SIZE * TEST_BLOCK_COUNT) / ((micros() - startTime) / 1000000.0) / 1024.0;
    Serial.print("Write speed: ");
    Serial.print(speed, 2);  // Display speed with 2 decimal places
    Serial.println(" KB/s");
    Serial.flush();
    status = true;
  }
  return status;
}

void readSpeedTest() {
  Serial.println("Starting read speed test...");
  Serial.flush();
  // Open file for reading
  testFile = SD.open(TEST_FILE_NAME, FILE_READ);
  if (!testFile) {
    Serial.println("Failed to open file for reading.");
  } else {

    uint8_t buffer[TEST_BLOCK_SIZE];
    unsigned long startTime = micros();
    for (int i = 0; i < TEST_BLOCK_COUNT; i++) {
      testFile.read(buffer, TEST_BLOCK_SIZE);
    }
    testFile.close();

    float speed = (TEST_BLOCK_SIZE * TEST_BLOCK_COUNT) / ((micros() - startTime) / 1000000.0) / 1024.0;
    Serial.print("Read speed: ");
    Serial.print(speed, 2);  // Display speed with 2 decimal places
    Serial.println(" KB/s");
    Serial.flush();
  }
}

void loop() {
  // Do nothing in loop
}
