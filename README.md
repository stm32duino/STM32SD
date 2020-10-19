# STM32 SD library for Arduino

With an STM32 board with SD card slot availability, this library enables
reading and writing on SD card using SD card slot of a STM32 board (NUCLEO, DISCOVERY, ...).
This library is for SD card slots connected to the SDIO-/SDMMC-hardware of the processor.
For slots connected to SPI-hardware use the standard Arduino SD library.

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

### SD

Some default definitions can be overridden using:
 * board `variant.h`
 * `build_opt.h`: see [Customize build options](https://github.com/stm32duino/wiki/wiki/Customize-build-options-using-build_opt.h)

 * `hal_conf_extra.h`: see [HAL configuration](https://github.com/stm32duino/wiki/wiki/HAL-configuration)


#### SD configurations

* `SD_INSTANCE`: some STM32 can have 2 SD peripherals `SDMMC1` and `SDMMC2`, note that this library can managed only one peripheral
  * `SDMMC1` (default)
  * `SDMMC2`

* `SD_HW_FLOW_CTRL`: specifies whether the SDMMC hardware flow control is enabled or disabled
  * `SD_HW_FLOW_CTRL_ENABLE`
  * `SD_HW_FLOW_CTRL_DISABLE` (default)

* `SD_BUS_WIDE`: specifies the SDMMC bus width
  * `SD_BUS_WIDE_1B`
  * `SD_BUS_WIDE_4B` (default)
  * `SD_BUS_WIDE_8B`

#### SD Transceiver

* `SD_TRANSCEIVER_MODE`: specifies whether external Transceiver is enabled or disabled. Available only on some STM32
  * `SDMMC_TRANSCEIVER_ENABLE`
  * `SDMMC_TRANSCEIVER_DISABLE` (default)

* `SD_TRANSCEIVER_EN` pin number to enable the level shifter
* `SD_TRANSCEIVER_SEL` pin number for voltage selection

#### SD detect and timeout
* `SD_DETECT_PIN` pin number

* `SD_DATATIMEOUT` constant for Read/Write block