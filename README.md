# STM32SD

## SD library for Arduino

With an STM32 board with SD card slot availability, this library enables
reading and writing on SD card using SD card slot of a STM32 board (NUCLEO, DISCOVERY, ...).

This library follow Arduino API.

For more information about it, please visit:
http://www.arduino.cc/en/Reference/SD

## Note

The library is based on FatFs, a generic FAT file system module for small embedded systems.  
[http://elm-chan.org/fsw/ff](http://elm-chan.org/fsw/ff/00index_e.html)

The FatFs has been ported as Arduino library [here](https://github.com/stm32duino/FatFs). The STM32SD library depends on it.
