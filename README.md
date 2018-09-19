# STM32 SD library for Arduino

With an STM32 board with SD card slot availability, this library enables
reading and writing on SD card using SD card slot of a STM32 board (NUCLEO, DISCOVERY, ...).

This library follow Arduino API.

For more information about it, please visit:
http://www.arduino.cc/en/Reference/SD

## Dependency

This library is based on FatFs, a generic FAT file system module for small embedded systems.  
[http://elm-chan.org/fsw/ff](http://elm-chan.org/fsw/ff/00index_e.html)

The FatFs has been ported as Arduino library [here](https://github.com/stm32duino/FatFs).
The STM32SD library depends on it.

## Configuration

### FatFs
The FatFs has several user defined options, which is specified from within the `ffconf.h` file.

This library provides a default user defined options file named `ffconf_default.h`.

User can provide his own defined options by adding his configuration in a file named
`ffconf_custom.h` at sketch level or in variant folder.

### SD detect and timeout
* `SD_DETECT_PIN` pin number can be defined in `variant.h` or using `build_opt.h`.

* `SD_DATATIMEOUT` constant for Read/Write block could be redefined in `variant.h` or using `build_opt.h`
