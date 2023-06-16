/**
  ******************************************************************************
  * @file    Sd2Card.cpp
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

#include <Arduino.h>
#include "Sd2Card.h"

#if defined(SDMMC1) || defined(SDMMC2)
bool Sd2Card::init(uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3, uint32_t ck, uint32_t cmd,
                   uint32_t ckin, uint32_t cdir, uint32_t d0dir, uint32_t d123dir)
{
  return init(SD_DETECT_NONE, data0, data1, data2, data3, ck, cmd, ckin, cdir, d0dir, d123dir);
}
#else
bool Sd2Card::init(uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3, uint32_t ck, uint32_t cmd)
{
  return init(SD_DETECT_NONE, data0, data1, data2, data3, ck, cmd);
}
#endif

#if defined(SDMMC1) || defined(SDMMC2)
bool Sd2Card::init(uint32_t detect, uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3,
                   uint32_t ck, uint32_t cmd, uint32_t ckin, uint32_t cdir, uint32_t d0dir, uint32_t d123dir)
#else
bool Sd2Card::init(uint32_t detect, uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3,
                   uint32_t ck, uint32_t cmd)
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
  if (detect != SD_DETECT_NONE) {
    PinName p = digitalPinToPinName(detect);
    if ((p == NC) || \
        BSP_SD_DetectPin(set_GPIO_Port_Clock(STM_PORT(p)),
                         STM_LL_GPIO_PIN(p)) != MSD_OK) {
      return false;
    }
  }
#if defined(USE_SD_TRANSCEIVER) && (USE_SD_TRANSCEIVER != 0U)
  PinName sd_en = digitalPinToPinName(SD_TRANSCEIVER_EN);
  PinName sd_sel = digitalPinToPinName(SD_TRANSCEIVER_SEL);
  if (BSP_SD_TransceiverPin(set_GPIO_Port_Clock(STM_PORT(sd_en)),
                            STM_LL_GPIO_PIN(sd_en),
                            set_GPIO_Port_Clock(STM_PORT(sd_sel)),
                            STM_LL_GPIO_PIN(sd_sel)) == MSD_ERROR) {
    return false;
  }
#endif
  if (BSP_SD_Init() == MSD_OK) {
    BSP_SD_GetCardInfo(&_SdCardInfo);
    return true;
  }
  return false;
}

uint8_t Sd2Card::type(void) const
{
  uint8_t cardType = SD_CARD_TYPE_UKN;
  switch (_SdCardInfo.CardType) {
    case CARD_SDSC:
      switch (_SdCardInfo.CardVersion) {
        case CARD_V1_X:
          cardType = SD_CARD_TYPE_SD1;
          break;
        case CARD_V2_X:
          cardType = SD_CARD_TYPE_SD2;
          break;
        default:
          cardType = SD_CARD_TYPE_UKN;
      }
      break;
    case CARD_SDHC_SDXC:
      cardType = SD_CARD_TYPE_SDHC;
      break;
    case CARD_SECURED:
      cardType = SD_CARD_TYPE_SECURED;
      break;
    default:
      cardType = SD_CARD_TYPE_UKN;
  }
  return cardType;
}

