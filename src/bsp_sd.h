/**
  ******************************************************************************
  * @file    sd_conf.h
  * @author  fpiSTM
  * @date    2017
  * @brief   This file contains the common defines and functions prototypes for
  *          the bdp_sd.c driver.
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_SD_H
#define __BSP_SD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "PinNames.h"
#include "stm32_def.h"
#include "variant.h"
#if !defined(STM32_CORE_VERSION) || (STM32_CORE_VERSION  <= 0x01060100)
#error "This library version required a STM32 core version > 1.6.1.\
Please update the core or install previous library version."
#endif

/* For backward compatibility */
#if defined(SD_TRANSCEIVER_MODE) && !defined(USE_SD_TRANSCEIVER)
#define USE_SD_TRANSCEIVER        1
#endif

/*SD Card information structure */

#define BSP_SD_CardInfo HAL_SD_CardInfoTypeDef
/* For backward compatibility */
#define SD_CardInfo BSP_SD_CardInfo
/*SD status structure definition */
#define MSD_OK                   ((uint8_t)0x00)
#define MSD_ERROR                ((uint8_t)0x01)
#define MSD_ERROR_SD_NOT_PRESENT ((uint8_t)0x02)

/* SD Exported Constants */
#define SD_PRESENT               ((uint8_t)0x01)
#define SD_NOT_PRESENT           ((uint8_t)0x00)
#define SD_DETECT_NONE           NUM_DIGITAL_PINS

/* Could be redefined in variant.h or using build_opt.h */
#ifndef SD_DATATIMEOUT
#define SD_DATATIMEOUT         100000000U
#endif

#if defined(USE_SD_TRANSCEIVER) && (USE_SD_TRANSCEIVER != 0U)
#ifndef SD_TRANSCEIVER_EN
#define SD_TRANSCEIVER_EN        NUM_DIGITAL_PINS
#endif

#ifndef SD_TRANSCEIVER_SEL
#define SD_TRANSCEIVER_SEL       NUM_DIGITAL_PINS
#endif
#endif

#ifndef GPIO_PIN_All
#define GPIO_PIN_All GPIO_PIN_ALL
#endif

/* Workaround while core does not defined *_NA for SDMMCx signals availability */
#if defined(SDMMC1) || defined(SDMMC2)
#if defined(STM32L4P5xx) || defined(STM32L4Q5xx) || defined(STM32L4R5xx) || defined(STM32L4R7xx) ||\
    defined(STM32L4R9xx) || defined(STM32L4S5xx) || defined(STM32L4S7xx) || defined(STM32L4S9xx)
#define STM32L4xx_PLUS
#endif
#if defined(STM32F7xx) || (defined(STM32L4xx) && !defined(STM32L4xx_PLUS))
#if !defined(SDMMC_CKIN_NA)
#define SDMMC_CKIN_NA
#endif
#if !defined(SDMMC_CDIR_NA)
#define SDMMC_CDIR_NA
#endif
#if !defined(SDMMC_D0DIR_NA)
#define SDMMC_D0DIR_NA
#endif
#if !defined(SDMMC_D123DIR_NA)
#define SDMMC_D123DIR_NA
#endif
#endif /* STM32F7xx || STM32L4xx_PLUS */
#endif /* SDMMC1 || SDMMC2 */

/* Default SDx pins definitions */
#ifndef SDX_D0
#define SDX_D0           NUM_DIGITAL_PINS
#endif
#ifndef SDX_D1
#define SDX_D1           NUM_DIGITAL_PINS
#endif
#ifndef SDX_D2
#define SDX_D2           NUM_DIGITAL_PINS
#endif
#ifndef SDX_D3
#define SDX_D3           NUM_DIGITAL_PINS
#endif
#ifndef SDX_CMD
#define SDX_CMD          NUM_DIGITAL_PINS
#endif
#ifndef SDX_CK
#define SDX_CK           NUM_DIGITAL_PINS
#endif
#if defined(SDMMC1) || defined(SDMMC2)
#ifndef SDX_CKIN
#define SDX_CKIN         NUM_DIGITAL_PINS
#endif
#ifndef SDX_CDIR
#define SDX_CDIR         NUM_DIGITAL_PINS
#endif
#ifndef SDX_D0DIR
#define SDX_D0DIR        NUM_DIGITAL_PINS
#endif
#ifndef SDX_D123DIR
#define SDX_D123DIR      NUM_DIGITAL_PINS
#endif
#endif /* SDMMC1 || SDMMC2 */

typedef struct {
  PinName pin_d0;
  PinName pin_d1;
  PinName pin_d2;
  PinName pin_d3;
  PinName pin_cmd;
  PinName pin_ck;
#if defined(SDMMC1) || defined(SDMMC2)
  PinName pin_ckin;
  PinName pin_cdir;
  PinName pin_d0dir;
  PinName pin_d123dir;
#endif
} SD_PinName_t;

extern SD_PinName_t SD_PinNames;

/* SD Exported Functions */
uint8_t BSP_SD_Init(void);
uint8_t BSP_SD_DeInit(void);
#if defined(USE_SD_TRANSCEIVER) && (USE_SD_TRANSCEIVER != 0U)
uint8_t BSP_SD_TransceiverPin(GPIO_TypeDef *enport, uint32_t enpin, GPIO_TypeDef *selport, uint32_t selpin);
#endif
uint8_t BSP_SD_DetectPin(GPIO_TypeDef *port, uint32_t pin);
uint8_t BSP_SD_DetectITConfig(void (*callback)(void));
uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout);
uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout);
uint8_t BSP_SD_Erase(uint64_t StartAddr, uint64_t EndAddr);
uint8_t BSP_SD_GetCardState(void);
void    BSP_SD_GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo);
uint8_t BSP_SD_IsDetected(void);

/* These __weak function can be surcharged by application code in case the current settings (e.g. DMA stream)
   need to be changed for specific needs */
void    BSP_SD_MspInit(SD_HandleTypeDef *hsd, void *Params);
void    BSP_SD_Detect_MspInit(SD_HandleTypeDef *hsd, void *Params);
void    BSP_SD_MspDeInit(SD_HandleTypeDef *hsd, void *Params);
#if defined(USE_SD_TRANSCEIVER) && (USE_SD_TRANSCEIVER != 0U)
void    BSP_SD_Transceiver_MspInit(SD_HandleTypeDef *hsd, void *Params);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __BSP_SD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
