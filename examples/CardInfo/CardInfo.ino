/*
  SD card test

 This example shows how use the utility libraries on which the'
 SD library is based in order to get info about your SD card.
 Very useful for testing a card when you're not sure whether its working or not.

  * SD card attached

 */
// include the SD library:
#include <STM32SD.h>

// If SD card slot has no detect pin then define it as SD_DETECT_NONE
// to ignore it. One other option is to call 'card.init()' without parameter.
#ifndef SD_DETECT_PIN
#define SD_DETECT_PIN SD_DETECT_NONE
#endif

Sd2Card card;
SdFatFs fatFs;

void setup() {
  bool disp = false;
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  while (!Serial)
    ;
  Serial.print("\nInitializing SD card...");
  while (!card.init(SD_DETECT_PIN)) {
    if (!disp) {
      Serial.println("initialization failed. Is a card inserted?");
      disp = true;
    }
    delay(10);
  }

  Serial.println("A card is present.");

  // print the type of card
  Serial.print("\nCard type: ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!fatFs.init()) {
    Serial.println("Could not find FAT partition.\nMake sure you've formatted the card");
    return;
  }

  // print the type and size of the first FAT-type volume
  uint64_t volumesize;
  Serial.print("\nVolume type is ");
  uint8_t fatType = fatFs.fatType();
#if defined(FAT_TYPE_EXFAT)
  if (fatType == FAT_TYPE_EXFAT) {
    Serial.println("exFAT");
  } else
#endif
  {
    if (fatType != FAT_TYPE_UNK) {
      Serial.printf("FAT%u\n", fatFs.fatType());
    } else {
      Serial.println("unknown");
    }
  }

  volumesize = fatFs.blocksPerCluster();  // clusters are collections of blocks
  volumesize *= fatFs.clusterCount();     // we'll have a lot of clusters
  volumesize *= 512;                      // SD card blocks are always 512 bytes
  Serial.print("Volume size (bytes): ");
  Serial.println(volumesize);
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);


  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  File root = SD.openRoot();

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
  root.close();
  if (!fatFs.deinit()) {
    Serial.println("Failed to deinit card");
  }
  if (!card.deinit()) {
    Serial.println("Failed to deinit card");
  }

  Serial.println("###### End of the SD tests ######");
}

void loop(void) {
  // do nothing
}
