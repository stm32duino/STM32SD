/*
 * @file    ffconf.h
 * @brief   Include header file to match Arduino library format
 */
#ifndef _ARDUINO_FFCONF_H
#define _ARDUINO_FFCONF_H

#include "stm32_def.h"
#include "bsp_sd.h"

/* FatFs specific configuration options. */
#if __has_include("ffconf_custom.h")
  #include "ffconf_custom.h"
#else
  #if defined(FF_DEFINED) && !defined(_FATFS)
    #define _FATFS FF_DEFINED
  #endif

  #if _FATFS == 80286
    /* Ensure backward compatibility with release prior to FatFs 0.14 */
    /* Those flags are */
    #define _USE_WRITE 1
    #define _USE_IOCTL 1
    #include "ffconf_default_80286.h"
  #elif _FATFS == 68300
    #include "ffconf_default_68300.h"
  #else
    #include "ffconf_default_32020.h"
  #endif
#endif
#endif /* _ARDUINO_FFCONF_H */
