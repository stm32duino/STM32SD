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
#include "ffconf_default.h"
#endif
#endif /* _ARDUINO_FFCONF_H */
