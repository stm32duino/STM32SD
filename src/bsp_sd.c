/**
******************************************************************************
* @file    bsp_sd.c
* @brief   This file includes the uSD card driver mounted on stm32
*          board.
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

/* Includes ------------------------------------------------------------------*/
#include "bsp_sd.h"
#include "core_debug.h"
#include "interrupt.h"
#include "PeripheralPins.h"
#include "stm32yyxx_ll_gpio.h"

/* Definition for BSP SD */
#if defined(SDMMC1) || defined(SDMMC2)
  #ifndef SD_INSTANCE
    #if defined(SDMMC1)
      #define SD_INSTANCE              SDMMC1
    #else
      #define SD_INSTANCE              SDMMC2
    #endif
  #endif

  #define SD_CLK_EDGE              SDMMC_CLOCK_EDGE_RISING
  #if defined(SDMMC_CLOCK_BYPASS_DISABLE)
    #define SD_CLK_BYPASS            SDMMC_CLOCK_BYPASS_DISABLE
  #endif
  #define SD_CLK_PWR_SAVE          SDMMC_CLOCK_POWER_SAVE_DISABLE
  #define SD_BUS_WIDE_1B           SDMMC_BUS_WIDE_1B
  #define SD_BUS_WIDE_4B           SDMMC_BUS_WIDE_4B
  #define SD_BUS_WIDE_8B           SDMMC_BUS_WIDE_8B
  #define SD_HW_FLOW_CTRL_ENABLE   SDMMC_HARDWARE_FLOW_CONTROL_ENABLE
  #define SD_HW_FLOW_CTRL_DISABLE  SDMMC_HARDWARE_FLOW_CONTROL_DISABLE

  #ifndef SD_CLK_DIV
    #if defined(SDMMC_TRANSFER_CLK_DIV)
      #define SD_CLK_DIV               SDMMC_TRANSFER_CLK_DIV
    #else
      #define SD_CLK_DIV               SDMMC_NSpeed_CLK_DIV
    #endif
  #endif

  #if defined(USE_SD_TRANSCEIVER) && (USE_SD_TRANSCEIVER != 0U)
    #if defined(SDMMC_TRANSCEIVER_ENABLE)
      #define SD_TRANSCEIVER_ENABLE    SDMMC_TRANSCEIVER_ENABLE
      #define SD_TRANSCEIVER_DISABLE   SDMMC_TRANSCEIVER_DISABLE
    #else
      #define SD_TRANSCEIVER_ENABLE    SDMMC_TRANSCEIVER_PRESENT
      #define SD_TRANSCEIVER_DISABLE   SDMMC_TRANSCEIVER_NOT_PRESENT
    #endif
  #endif

#elif defined(SDIO)
  #define SD_INSTANCE              SDIO
  #define SD_CLK_EDGE              SDIO_CLOCK_EDGE_RISING
  #if defined(SDIO_CLOCK_BYPASS_DISABLE)
    #define SD_CLK_BYPASS            SDIO_CLOCK_BYPASS_DISABLE
  #endif
  #define SD_CLK_PWR_SAVE          SDIO_CLOCK_POWER_SAVE_DISABLE
  #define SD_BUS_WIDE_1B           SDIO_BUS_WIDE_1B
  #define SD_BUS_WIDE_4B           SDIO_BUS_WIDE_4B
  #define SD_BUS_WIDE_8B           SDIO_BUS_WIDE_8B
  #define SD_HW_FLOW_CTRL_ENABLE   SDIO_HARDWARE_FLOW_CONTROL_ENABLE
  #define SD_HW_FLOW_CTRL_DISABLE  SDIO_HARDWARE_FLOW_CONTROL_DISABLE
  #ifndef SD_CLK_DIV
    #define SD_CLK_DIV               SDIO_TRANSFER_CLK_DIV
  #endif
#else
  #error "Unknown SD_INSTANCE"
#endif

#ifndef SD_HW_FLOW_CTRL
  #define SD_HW_FLOW_CTRL          SD_HW_FLOW_CTRL_DISABLE
#endif

#ifndef SD_BUS_WIDE
  #define SD_BUS_WIDE              SD_BUS_WIDE_4B
#endif

/* BSP SD Private Variables */
static SD_HandleTypeDef uSdHandle;
static uint32_t SD_detect_ll_gpio_pin = LL_GPIO_PIN_ALL;
static GPIO_TypeDef *SD_detect_gpio_port = GPIOA;
#if defined(USE_SD_TRANSCEIVER) && (USE_SD_TRANSCEIVER != 0U)
  static uint32_t SD_trans_en_ll_gpio_pin = LL_GPIO_PIN_ALL;
  static GPIO_TypeDef *SD_trans_en_gpio_port = GPIOA;
  static uint32_t SD_trans_sel_ll_gpio_pin = LL_GPIO_PIN_ALL;
  static GPIO_TypeDef *SD_trans_sel_gpio_port = GPIOA;
#endif
#define SD_TRANSFER_OK                ((uint8_t)0x00)
#define SD_TRANSFER_BUSY              ((uint8_t)0x01)
SD_PinName_t SD_PinNames = {
  .pin_d0 = NC,
  .pin_d1 = NC,
  .pin_d2 = NC,
  .pin_d3 = NC,
  .pin_cmd = NC,
  .pin_ck = NC,
#if defined(SDMMC1) || defined(SDMMC2)
  .pin_ckin = NC,
  .pin_cdir = NC,
  .pin_d0dir = NC,
  .pin_d123dir = NC
#endif
};

#if defined(STM32_CORE_VERSION) && (STM32_CORE_VERSION > 0x02050000)
/**
  * @brief  Get the SD card device instance from pins
  * @retval SD status
  */
uint8_t BSP_SD_GetInstance(void)
{
  SD_TypeDef *sd_d0 = NP;
  SD_TypeDef *sd_d1 = NP;
  SD_TypeDef *sd_d2 = NP;
  SD_TypeDef *sd_d3 = NP;
  SD_TypeDef *sd_cmd = NP;
  SD_TypeDef *sd_ck = NP;

  if (SD_PinNames.pin_d0 == NC) {
    /* No pin defined assume to use first pin available in each PinMap_SD_* arrays */
    SD_PinNames.pin_d0 = PinMap_SD_DATA0[0].pin;
    SD_PinNames.pin_d1 = PinMap_SD_DATA1[0].pin;
    SD_PinNames.pin_d2 = PinMap_SD_DATA2[0].pin;
    SD_PinNames.pin_d3 = PinMap_SD_DATA3[0].pin;
    SD_PinNames.pin_cmd = PinMap_SD_CMD[0].pin;
    SD_PinNames.pin_ck = PinMap_SD_CK[0].pin;
#if defined(SDMMC1) || defined(SDMMC2)
#if !defined(SDMMC_CKIN_NA)
    SD_PinNames.pin_ckin = PinMap_SD_CKIN[0].pin;
#endif
#if !defined(SDMMC_CDIR_NA)
    SD_PinNames.pin_cdir = PinMap_SD_CDIR[0].pin;
#endif
#if !defined(SDMMC_D0DIR_NA)
    SD_PinNames.pin_d0dir = PinMap_SD_D0DIR[0].pin;
#endif
#if !defined(SDMMC_D123DIR_NA)
    SD_PinNames.pin_d123dir = PinMap_SD_D123DIR[0].pin;
#endif
#endif /* SDMMC1 || SDMMC2 */
  }
  /* Get SD instance from pins */
  sd_d0 = pinmap_peripheral(SD_PinNames.pin_d0, PinMap_SD_DATA0);
  sd_d1 = pinmap_peripheral(SD_PinNames.pin_d1, PinMap_SD_DATA1);
  sd_d2 = pinmap_peripheral(SD_PinNames.pin_d2, PinMap_SD_DATA2);
  sd_d3 = pinmap_peripheral(SD_PinNames.pin_d3, PinMap_SD_DATA3);

  sd_cmd = pinmap_peripheral(SD_PinNames.pin_cmd, PinMap_SD_CMD);
  sd_ck = pinmap_peripheral(SD_PinNames.pin_ck, PinMap_SD_CK);

  /* Pins Dx/cmd/CK must not be NP. */
  if (sd_d0 == NP || sd_d1 == NP || sd_d2 == NP || sd_d3 == NP || sd_cmd == NP || sd_ck == NP) {
    core_debug("ERROR: at least one SD pin has no peripheral\n");
    return MSD_ERROR;
  }

  SD_TypeDef *sd_d01 = pinmap_merge_peripheral(sd_d0, sd_d1);
  SD_TypeDef *sd_d23 = pinmap_merge_peripheral(sd_d2, sd_d3);
  SD_TypeDef *sd_cx = pinmap_merge_peripheral(sd_cmd, sd_ck);
  SD_TypeDef *sd_dx = pinmap_merge_peripheral(sd_d01, sd_d23);
  SD_TypeDef *sd_base = pinmap_merge_peripheral(sd_dx, sd_cx);
  if (sd_d01 == NP || sd_d23 == NP || sd_cx == NP || sd_dx == NP || sd_base == NP) {
    core_debug("ERROR: SD pins mismatch\n");
    return MSD_ERROR;
  }
  uSdHandle.Instance = sd_base;
#if defined(SDMMC1) || defined(SDMMC2)
#if !defined(SDMMC_CKIN_NA)
  if (SD_PinNames.pin_ckin != NC) {
    SD_TypeDef *sd_ckin = pinmap_peripheral(SD_PinNames.pin_ckin, PinMap_SD_CKIN);
    if (pinmap_merge_peripheral(sd_ckin, sd_base) == NP) {
      core_debug("ERROR: SD CKIN pin mismatch\n");
      return MSD_ERROR;
    }
  }
#endif
#if !defined(SDMMC_CDIR_NA)
  if (SD_PinNames.pin_cdir != NC) {
    SD_TypeDef *sd_cdir = pinmap_peripheral(SD_PinNames.pin_cdir, PinMap_SD_CDIR);
    if (pinmap_merge_peripheral(sd_cdir, sd_base) == NP) {
      core_debug("ERROR: SD CDIR pin mismatch\n");
      return MSD_ERROR;
    }
  }
#endif
#if !defined(SDMMC_D0DIR_NA)
  if (SD_PinNames.pin_cdir != NC) {
    SD_TypeDef *sd_d0dir = pinmap_peripheral(SD_PinNames.pin_d0dir, PinMap_SD_D0DIR);
    if (pinmap_merge_peripheral(sd_d0dir, sd_base) == NP) {
      core_debug("ERROR: SD DODIR pin mismatch\n");
      return MSD_ERROR;
    }
  }
#endif
#if !defined(SDMMC_D123DIR_NA)
  if (SD_PinNames.pin_cdir != NC) {
    SD_TypeDef *sd_d123dir = pinmap_peripheral(SD_PinNames.pin_d123dir, PinMap_SD_D123DIR);
    if (pinmap_merge_peripheral(sd_d123dir, sd_base) == NP) {
      core_debug("ERROR: SD D123DIR pin mismatch\n");
      return MSD_ERROR;
    }
  }
#endif
#endif /* SDMMC1 || SDMMC2 */
  /* Are all pins connected to the same SDx instance? */
  if (uSdHandle.Instance == NP) {
    core_debug("ERROR: SD pins mismatch\n");
    return MSD_ERROR;
  }
  return MSD_OK;
}
#endif /* STM32_CORE_VERSION && (STM32_CORE_VERSION > 0x02050000) */

/**
  * @brief  Initializes the SD card device with CS check if any.
  * @retval SD status
  */
uint8_t BSP_SD_Init(void)
{
  uint8_t sd_state = MSD_OK;

  /* Check if SD is not yet initialized */
  if (uSdHandle.State == HAL_SD_STATE_RESET) {
    /* uSD device interface configuration */
#if !defined(STM32_CORE_VERSION) || (STM32_CORE_VERSION <= 0x02050000)
    uSdHandle.Instance = SD_INSTANCE;
#else
    if (BSP_SD_GetInstance() == MSD_ERROR) {
      return MSD_ERROR;
    }
#endif /* !STM32_CORE_VERSION || (STM32_CORE_VERSION <= 0x02050000) */

    uSdHandle.Init.ClockEdge           = SD_CLK_EDGE;
#if defined(SD_CLK_BYPASS)
    uSdHandle.Init.ClockBypass         = SD_CLK_BYPASS;
#endif
    uSdHandle.Init.ClockPowerSave      = SD_CLK_PWR_SAVE;
    uSdHandle.Init.BusWide             = SD_BUS_WIDE_1B;
    uSdHandle.Init.HardwareFlowControl = SD_HW_FLOW_CTRL;
    uSdHandle.Init.ClockDiv            = SD_CLK_DIV;
#if defined(USE_SD_TRANSCEIVER) && (USE_SD_TRANSCEIVER != 0U)
#if defined(SDMMC_TRANSCEIVER_ENABLE)
    uSdHandle.Init.Transceiver = SD_TRANSCEIVER_ENABLE;
#else
    uSdHandle.Init.TranceiverPresent   = SD_TRANSCEIVER_ENABLE;
#endif
    BSP_SD_Transceiver_MspInit(&uSdHandle, NULL);
#endif

    if (SD_detect_ll_gpio_pin != LL_GPIO_PIN_ALL) {
      /* Msp SD Detect pin initialization */
      BSP_SD_Detect_MspInit(&uSdHandle, NULL);
      if (BSP_SD_IsDetected() != SD_PRESENT) { /* Check if SD card is present */
        return MSD_ERROR_SD_NOT_PRESENT;
      }
    }

    /* Msp SD initialization */
    BSP_SD_MspInit(&uSdHandle, NULL);

    /* HAL SD initialization */
    if (HAL_SD_Init(&uSdHandle) != HAL_OK) {
      sd_state = MSD_ERROR;
    }

    /* Configure SD Bus width */
    if (sd_state == MSD_OK) {
      /* Enable wide operation */
      if (HAL_SD_ConfigWideBusOperation(&uSdHandle, SD_BUS_WIDE) != HAL_OK) {
        sd_state = MSD_ERROR;
      } else {
        sd_state = MSD_OK;
      }
    }
  }
  return  sd_state;
}

/**
  * @brief  DeInitializes the SD card device.
  * @retval SD status
  */
uint8_t BSP_SD_DeInit(void)
{
  uint8_t sd_state = MSD_OK;

#if !defined(STM32_CORE_VERSION) || (STM32_CORE_VERSION <= 0x02050000)
  uSdHandle.Instance = SD_INSTANCE;
#else
  if (BSP_SD_GetInstance() == MSD_ERROR) {
    return MSD_ERROR;
  }
#endif

  /* HAL SD deinitialization */
  if (HAL_SD_DeInit(&uSdHandle) != HAL_OK) {
    sd_state = MSD_ERROR;
  }

  /* Msp SD deinitialization */
  BSP_SD_MspDeInit(&uSdHandle, NULL);

  return  sd_state;
}

#if defined(USE_SD_TRANSCEIVER) && (USE_SD_TRANSCEIVER != 0U)
/**
  * @brief  Set the transceiver pin and port.
  * @param  enport enable gpio port
  * @param  enpin enable gpio pin
  * @param  selport select gpio port
  * @param  selpin select gpio pin
  * @retval SD status
  */
uint8_t BSP_SD_TransceiverPin(GPIO_TypeDef *enport, uint32_t enpin, GPIO_TypeDef *selport, uint32_t selpin)
{
  if ((enport != 0) && (selport != 0)) {
    SD_trans_en_ll_gpio_pin = enpin;
    SD_trans_en_gpio_port = enport;
    SD_trans_sel_ll_gpio_pin = selpin;
    SD_trans_sel_gpio_port = selport;
    return MSD_OK;
  }
  return MSD_ERROR;
}
#endif

/**
  * @brief  Set the SD card device detect pin and port.
  * @param  port one of the gpio port
  * @param  pin one of the gpio pin
  * @retval SD status
  */
uint8_t BSP_SD_DetectPin(GPIO_TypeDef *port, uint32_t pin)
{
  if (port != 0) {
    SD_detect_ll_gpio_pin = pin;
    SD_detect_gpio_port = port;
    return MSD_OK;
  }
  return MSD_ERROR;
}

/**
  * @brief  Configures Interrupt mode for SD detection pin.
  * @retval Status
  */
uint8_t BSP_SD_DetectITConfig(void (*callback)(void))
{
  uint8_t sd_state = MSD_ERROR;
  if (SD_detect_ll_gpio_pin != LL_GPIO_PIN_ALL) {
    LL_GPIO_SetPinPull(SD_detect_gpio_port, SD_detect_ll_gpio_pin, LL_GPIO_PULL_UP);
    uint16_t SD_detect_gpio_pin = GPIO_PIN_All;
    switch (SD_detect_ll_gpio_pin) {
      case LL_GPIO_PIN_0:
        SD_detect_gpio_pin = GPIO_PIN_0;
        break;
      case LL_GPIO_PIN_1:
        SD_detect_gpio_pin = GPIO_PIN_1;
        break;
      case LL_GPIO_PIN_2:
        SD_detect_gpio_pin = GPIO_PIN_2;
        break;
      case LL_GPIO_PIN_3:
        SD_detect_gpio_pin = GPIO_PIN_3;
        break;
      case LL_GPIO_PIN_4:
        SD_detect_gpio_pin = GPIO_PIN_4;
        break;
      case LL_GPIO_PIN_5:
        SD_detect_gpio_pin = GPIO_PIN_5;
        break;
      case LL_GPIO_PIN_6:
        SD_detect_gpio_pin = GPIO_PIN_6;
        break;
      case LL_GPIO_PIN_7:
        SD_detect_gpio_pin = GPIO_PIN_7;
        break;
      case LL_GPIO_PIN_8:
        SD_detect_gpio_pin = GPIO_PIN_8;
        break;
      case LL_GPIO_PIN_9:
        SD_detect_gpio_pin = GPIO_PIN_9;
        break;
      case LL_GPIO_PIN_10:
        SD_detect_gpio_pin = GPIO_PIN_10;
        break;
      case LL_GPIO_PIN_11:
        SD_detect_gpio_pin = GPIO_PIN_11;
        break;
      case LL_GPIO_PIN_12:
        SD_detect_gpio_pin = GPIO_PIN_12;
        break;
      case LL_GPIO_PIN_13:
        SD_detect_gpio_pin = GPIO_PIN_13;
        break;
      case LL_GPIO_PIN_14:
        SD_detect_gpio_pin = GPIO_PIN_14;
        break;
      case LL_GPIO_PIN_15:
        SD_detect_gpio_pin = GPIO_PIN_15;
        break;
      default:
        Error_Handler();
        break;
    }
    stm32_interrupt_enable(SD_detect_gpio_port, SD_detect_gpio_pin, callback, GPIO_MODE_IT_RISING_FALLING);
    sd_state = MSD_OK;
  }
  return sd_state;
}

/**
 * @brief  Detects if SD card is correctly plugged in the memory slot or not.
 * @retval Returns if SD is detected or not
 */
uint8_t BSP_SD_IsDetected(void)
{
  uint8_t  status = SD_NOT_PRESENT;
  /* Check SD card detect pin */
  if (!LL_GPIO_IsInputPinSet(SD_detect_gpio_port, SD_detect_ll_gpio_pin)) {
    status = SD_PRESENT;
  }
  return status;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read
  * @param  NumOfBlocks: Number of SD blocks to read
  * @param  Timeout: Timeout for read operation
  * @retval SD status
  */
uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
  if (HAL_SD_ReadBlocks(&uSdHandle, (uint8_t *)pData, ReadAddr, NumOfBlocks, Timeout) != HAL_OK) {
    return MSD_ERROR;
  } else {
    return MSD_OK;
  }
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in polling mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written
  * @param  NumOfBlocks: Number of SD blocks to write
  * @param  Timeout: Timeout for write operation
  * @retval SD status
  */
uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
  if (HAL_SD_WriteBlocks(&uSdHandle, (uint8_t *)pData, WriteAddr, NumOfBlocks, Timeout) != HAL_OK) {
    return MSD_ERROR;
  } else {
    return MSD_OK;
  }
}

/**
  * @brief  Erases the specified memory area of the given SD card.
  * @param  StartAddr: Start byte address
  * @param  EndAddr: End byte address
  * @retval SD status
  */
uint8_t BSP_SD_Erase(uint64_t StartAddr, uint64_t EndAddr)
{
  if (HAL_SD_Erase(&uSdHandle, StartAddr, EndAddr) != HAL_OK) {
    return MSD_ERROR;
  } else {
    return MSD_OK;
  }
}

/**
  * @brief  Initializes the SD MSP.
  * @param  hsd: SD handle
  * @param  Params : pointer on additional configuration parameters, can be NULL.
  */
__weak void BSP_SD_MspInit(SD_HandleTypeDef *hsd, void *Params)
{
  UNUSED(Params);
#if !defined(STM32_CORE_VERSION) || (STM32_CORE_VERSION <= 0x02050000)
  /* Configure SD GPIOs */
  const PinMap *map = PinMap_SD;
  while (map->pin != NC) {
    pin_function(map->pin, map->function);
    map++;
  }
#else
  /* Configure SD GPIO pins */
  pinmap_pinout(SD_PinNames.pin_d0, PinMap_SD_DATA0);
  pinmap_pinout(SD_PinNames.pin_d1, PinMap_SD_DATA1);
  pinmap_pinout(SD_PinNames.pin_d2, PinMap_SD_DATA2);
  pinmap_pinout(SD_PinNames.pin_d3, PinMap_SD_DATA3);
  pinmap_pinout(SD_PinNames.pin_cmd, PinMap_SD_CMD);
  pinmap_pinout(SD_PinNames.pin_ck, PinMap_SD_CK);
#if defined(SDMMC1) || defined(SDMMC2)
#if !defined(SDMMC_CKIN_NA)
  if (SD_PinNames.pin_ckin != NC) {
    pinmap_pinout(SD_PinNames.pin_ckin, PinMap_SD_CKIN);
  }
#endif
#if !defined(SDMMC_CDIR_NA)
  if (SD_PinNames.pin_cdir != NC) {
    pinmap_pinout(SD_PinNames.pin_cdir, PinMap_SD_CDIR);
  }
#endif
#if !defined(SDMMC_D0DIR_NA)
  if (SD_PinNames.pin_d0dir != NC) {
    pinmap_pinout(SD_PinNames.pin_d0dir, PinMap_SD_D0DIR);
  }
#endif
#if !defined(SDMMC_D123DIR_NA)
  if (SD_PinNames.pin_d123dir != NC) {
    pinmap_pinout(SD_PinNames.pin_d123dir, PinMap_SD_D123DIR);
  }
#endif
#endif /* SDMMC1 || SDMMC2 */
#endif /* !STM32_CORE_VERSION || (STM32_CORE_VERSION <= 0x02050000) */
  /* Enable SD clock */
#if defined(SDMMC1) || defined(SDMMC2)
#if defined(SDMMC1)
  if (hsd->Instance == SDMMC1) {
    __HAL_RCC_SDMMC1_CLK_ENABLE();
  }
#endif
#if defined(SDMMC2)
  if (hsd->Instance == SDMMC2) {
    __HAL_RCC_SDMMC2_CLK_ENABLE();
  }
#endif
#else
  UNUSED(hsd);
  __HAL_RCC_SDIO_CLK_ENABLE();
#endif
}

/**
  * @brief  Initializes the SD Detect pin MSP.
  * @param  hsd: SD handle
  * @param  Params : pointer on additional configuration parameters, can be NULL.
  */
__weak void BSP_SD_Detect_MspInit(SD_HandleTypeDef *hsd, void *Params)
{
  UNUSED(hsd);
  UNUSED(Params);

  /* GPIO configuration in input for uSD_Detect signal */
#ifdef LL_GPIO_SPEED_FREQ_VERY_HIGH
  LL_GPIO_SetPinSpeed(SD_detect_gpio_port, SD_detect_ll_gpio_pin, LL_GPIO_SPEED_FREQ_VERY_HIGH);
#else
  LL_GPIO_SetPinSpeed(SD_detect_gpio_port, SD_detect_ll_gpio_pin, LL_GPIO_SPEED_FREQ_HIGH);
#endif
  LL_GPIO_SetPinMode(SD_detect_gpio_port, SD_detect_ll_gpio_pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinPull(SD_detect_gpio_port, SD_detect_ll_gpio_pin, LL_GPIO_PULL_UP);
}

/**
  * @brief  DeInitializes the SD MSP.
  * @param  hsd: SD handle
  * @param  Params : pointer on additional configuration parameters, can be NULL.
  */
__weak void BSP_SD_MspDeInit(SD_HandleTypeDef *hsd, void *Params)
{
  UNUSED(Params);
  /* DeInit GPIO pins can be done in the application
     (by surcharging this __weak function) */
#if !defined(STM32_CORE_VERSION) || (STM32_CORE_VERSION <= 0x02050000)
  const PinMap *map = PinMap_SD;
  while (map->pin != NC) {
    HAL_GPIO_DeInit((GPIO_TypeDef *)STM_PORT(map->pin), STM_GPIO_PIN(map->pin));
    map++;
  }
#else
  HAL_GPIO_DeInit((GPIO_TypeDef *)STM_PORT(SD_PinNames.pin_d0), STM_GPIO_PIN(SD_PinNames.pin_d0));
  HAL_GPIO_DeInit((GPIO_TypeDef *)STM_PORT(SD_PinNames.pin_d1), STM_GPIO_PIN(SD_PinNames.pin_d1));
  HAL_GPIO_DeInit((GPIO_TypeDef *)STM_PORT(SD_PinNames.pin_d2), STM_GPIO_PIN(SD_PinNames.pin_d2));
  HAL_GPIO_DeInit((GPIO_TypeDef *)STM_PORT(SD_PinNames.pin_d3), STM_GPIO_PIN(SD_PinNames.pin_d3));
  HAL_GPIO_DeInit((GPIO_TypeDef *)STM_PORT(SD_PinNames.pin_cmd), STM_GPIO_PIN(SD_PinNames.pin_cmd));
  HAL_GPIO_DeInit((GPIO_TypeDef *)STM_PORT(SD_PinNames.pin_ck), STM_GPIO_PIN(SD_PinNames.pin_ck));
#if defined(SDMMC1) || defined(SDMMC2)
#if !defined(SDMMC_CKIN_NA)
  if (SD_PinNames.pin_ckin != NC) {
    HAL_GPIO_DeInit((GPIO_TypeDef *)STM_PORT(SD_PinNames.pin_ckin), STM_GPIO_PIN(SD_PinNames.pin_ckin));
  }
#endif
#if !defined(SDMMC_CDIR_NA)
  if (SD_PinNames.pin_cdir != NC) {
    HAL_GPIO_DeInit((GPIO_TypeDef *)STM_PORT(SD_PinNames.pin_cdir), STM_GPIO_PIN(SD_PinNames.pin_cdir));
  }
#endif
#if !defined(SDMMC_D0DIR_NA)
  if (SD_PinNames.pin_d0dir != NC) {
    HAL_GPIO_DeInit((GPIO_TypeDef *)STM_PORT(SD_PinNames.pin_d0dir), STM_GPIO_PIN(SD_PinNames.pin_d0dir));
  }
#endif
#if !defined(SDMMC_D123DIR_NA)
  if (SD_PinNames.pin_d123dir != NC) {
    HAL_GPIO_DeInit((GPIO_TypeDef *)STM_PORT(SD_PinNames.pin_d123dir), STM_GPIO_PIN(SD_PinNames.pin_d123dir));
  }
#endif
#endif /* SDMMC1 || SDMMC2 */
#endif /* !STM32_CORE_VERSION || (STM32_CORE_VERSION <= 0x02050000) */

  /* Disable SD clock */
#if defined(SDMMC1) || defined(SDMMC2)
#if defined(SDMMC1)
  if (hsd->Instance == SDMMC1) {
    __HAL_RCC_SDMMC1_CLK_DISABLE();
  }
#endif
#if defined(SDMMC2)
  if (hsd->Instance == SDMMC2) {
    __HAL_RCC_SDMMC2_CLK_DISABLE();
  }
#endif
#else
  UNUSED(hsd);
  __HAL_RCC_SDIO_CLK_DISABLE();
#endif
}

#if defined(USE_SD_TRANSCEIVER) && (USE_SD_TRANSCEIVER != 0U)
/**
  * @brief  Initializes the SD Transceiver pin MSP.
  * @param  hsd: SD handle
  * @param  Params : pointer on additional configuration parameters, can be NULL.
  */
__weak void BSP_SD_Transceiver_MspInit(SD_HandleTypeDef *hsd, void *Params)
{
  UNUSED(hsd);
  UNUSED(Params);

  LL_GPIO_SetPinSpeed(SD_trans_en_gpio_port, SD_trans_en_ll_gpio_pin, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinMode(SD_trans_en_gpio_port, SD_trans_en_ll_gpio_pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinPull(SD_trans_en_gpio_port, SD_trans_en_ll_gpio_pin, LL_GPIO_PULL_NO);

  LL_GPIO_SetPinSpeed(SD_trans_sel_gpio_port, SD_trans_sel_ll_gpio_pin, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinMode(SD_trans_sel_gpio_port, SD_trans_sel_ll_gpio_pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinPull(SD_trans_sel_gpio_port, SD_trans_sel_ll_gpio_pin, LL_GPIO_PULL_NO);

  /* Enable the level shifter */
  LL_GPIO_SetOutputPin(SD_trans_en_gpio_port, SD_trans_en_ll_gpio_pin);

  /* By default start with the default voltage */
  LL_GPIO_ResetOutputPin(SD_trans_sel_gpio_port, SD_trans_sel_ll_gpio_pin);
}

/**
  * @brief  Enable/Disable the SD Transceiver 1.8V Mode Callback.
  * @param  status: Voltage Switch State
  * @retval None
  */
#if defined(SDMMC_TRANSCEIVER_ENABLE)
  void HAL_SDEx_DriveTransceiver_1_8V_Callback(FlagStatus status)
#else
  void HAL_SD_DriveTransceiver_1_8V_Callback(FlagStatus status)
#endif
{
  if (status == SET) {
    LL_GPIO_SetOutputPin(SD_trans_sel_gpio_port, SD_trans_sel_ll_gpio_pin);
  } else {
    LL_GPIO_ResetOutputPin(SD_trans_sel_gpio_port, SD_trans_sel_ll_gpio_pin);
  }
}
#endif /* USE_SD_TRANSCEIVER && (USE_SD_TRANSCEIVER != 0U) */

/**
  * @brief  Gets the current SD card data status.
  * @retval Data transfer state.
  *          This value can be one of the following values:
  *            @arg  SD_TRANSFER_OK: No data transfer is acting
  *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
  */
uint8_t BSP_SD_GetCardState(void)
{
  return ((HAL_SD_GetCardState(&uSdHandle) == HAL_SD_CARD_TRANSFER) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

/**
  * @brief  Get SD information about specific SD card.
  * @param  CardInfo: Pointer to HAL_SD_CardInfoTypedef structure
  */
void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo)
{
  /* Get SD card Information */
  HAL_SD_GetCardInfo(&uSdHandle, CardInfo);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
