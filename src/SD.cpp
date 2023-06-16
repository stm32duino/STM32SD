/*

 SD - a slightly more friendly wrapper for sdfatlib

 This library aims to expose a subset of SD card functionality
 in the form of a higher level "wrapper" object.

 License: GNU General Public License V3
          (Because sdfatlib is licensed with this.)

 (C) Copyright 2010 SparkFun Electronics

 Modified by Frederic Pillon <frederic.pillon@st.com> for STMicroelectronics

 This library provides four key benefits:

   * Including `STM32SD.h` automatically creates a global
     `SD` object which can be interacted with in a similar
     manner to other standard global objects like `Serial` and `Ethernet`.

   * Boilerplate initialisation code is contained in one method named
     `begin` and no further objects need to be created in order to access
     the SD card.

   * Calls to `open` can supply a full path name including parent
     directories which simplifies interacting with files in subdirectories.

   * Utility methods are provided to determine whether a file exists
     and to create a directory hierarchy.

 */

/*

  Implementation Notes

  In order to handle multi-directory path traversal, functionality that
  requires this ability is implemented as callback functions.

  Individual methods call the `walkPath` function which performs the actual
  directory traversal (swapping between two different directory/file handles
  along the way) and at each level calls the supplied callback function.

  Some types of functionality will take an action at each level (e.g. exists
  or make directory) which others will only take an action at the bottom
  level (e.g. open).

 */

extern "C" {
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "stm32_def.h"
}
#include "STM32SD.h"
SDClass SD;

/**
  * @brief  Link SD, register the file system object to the FatFs mode and configure
  *         relatives SD IOs including SD Detect Pin if any
  * @param  data0: data0 pin number (default SDX_D0)
  * @param  data1: data1 pin number (default SDX_D1)
  * @param  data2: data2 pin number (default SDX_D2)
  * @param  data3: data3 pin number (default SDX_D3)
  * @param  ck: ck pin number (default SDX_CK)
  * @param  cmd: cmd pin number (default SDX_CMD)
  * @param  ckin: ckin pin number only for SDMMC (default SDX_CKIN)
  * @param  cdir: cdir pin number only for SDMMC (default SDX_CDIR)
  * @param  d0dir: d0dir pin number only for SDMMC (default SDX_D0DIR)
  * @param  d123dir: d123dir pin number only for SDMMC (default SDX_D123DIR)
  * @retval true or false
  */
#if defined(SDMMC1) || defined(SDMMC2)
bool SDClass::begin(uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3,
                    uint32_t ck, uint32_t cmd, uint32_t ckin, uint32_t cdir,
                    uint32_t d0dir, uint32_t d123dir)
{
  return begin(SD_DETECT_NONE, data0, data1, data2, data3, ck, cmd, ckin, cdir, d0dir, d123dir);
}

#else
bool SDClass::begin(uint32_t data0, uint32_t data1, uint32_t data2,
                    uint32_t data3, uint32_t ck, uint32_t cmd)
{
  return begin(SD_DETECT_NONE, data0, data1, data2, data3, ck, cmd);
}
#endif

/**
  * @brief  Link SD, register the file system object to the FatFs mode and configure
  *         relatives SD IOs including SD Detect Pin if any
  * @param  detect: detect pin number (default SD_DETECT_NONE)
  * @param  data0: data0 pin number (default SDX_D0)
  * @param  data1: data1 pin number (default SDX_D1)
  * @param  data2: data2 pin number (default SDX_D2)
  * @param  data3: data3 pin number (default SDX_D3)
  * @param  ck: ck pin number (default SDX_CK)
  * @param  cmd: cmd pin number (default SDX_CMD)
  * @param  ckin: ckin pin number only for SDMMC (default SDX_CKIN)
  * @param  cdir: cdir pin number only for SDMMC (default SDX_CDIR)
  * @param  d0dir: d0dir pin number only for SDMMC (default SDX_D0DIR)
  * @param  d123dir: d123dir pin number only for SDMMC (default SDX_D123DIR)
  * @retval true or false
  */
#if defined(SDMMC1) || defined(SDMMC2)
bool SDClass::begin(uint32_t detect, uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3,
                    uint32_t ck, uint32_t cmd, uint32_t ckin, uint32_t cdir, uint32_t d0dir, uint32_t d123dir)
#else
bool SDClass::begin(uint32_t detect, uint32_t data0, uint32_t data1, uint32_t data2,
                    uint32_t data3, uint32_t ck, uint32_t cmd)
#endif
{
  setDx(data0, data1, data2, data3);
  setCK(ck);
  setCMD(cmd);
#if defined(SDMMC1) || defined(SDMMC2)
  setCKIN(ckin);
  setCDIR(cdir);
  setDxDIR(d0dir, d123dir);
#endif
  /*##-1- Initializes SD IOs #############################################*/
  if (_card.init(detect)) {
    return _fatFs.init();
  }
  return false;
}

/**
  * @brief  Check if a file or folder exist on the SD disk
  * @param  filename: File name
  * @retval true or false
  */
bool SDClass::exists(const char *filepath)
{
  FILINFO fno;

  if (f_stat(filepath, &fno) != FR_OK) {
    return false;
  } else {
    return true;
  }
}

/**
  * @brief  Create directory on the SD disk
  * @param  filename: File name
  * @retval true if created or existing else false
  */
bool SDClass::mkdir(const char *filepath)
{
  FRESULT res = f_mkdir(filepath);
  if ((res != FR_OK) && (res != FR_EXIST)) {
    return false;
  } else {
    return true;
  }
}

/**
  * @brief  Remove directory on the SD disk
  * @param  filename: File name
  * @retval true or false
  */
bool SDClass::rmdir(const char *filepath)
{
  if (f_unlink(filepath) != FR_OK) {
    return false;
  } else {
    return true;
  }
}

/**
  * @brief  Open a file on the SD disk, if not existing it's created
  * @param  filename: File name
  * @param  mode: the mode in which to open the file
  * @retval File object referring to the opened file
  */
File SDClass::open(const char *filepath, uint8_t mode /* = FA_READ */)
{
  File file = File();

  file._name = (char *)malloc(strlen(filepath) + 1);
  if (file._name == NULL) {
    Error_Handler();
  }
  sprintf(file._name, "%s", filepath);

  file._fil = (FIL *)malloc(sizeof(FIL));
  if (file._fil == NULL) {
    Error_Handler();
  }

#if _FATFS == 68300
  file._fil->obj.fs = 0;
  file._dir.obj.fs = 0;
#else
  file._fil->fs = 0;
  file._dir.fs = 0;
#endif

  if ((mode == FILE_WRITE) && (!SD.exists(filepath))) {
    mode = mode | FA_CREATE_ALWAYS;
  }

  file._res = f_open(file._fil, filepath, mode);
  if (file._res != FR_OK) {
    free(file._fil);
    file._fil = NULL;
    file._res = f_opendir(&file._dir, filepath);
    if (file._res != FR_OK) {
      free(file._name);
      file._name = NULL;
    }
  }
  return file;
}

/**
  * @brief  Remove a file on the SD disk
  * @param  filename: File name
  * @retval true or false
  */
bool SDClass::remove(const char *filepath)
{
  if (f_unlink(filepath) != FR_OK) {
    return false;
  } else {
    return true;
  }
}

File SDClass::openRoot(void)
{
  return open(_fatFs.getRoot());
}

File::File(FRESULT result /* = FR_OK */)
{
  _name = NULL;
  _fil = NULL;
  _res = result;
}

/** List directory contents to Serial.
 *
 * \param[in] flags The inclusive OR of
 *
 * LS_DATE - %Print file modification date
 *
 * LS_SIZE - %Print file size.
 *
 * LS_R - Recursive list of subdirectories.
 *
 * \param[in] indent Amount of space before file name. Used for recursive
 * list to indicate subdirectory level.
 */
void File::ls(uint8_t flags, uint8_t indent)
{
  FRESULT res = FR_OK;
  FILINFO fno;
  char *fn;

#if _USE_LFN
#if _FATFS == 68300
  /* altname */
#else
  static char lfn[_MAX_LFN];
  fno.lfname = lfn;
  fno.lfsize = sizeof(lfn);
#endif
#endif

  while (1) {
    res = f_readdir(&_dir, &fno);
    if (res != FR_OK || fno.fname[0] == 0) {
      break;
    }
    if (fno.fname[0] == '.') {
      continue;
    }
#if _USE_LFN && _FATFS != 68300
    fn = *fno.lfname ? fno.lfname : fno.fname;
#else
    fn = fno.fname;
#endif
    //print any indent spaces
    for (int8_t i = 0; i < indent; i++) {
      Serial.print(' ');
    }
    Serial.print(fn);

    if ((fno.fattrib & AM_DIR) == 0) {
      // print modify date/time if requested
      if (flags & LS_DATE) {
        Serial.print(' ');
        printFatDate(fno.fdate);
        Serial.print(' ');
        printFatTime(fno.ftime);
      }
      // print size if requested
      if (flags & LS_SIZE) {
        Serial.print(' ');
        Serial.print(fno.fsize);
      }
      Serial.println();
    } else {
      // list subdirectory content if requested
      if (flags & LS_R) {
        char *fullPath;
        fullPath = (char *)malloc(strlen(_name) + 1 + strlen(fn) + 1);
        if (fullPath != NULL) {
          sprintf(fullPath, "%s/%s", _name, fn);
          File filtmp = SD.open(fullPath);

          if (filtmp) {
            Serial.println();
            filtmp.ls(flags, indent + 2);
            filtmp.close();
          } else {
            Serial.println(fn);
            Serial.print("Error to open dir: ");
            Serial.println(fn);
          }
          free(fullPath);
        } else {
          Serial.println();
          Serial.print("Error to allocate memory!");
        }
      }
    }
  }
}
//------------------------------------------------------------------------------
/** %Print a directory date field to Serial.
 *
 *  Format is yyyy-mm-dd.
 *
 * \param[in] fatDate The date field from a directory entry.
 */
void File::printFatDate(uint16_t fatDate)
{
  Serial.print(FAT_YEAR(fatDate));
  Serial.print('-');
  printTwoDigits(FAT_MONTH(fatDate));
  Serial.print('-');
  printTwoDigits(FAT_DAY(fatDate));
}
//------------------------------------------------------------------------------
/** %Print a directory time field to Serial.
 *
 * Format is hh:mm:ss.
 *
 * \param[in] fatTime The time field from a directory entry.
 */
void File::printFatTime(uint16_t fatTime)
{
  printTwoDigits(FAT_HOUR(fatTime));
  Serial.print(':');
  printTwoDigits(FAT_MINUTE(fatTime));
  Serial.print(':');
  printTwoDigits(FAT_SECOND(fatTime));
}
//------------------------------------------------------------------------------
/** %Print a value as two digits to Serial.
 *
 * \param[in] v Value to be printed, 0 <= \a v <= 99
 */
void File::printTwoDigits(uint8_t v)
{
  char str[3];
  str[0] = '0' + v / 10;
  str[1] = '0' + v % 10;
  str[2] = 0;
  Serial.print(str);
}

/**
  * @brief  Read byte from the file
  * @retval Byte read
  */
int File::read()
{
  UINT byteread;
  int8_t data;
  if (f_read(_fil, (void *)&data, 1, (UINT *)&byteread) == FR_OK) {
    return data;
  }
  return -1;
}

/**
  * @brief  Read an amount of data from the file
  * @param  buf: an array to store the read data from the file
  * @param  len: the number of elements to read
  * @retval Number of bytes read
  */
int File::read(void *buf, size_t len)
{
  UINT bytesread;

  if (f_read(_fil, buf, len, (UINT *)&bytesread) == FR_OK) {
    return bytesread;
  }
  return -1;
}

/**
  * @brief  Close a file on the SD disk
  * @param  None
  * @retval None
  */
void File::close()
{
  if (_name) {
#if _FATFS == 68300
    if (_fil) {
      if (_fil->obj.fs != 0) {
#else
    if (_fil) {
      if (_fil->fs != 0) {
#endif
        /* Flush the file before close */
        f_sync(_fil);

        /* Close the file */
        f_close(_fil);
      }
      free(_fil);
      _fil = NULL;
    }

#if _FATFS == 68300
    if (_dir.obj.fs != 0) {
#else
    if (_dir.fs != 0) {
#endif
      f_closedir(&_dir);
    }

    free(_name);
    _name = NULL;
  }
}


/**
  * @brief  Ensures that any bytes written to the file are physically saved to the SD card
  * @param  None
  * @retval None
  */
void File::flush()
{
  f_sync(_fil);
}

/**
  * @brief  Read a byte from the file without advancing to the next one
  * @param  None
  * @retval read byte
  */
int File::peek()
{
  int data;
  data = read();
  seek(position() - 1);
  return data;
}

/**
  * @brief  Get the current position within the file
  * @param  None
  * @retval position within file
  */
uint32_t File::position()
{
  uint32_t filepos = 0;
  filepos = f_tell(_fil);
  return filepos;
}

/**
  * @brief  Seek to a new position in the file
  * @param  pos: The position to which to seek
  * @retval true or false
  */
bool File::seek(uint32_t pos)
{
  if (pos > size()) {
    return false;
  } else {
    if (f_lseek(_fil, pos) != FR_OK) {
      return false;
    } else {
      return true;
    }
  }
}

/**
  * @brief  Get the size of the file
  * @param  None
  * @retval file's size
  */
uint32_t File::size()
{
  uint32_t file_size = 0;

  file_size = f_size(_fil);
  return (file_size);
}

File::operator bool()
{
#if _FATFS == 68300
  return !((_name == NULL) || ((_fil == NULL) && (_dir.obj.fs == 0)) || ((_fil != NULL) && (_fil->obj.fs == 0) && (_dir.obj.fs == 0)));
#else
  return !((_name == NULL) || ((_fil == NULL) && (_dir.fs == 0)) || ((_fil != NULL) && (_fil->fs == 0) && (_dir.fs == 0)));
#endif
}
/**
  * @brief  Write data to the file
  * @param  data: Data to write to the file
  * @retval Number of data written (1)
  */
size_t File::write(uint8_t data)
{
  return write(&data, 1);
}

/**
  * @brief  Write an array of data to the file
  * @param  buf: an array of characters or bytes to write to the file
  * @param  len: the number of elements in buf
  * @retval Number of data written
  */
size_t File::write(const char *buf, size_t size)
{
  size_t byteswritten;
  f_write(_fil, (const void *)buf, size, (UINT *)&byteswritten);
  return byteswritten;
}

size_t File::write(const uint8_t *buf, size_t size)
{
  return write((const char *)buf, size);
}

/**
  * @brief  Print data to the file
  * @param  data: Data to write to the file
  * @retval Number of data written (1)
  */
size_t File::print(const char *data)
{
  return write(data, strlen(data));
}

/**
  * @brief  Print data to the file
  * @retval Number of data written (1)
  */
size_t File::println()
{
  return write("\r\n", 2);
}

/**
  * @brief  Print data to the file
  * @param  data: Data to write to the file
  * @retval Number of data written (1)
  */
size_t File::println(const char *data)
{
  size_t bytewritten = write(data, strlen(data));
  bytewritten += println();
  return bytewritten;
}

/**
  * @brief  Print data to the file
  * @param  data: Data of type String to write to the file
  * @retval Number of data written (1)
  */
size_t File::println(String &data)
{
  return println(data.c_str());
}

/**
  * @brief  Check if there are any bytes available for reading from the file
  * @retval Number of bytes available
  */
int File::available()
{
  uint32_t n = size() - position();
  return n > 0x7FFF ? 0x7FFF : n;
}


char *File::name()
{
  char *name = strrchr(_name, '/');
  if (name && name[0] == '/') {
    name++;
  }
  return name;
}

/**
  * @brief  Check if the file is directory or normal file
  * @retval TRUE if directory else FALSE
  */
bool File::isDirectory()
{
  FILINFO fno;
  if (_name == NULL) {
    Error_Handler();
  }
#if _FATFS == 68300
  if (_dir.obj.fs != 0)
#else
  if (_dir.fs != 0)
#endif
    return true;
#if _FATFS == 68300
  else if (_fil->obj.fs != 0)
#else
  else if (_fil->fs != 0)
#endif
    return false;
  // if not init get info
  if (f_stat(_name, &fno) == FR_OK) {
    if (fno.fattrib & AM_DIR) {
      return true;
    }
  }
  // Assume not a directory
  return false;
}

File File::openNextFile(uint8_t mode)
{
  FRESULT res = FR_OK;
  FILINFO fno;
  char *fn;
#if _USE_LFN && _FATFS != 68300
  static char lfn[_MAX_LFN];
  fno.lfname = lfn;
  fno.lfsize = sizeof(lfn);
#endif
  while (1) {
    res = f_readdir(&_dir, &fno);
    if (res != FR_OK || fno.fname[0] == 0) {
      return File(res);
    }
    if (fno.fname[0] == '.') {
      continue;
    }
#if _USE_LFN && _FATFS != 68300
    fn = *fno.lfname ? fno.lfname : fno.fname;
#else
    fn = fno.fname;
#endif
    size_t name_len = strlen(_name);
    char *fullPath = (char *)malloc(name_len + strlen(fn) + 2);
    if (fullPath != NULL) {
      // Avoid twice '/'
      if ((name_len > 0)  && (_name[name_len - 1] == '/')) {
        sprintf(fullPath, "%s%s", _name, fn);
      } else {
        sprintf(fullPath, "%s/%s", _name, fn);
      }
      File filtmp = SD.open(fullPath, mode);
      free(fullPath);
      return filtmp;
    } else {
      return File(FR_NOT_ENOUGH_CORE);
    }
  }
}

void File::rewindDirectory(void)
{
  if (isDirectory()) {
#if _FATFS == 68300
    if (_dir.obj.fs != 0) {
#else
    if (_dir.fs != 0) {
#endif
      f_closedir(&_dir);
    }
    f_opendir(&_dir, _name);
  }
}

