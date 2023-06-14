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
#include "stm32_def.h"
#if !defined(STM32_CORE_VERSION) || (STM32_CORE_VERSION  <= 0x01050000)
#include "variant.h"
#endif
#if !defined(STM32_CORE_VERSION) || (STM32_CORE_VERSION  <= 0x01060100)
#error "This library version required a STM32 core version > 1.6.1.\
Please update the core or install previous library version."
#endif

/* For backward compatibility */
#if defined(SD_TRANSCEIVER_MODE) && !defined(USE_SD_TRANSCEIVER)
#define USE_SD_TRANSCEIVER        1
#endif

/*SD Card information structure */

#define SD_CardInfo HAL_SD_CardInfoTypeDef

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
