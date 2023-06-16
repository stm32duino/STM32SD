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
 * board `variant*.h`
 * `build_opt.h`: see [Customize build options](https://github.com/stm32duino/Arduino_Core_STM32/wiki/Customize-build-options-using-build_opt.h)

 * `hal_conf_extra.h`: see [HAL configuration](https://github.com/stm32duino/Arduino_Core_STM32/wiki/HAL-configuration)

#### SDIO/SDMMC pins definition

Since STM32 core v2.6.0, the `PinMap_SD[]` array defined in the `PeripheralPins*.c` has been split per signals:
```C
PinMap_SD_CK[]
PinMap_SD_DATA0[]
PinMap_SD_DATA1[]
PinMap_SD_DATA2[]
PinMap_SD_DATA3[]
PinMap_SD_DATA4[]
PinMap_SD_DATA5[]
PinMap_SD_DATA6[]
PinMap_SD_DATA7[]
/* Only for SDMMC */
PinMap_SD_CKIN[]
PinMap_SD_CDIR[]
PinMap_SD_D0DIR[]
PinMap_SD_D123DIR[]
```

This allows to define the SDIO/SDMMC pins to use instead of using all the pins defined in the old array.
By default, if no pins are explicitly defined, the first one from each array is used.

* To redefine the default one, use the following macros:

  * `SDX_D0`
  * `SDX_D1`
  * `SDX_D2`
  * `SDX_D3`
  * `SDX_CMD`
  * `SDX_CK`

  * Only for `SDMMC`:
    * `SDX_CKIN`
    * `SDX_CDIR`
    * `SDX_D0DIR`
    * `SDX_D123DIR`

* or redefine the default one before call of `begin()` of `SDClass` or `init()` of `Sd2Card`, use the following methods:

  * `setDx(uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3)`
  * `setCK(uint32_t ck)`
  * `setCK(PinName ck)`
  * `setCMD(uint32_t cmd)`
  * `setCMD(PinName cmd)`

  * Only for `SDMMC`:
    * `setCKIN(uint32_t ckin)`
    * `setCKIN(PinName ckin)`
    * `setCDIR(uint32_t cdir)`
    * `setCDIR(PinName cdir)`
    * `setDxDIR(uint32_t d0dir, uint32_t d123dir)`
    * `setDxDIR(PinName d0dir, PinName d123dir)`

  *Code snippet:*
```C++
  Sd2Card card;
  card.setDx(PE12, PE13, PE14, PE15);
  card.setCMD(PB_14); // using PinName
  card.setCK(PB15);
  card.init();
```
  or
```C++
  SD.setDx(PE12, PE13, PE14, PE15);
  SD.setCMD(PB14);
  SD.setCK(PB_15); // using PinName
  SD.begin();
```

* or using the `begin()` of `SDClass` or `init()` of `Sd2Card` methods:

  * For `SDIO`:
    * `SDClass`:
      * `begin(uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3, uint32_t ck, uint32_t cmd)`
      * `begin(uint32_t detect = SD_DETECT_NONE, uint32_t data0 = SDX_D0, uint32_t data1 = SDX_D1, uint32_t data2 = SDX_D2, uint32_t data3 = SDX_D3, uint32_t ck = SDX_CK, uint32_t cmd = SDX_CMD, uint32_t ckin = SDX_CKIN, uint32_t cdir = SDX_CDIR, uint32_t d0dir = SDX_D0DIR, uint32_t d123dir = SDX_D123DIR);`
    * `Sd2Card`:
      * `init(uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3, uint32_t ck, uint32_t cmd)`
      * `init(uint32_t detect = SD_DETECT_NONE, uint32_t data0 = SDX_D0, uint32_t data1 = SDX_D1, uint32_t data2 = SDX_D2, uint32_t data3 = SDX_D3, uint32_t ck = SDX_CK, uint32_t cmd = SDX_CMD);`

  * For `SDMMC`:
    * `SDClass`:
      * `begin(uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3, uint32_t ck, uint32_t cmd, uint32_t ckin,  uint32_t cdir, uint32_t d0dir, uint32_t d123dir);`
      * `begin(uint32_t detect = SD_DETECT_NONE, uint32_t data0 = SDX_D0, uint32_t data1 = SDX_D1, uint32_t data2 = SDX_D2, uint32_t data3 = SDX_D3, uint32_t ck = SDX_CK, uint32_t cmd = SDX_CMD, uint32_t ckin = SDX_CKIN, uint32_t cdir = SDX_CDIR, uint32_t d0dir = SDX_D0DIR, uint32_t d123dir = SDX_D123DIR);`
      *
    * * `Sd2Card`:
      * `init(uint32_t data0 = SDX_D0, uint32_t data1 = SDX_D1, uint32_t data2 = SDX_D2, uint32_t data3 = SDX_D3, uint32_t ck = SDX_CK, uint32_t cmd = SDX_CMD, uint32_t ckin = SDX_CKIN, uint32_t cdir = SDX_CDIR, uint32_t d0dir = SDX_D0DIR, uint32_t d123dir = SDX_D123DIR)`
      * `init(uint32_t detect = SD_DETECT_NONE, uint32_t data0 = SDX_D0, uint32_t data1 = SDX_D1, uint32_t data2 = SDX_D2, uint32_t data3 = SDX_D3, uint32_t ck = SDX_CK, uint32_t cmd = SDX_CMD, uint32_t ckin = SDX_CKIN, uint32_t cdir = SDX_CDIR, uint32_t d0dir = SDX_D0DIR, uint32_t d123dir = SDX_D123DIR);`

  *Code snippet:*
```C++
  card.init(PE12, PE13, PE14, PE15, PB14, PB15);
  SD.begin(SD_DETECT_PIN, PE12, PE13, PE14, PE15, PB14, PB15);
```

#### SD configurations

* `SD_INSTANCE`: some STM32 can have 2 SD peripherals `SDMMC1` and `SDMMC2`, note that this library can managed only one peripheral
  * `SDIO` or `SDMMC1` (default)
  * `SDMMC2`

* `SD_HW_FLOW_CTRL`: specifies whether the SDMMC hardware flow control is enabled or disabled
  * `SD_HW_FLOW_CTRL_ENABLE`
  * `SD_HW_FLOW_CTRL_DISABLE` (default)

* `SD_BUS_WIDE`: specifies the SDMMC bus width
  * `SD_BUS_WIDE_1B`
  * `SD_BUS_WIDE_4B` (default)
  * `SD_BUS_WIDE_8B`

* `SD_CLK_DIV`: specifies the clock frequency of the SDMMC controller (0-255)
  * `SDIO_TRANSFER_CLK_DIV` (default) for `SDIO`
  * `SDMMC_TRANSFER_CLK_DIV` or `SDMMC_NSpeed_CLK_DIV` (default) for `SDMMC`

#### SD Transceiver

* To specifies whether external Transceiver is present and enabled (Available only on some STM32) add:

  `#define USE_SD_TRANSCEIVER  1`
* Then define pins:
  * `SD_TRANSCEIVER_EN` pin number to enable the level shifter
  * `SD_TRANSCEIVER_SEL` pin number for voltage selection

#### SD detect and timeout
* `SD_DETECT_PIN` pin number

* `SD_DATATIMEOUT` constant for Read/Write block
