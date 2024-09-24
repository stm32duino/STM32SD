/**
  ******************************************************************************
  * @file    Sd2Card.h
  * @author  Frederic Pillon <frederic.pillon@st.com> for STMicroelectronics
  * @date    2017
  * @brief
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#ifndef Sd2Card_h
#define Sd2Card_h

#include "bsp_sd.h"

// card types to match Arduino definition
#define SD_CARD_TYPE_UKN      0
/** Standard capacity V1 SD card */
#define SD_CARD_TYPE_SD1      1
/** Standard capacity V2 SD card */
#define SD_CARD_TYPE_SD2      2
/** High Capacity SD card */
#define SD_CARD_TYPE_SDHC     3
/** High Capacity SD card */
#define SD_CARD_TYPE_SECURED  4

class Sd2Card {
  public:
    Sd2Card();

    bool init(uint32_t detect = SD_DETECT_NONE, uint32_t level = SD_DETECT_LEVEL);
    bool deinit(void);

    // set* have to be called before init()
    void setDx(uint32_t data0, uint32_t data1 = PNUM_NOT_DEFINED, uint32_t data2 = PNUM_NOT_DEFINED, uint32_t data3 = PNUM_NOT_DEFINED)
    {
      SD_PinNames.pin_d0 = digitalPinToPinName(data0);
      SD_PinNames.pin_d1 = digitalPinToPinName(data1);
      SD_PinNames.pin_d2 = digitalPinToPinName(data2);
      SD_PinNames.pin_d3 = digitalPinToPinName(data3);
    };
    void setCK(uint32_t ck)
    {
      SD_PinNames.pin_ck = digitalPinToPinName(ck);
    };
    void setCMD(uint32_t cmd)
    {
      SD_PinNames.pin_cmd = digitalPinToPinName(cmd);
    };

    void setDx(PinName data0, PinName data1 = NC, PinName data2 = NC, PinName data3 = NC)
    {
      SD_PinNames.pin_d0 = data0;
      SD_PinNames.pin_d1 = data1;
      SD_PinNames.pin_d2 = data2;
      SD_PinNames.pin_d3 = data3;
    };
    void setCK(PinName ck)
    {
      SD_PinNames.pin_ck = ck;
    };
    void setCMD(PinName cmd)
    {
      SD_PinNames.pin_cmd = cmd;
    };
#if defined(SDMMC1) || defined(SDMMC2)
    void setCKIN(uint32_t ckin)
    {
      SD_PinNames.pin_ckin = digitalPinToPinName(ckin);
    };
    void setCDIR(uint32_t cdir)
    {
      SD_PinNames.pin_cdir = digitalPinToPinName(cdir);
    };
    void setDxDIR(uint32_t d0dir, uint32_t d123dir)
    {
      SD_PinNames.pin_d0dir = digitalPinToPinName(d0dir);
      SD_PinNames.pin_d123dir = digitalPinToPinName(d123dir);
    };

    void setCKIN(PinName ckin)
    {
      SD_PinNames.pin_ckin = ckin;
    };
    void setCDIR(PinName cdir)
    {
      SD_PinNames.pin_cdir = cdir;
    };
    void setDxDIR(PinName d0dir, PinName d123dir)
    {
      SD_PinNames.pin_d0dir = d0dir;
      SD_PinNames.pin_d123dir = d123dir;
    };
#endif
    /** Return the card type: SD V1, SD V2 or SDHC */
    uint8_t type(void) const;

  private:
    BSP_SD_CardInfo _SdCardInfo;

};
#endif  // sd2Card_h
