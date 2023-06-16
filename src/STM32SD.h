/*

 SD - a slightly more friendly wrapper for sdfatlib

 This library aims to expose a subset of SD card functionality
 in the form of a higher level "wrapper" object.

 License: GNU General Public License V3
          (Because sdfatlib is licensed with this.)

 (C) Copyright 2010 SparkFun Electronics

 Modified by Frederic Pillon <frederic.pillon@st.com> for STMicroelectronics

 */

#ifndef __SD_H__
#define __SD_H__

#include <Arduino.h>

#include "Sd2Card.h"
#include "SdFatFs.h"

// flags for ls()
/** ls() flag to print modify date */
uint8_t const LS_DATE = 1;
/** ls() flag to print file size */
uint8_t const LS_SIZE = 2;
/** ls() flag for recursive list of subdirectories */
uint8_t const LS_R = 4;

class File {
  public:
    File(FRESULT res = FR_OK);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual size_t write(const char *buf, size_t size);

    virtual int read();
    virtual int peek();
    virtual int available();
    virtual void flush();
    int read(void *buf, size_t len);
    bool seek(uint32_t pos);
    uint32_t position();
    uint32_t size();
    void close();
    operator bool();

    char *name(void);
    char *fullname(void)
    {
      return _name;
    };
    bool isDirectory();
    File openNextFile(uint8_t mode = FILE_READ);
    void rewindDirectory(void);

    virtual size_t print(const char *data);
    virtual size_t println();
    virtual size_t println(const char *data);
    virtual size_t println(String &data);

    // Print to Serial line
    void ls(uint8_t flags, uint8_t indent = 0);
    static void printFatDate(uint16_t fatDate);
    static void printFatTime(uint16_t fatTime);
    static void printTwoDigits(uint8_t v);


    char *_name = NULL; //file or dir name
    FIL *_fil = NULL; // underlying file object structure pointer
    DIR _dir = {}; // init all fields to 0
    FRESULT _res = FR_OK;

    FRESULT getErrorstate(void)
    {
      return _res;
    }

};

class SDClass {

  public:
    /* Initialize the SD peripheral */
#if defined(SDMMC1) || defined(SDMMC2)
    bool begin(uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3, uint32_t ck, uint32_t cmd,
               uint32_t ckin,  uint32_t cdir, uint32_t d0dir, uint32_t d123dir
              );
    bool begin(uint32_t detect = SD_DETECT_NONE, uint32_t data0 = SDX_D0, uint32_t data1 = SDX_D1,
               uint32_t data2 = SDX_D2, uint32_t data3 = SDX_D3, uint32_t ck = SDX_CK, uint32_t cmd = SDX_CMD,
               uint32_t ckin = SDX_CKIN, uint32_t cdir = SDX_CDIR, uint32_t d0dir = SDX_D0DIR, uint32_t d123dir = SDX_D123DIR
              );
#else
    bool begin(uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3, uint32_t ck, uint32_t cmd);
    bool begin(uint32_t detect = SD_DETECT_NONE, uint32_t data0 = SDX_D0, uint32_t data1 = SDX_D1,
               uint32_t data2 = SDX_D2, uint32_t data3 = SDX_D3, uint32_t ck = SDX_CMD, uint32_t cmd = SDX_CMD
              );
#endif

    // set* have to be called before begin()
    void setDx(uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3)
    {
      _card.setDx(data0, data1, data2, data3);
    };
    void setCK(uint32_t ck)
    {
      _card.setCK(ck);
    };
    void setCMD(uint32_t cmd)
    {
      _card.setCMD(cmd);
    };

    void setDx(PinName data0, PinName data1, PinName data2, PinName data3)
    {
      _card.setDx(data0, data1, data2, data3);
    };
    void setCK(PinName ck)
    {
      _card.setCK(ck);
    };
    void setCMD(PinName cmd)
    {
      _card.setCMD(cmd);
    };
#if defined(SDMMC1) || defined(SDMMC2)
    void setCKIN(uint32_t ckin)
    {
      _card.setCKIN(ckin);
    };
    void setCDIR(uint32_t cdir)
    {
      _card.setCDIR(cdir);
    };
    void setDxDIR(uint32_t d0dir, uint32_t d123dir)
    {
      _card.setDxDIR(d0dir, d123dir);
    };

    void setCKIN(PinName ckin)
    {
      _card.setCKIN(ckin);
    };
    void setCDIR(PinName cdir)
    {
      _card.setCDIR(cdir);
    };
    void setDxDIR(PinName d0dir, PinName d123dir)
    {
      _card.setDxDIR(d0dir, d123dir);
    };
#endif
    static File open(const char *filepath, uint8_t mode = FA_READ);
    static bool exists(const char *filepath);
    static bool mkdir(const char *filepath);
    static bool remove(const char *filepath);
    static bool rmdir(const char *filepath);

    File openRoot(void);

    friend class File;

  private:
    Sd2Card _card;
    SdFatFs _fatFs;

};

extern SDClass SD;

#endif
