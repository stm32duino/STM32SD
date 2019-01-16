  /**
  ******************************************************************************
  * @file    bsp_sd.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    10-June-2016
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

/* File Info : -----------------------------------------------------------------
                                   User NOTES
1. How To use this driver:
--------------------------
   - This driver is used to drive the micro SD external card mounted on a board.
   - This driver does not need a specific component driver for the micro SD device
     to be included with.

2. Driver description:
---------------------
  + Initialization steps:
     o Initialize the micro SD card using the BSP_SD_Init() function. This
       function includes the MSP layer hardware resources initialization and the
       SDIO interface configuration to interface with the external micro SD. It
       also includes the micro SD initialization sequence.
     o To check the SD card presence you can use the function BSP_SD_IsDetected() which
       returns the detection status
     o If SD presence detection interrupt mode is desired, you must configure the
       SD detection interrupt mode by calling the function BSP_SD_ITConfig(). The interrupt
       is generated as an external interrupt whenever the micro SD card is
       plugged/unplugged in/from the board. The SD detection interrupt
       is handled by calling the function BSP_SD_DetectIT() which is called in the IRQ
       handler file, the user callback is implemented in the function BSP_SD_DetectCallback().
     o The function BSP_SD_GetCardInfo() is used to get the micro SD card information
       which is stored in the structure "HAL_SD_CardInfoTypedef".

  + Micro SD card operations
     o The micro SD card can be accessed with read/write block(s) operations once
       it is reay for access. The access cand be performed whether using the polling
       mode by calling the functions BSP_SD_ReadBlocks()/BSP_SD_WriteBlocks(), or by DMA
       transfer using the functions BSP_SD_ReadBlocks_DMA()/BSP_SD_WriteBlocks_DMA()
     o The DMA transfer complete is used with interrupt mode. Once the SD transfer
       is complete, the SD interrupt is handeled using the function BSP_SD_IRQHandler(),
       the DMA Tx/Rx transfer complete are handeled using the functions
       BSP_SD_DMA_Tx_IRQHandler()/BSP_SD_DMA_Rx_IRQHandler(). The corresponding user callbacks
       are implemented by the user at application level.
     o The SD erase block(s) is performed using the function BSP_SD_Erase() with specifying
       the number of blocks to erase.
     o The SD runtime status is returned when calling the function BSP_SD_GetStatus().

------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "bsp_sd.h"

#ifdef SDMMC1
/* Definition for BSP SD */
#define SD_INSTANCE            SDMMC1
#define SD_CLK_ENABLE          __HAL_RCC_SDMMC1_CLK_ENABLE
#define SD_CLK_DISABLE         __HAL_RCC_SDMMC1_CLK_DISABLE
#define SD_CLK_EDGE            SDMMC_CLOCK_EDGE_RISING
#define SD_CLK_BYPASS          SDMMC_CLOCK_BYPASS_DISABLE
#define SD_CLK_PWR_SAVE        SDMMC_CLOCK_POWER_SAVE_DISABLE
#define SD_BUS_WIDE_1B         SDMMC_BUS_WIDE_1B
#define SD_BUS_WIDE_4B         SDMMC_BUS_WIDE_4B
#ifndef SD_HW_FLOW_CTRL
#define SD_HW_FLOW_CTRL        SDMMC_HARDWARE_FLOW_CONTROL_DISABLE
#endif
#ifdef STM32H7xx
#define SD_CLK_DIV             1
#else
#define SD_CLK_DIV             SDMMC_TRANSFER_CLK_DIV
#endif
/* Definition for MSP SD */
#define SD_AF                  GPIO_AF12_SDMMC1
#elif defined(SDIO)
/* Definition for BSP SD */
#define SD_INSTANCE            SDIO
#define SD_CLK_ENABLE          __HAL_RCC_SDIO_CLK_ENABLE
#define SD_CLK_DISABLE         __HAL_RCC_SDIO_CLK_DISABLE
#define SD_CLK_EDGE            SDIO_CLOCK_EDGE_RISING
#define SD_CLK_BYPASS          SDIO_CLOCK_BYPASS_DISABLE
#define SD_CLK_PWR_SAVE        SDIO_CLOCK_POWER_SAVE_DISABLE
#define SD_BUS_WIDE_1B         SDIO_BUS_WIDE_1B
#define SD_BUS_WIDE_4B         SDIO_BUS_WIDE_4B
#ifndef SD_HW_FLOW_CTRL
#define SD_HW_FLOW_CTRL        SDIO_HARDWARE_FLOW_CONTROL_DISABLE
#endif
#define SD_CLK_DIV             SDIO_TRANSFER_CLK_DIV
/* Definition for MSP SD */
#ifndef STM32F1xx
#define SD_AF                  GPIO_AF12_SDIO
#endif
#else
#error "Unknown SD_INSTANCE"
#endif

#ifdef GPIO_SPEED_FREQ_VERY_HIGH
#define SD_GPIO_SPEED  GPIO_SPEED_FREQ_VERY_HIGH
#else
#define SD_GPIO_SPEED  GPIO_SPEED_FREQ_HIGH
#endif

/* BSP SD Private Variables */
static SD_HandleTypeDef uSdHandle;
static uint32_t SD_detect_gpio_pin = GPIO_PIN_All;
static GPIO_TypeDef *SD_detect_gpio_port = GPIOA;
#ifndef STM32L1xx
#define SD_OK                         HAL_OK
#define SD_TRANSFER_OK                ((uint8_t)0x00)
#define SD_TRANSFER_BUSY              ((uint8_t)0x01)
#else /* STM32L1xx */
static SD_CardInfo uSdCardInfo;
#endif


/**
  * @brief  Initializes the SD card device with CS check if any.
  * @retval SD status
  */
uint8_t BSP_SD_Init(void)
{
  uint8_t sd_state = MSD_OK;

  /* PLLSAI is dedicated to LCD periph. Do not use it to get 48MHz*/

  /* uSD device interface configuration */
  uSdHandle.Instance = SD_INSTANCE;

  uSdHandle.Init.ClockEdge           = SD_CLK_EDGE;
#ifndef STM32H7xx
  uSdHandle.Init.ClockBypass         = SD_CLK_BYPASS;
#endif
  uSdHandle.Init.ClockPowerSave      = SD_CLK_PWR_SAVE;
  uSdHandle.Init.BusWide             = SD_BUS_WIDE_1B;
  uSdHandle.Init.HardwareFlowControl = SD_HW_FLOW_CTRL;
  uSdHandle.Init.ClockDiv            = SD_CLK_DIV;

  if(SD_detect_gpio_pin != GPIO_PIN_All) {
    /* Msp SD Detect pin initialization */
    BSP_SD_Detect_MspInit(&uSdHandle, NULL);
    if(BSP_SD_IsDetected() != SD_PRESENT)   /* Check if SD card is present */
    {
      return MSD_ERROR_SD_NOT_PRESENT;
    }
  }

  /* Msp SD initialization */
  BSP_SD_MspInit(&uSdHandle, NULL);

  /* HAL SD initialization */
#ifndef STM32L1xx
  if(HAL_SD_Init(&uSdHandle) != SD_OK)
#else /* STM32L1xx */
  if(HAL_SD_Init(&uSdHandle, &uSdCardInfo) != SD_OK)
#endif
  {
    sd_state = MSD_ERROR;
  }

  /* Configure SD Bus width */
  if(sd_state == MSD_OK)
  {
    /* Enable wide operation */
    if(HAL_SD_WideBusOperation_Config(&uSdHandle, SD_BUS_WIDE_4B) != SD_OK)
    {
      sd_state = MSD_ERROR;
    }
    else
    {
      sd_state = MSD_OK;
    }
  }
  return  sd_state;
}

/**
  * @brief  Set the SD card device detect pin and port.
  * @param  csport one of the gpio port
  * @param  cspin one of the gpio pin
  * @retval SD status
  */
uint8_t BSP_SD_CSSet(GPIO_TypeDef *csport, uint32_t cspin)
{
  if(csport != 0) {
    SD_detect_gpio_pin = cspin;
    SD_detect_gpio_port = csport;
	return MSD_OK;
  }
  return MSD_ERROR;
}

/**
  * @brief  DeInitializes the SD card device.
  * @retval SD status
  */
uint8_t BSP_SD_DeInit(void)
{
  uint8_t sd_state = MSD_OK;

  uSdHandle.Instance = SD_INSTANCE;

  /* HAL SD deinitialization */
  if(HAL_SD_DeInit(&uSdHandle) != HAL_OK)
  {
    sd_state = MSD_ERROR;
  }

  /* Msp SD deinitialization */
  uSdHandle.Instance = SD_INSTANCE;
  BSP_SD_MspDeInit(&uSdHandle, NULL);

  return  sd_state;
}

/**
  * @brief  Configures Interrupt mode for SD detection pin.
  * @retval Returns 0
  */
uint8_t BSP_SD_ITConfig(void)
{
  uint8_t sd_state = MSD_OK;
  GPIO_InitTypeDef gpio_init_structure;
  IRQn_Type sd_detect_EXTI_IRQn = EXTI0_IRQn;

  /* Configure Interrupt mode for SD detection pin */
  gpio_init_structure.Pin = SD_detect_gpio_pin;
  gpio_init_structure.Pull = GPIO_PULLUP;
  gpio_init_structure.Speed = SD_GPIO_SPEED;
  gpio_init_structure.Mode = GPIO_MODE_IT_RISING_FALLING;
  HAL_GPIO_Init(SD_detect_gpio_port, &gpio_init_structure);

  if(SD_detect_gpio_pin == GPIO_PIN_0) {
      sd_detect_EXTI_IRQn = EXTI0_IRQn;
  } else {
    if(SD_detect_gpio_pin == GPIO_PIN_1) {
      sd_detect_EXTI_IRQn = EXTI1_IRQn;
    } else {
      if(SD_detect_gpio_pin == GPIO_PIN_2) {
        sd_detect_EXTI_IRQn = EXTI2_IRQn;
      } else {
        if(SD_detect_gpio_pin == GPIO_PIN_3) {
          sd_detect_EXTI_IRQn = EXTI3_IRQn;
        } else {
          if(SD_detect_gpio_pin == GPIO_PIN_4) {
            sd_detect_EXTI_IRQn = EXTI4_IRQn;
          } else {
            if((SD_detect_gpio_pin == GPIO_PIN_5) ||\
               (SD_detect_gpio_pin == GPIO_PIN_6) ||\
               (SD_detect_gpio_pin == GPIO_PIN_7) ||\
               (SD_detect_gpio_pin == GPIO_PIN_8) ||\
               (SD_detect_gpio_pin == GPIO_PIN_9)) {
              sd_detect_EXTI_IRQn = EXTI9_5_IRQn;
            } else {
              if((SD_detect_gpio_pin == GPIO_PIN_10) ||\
                 (SD_detect_gpio_pin == GPIO_PIN_11) ||\
                 (SD_detect_gpio_pin == GPIO_PIN_12) ||\
                 (SD_detect_gpio_pin == GPIO_PIN_13) ||\
                 (SD_detect_gpio_pin == GPIO_PIN_14) ||\
                 (SD_detect_gpio_pin == GPIO_PIN_15)) {
                sd_detect_EXTI_IRQn = EXTI15_10_IRQn;
              } else {
                sd_state = MSD_ERROR;
              }
            }
          }
        }
      }
    }
  }
  if(sd_state == MSD_OK) {
    /* Enable and set SD detect EXTI Interrupt to the lowest priority */
    HAL_NVIC_SetPriority(sd_detect_EXTI_IRQn, 0x0F, 0x00);
    HAL_NVIC_EnableIRQ(sd_detect_EXTI_IRQn);
  }
  return sd_state;
}

/**
 * @brief  Detects if SD card is correctly plugged in the memory slot or not.
 * @retval Returns if SD is detected or not
 */
uint8_t BSP_SD_IsDetected(void)
{
  uint8_t  status = SD_PRESENT;

  /* Check SD card detect pin */
  if (HAL_GPIO_ReadPin(SD_detect_gpio_port, SD_detect_gpio_pin) == GPIO_PIN_SET)
  {
    status = SD_NOT_PRESENT;
  }

  return status;
}

#ifndef STM32L1xx
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
  if(HAL_SD_ReadBlocks(&uSdHandle, (uint8_t *)pData, ReadAddr, NumOfBlocks, Timeout) != HAL_OK)
  {
    return MSD_ERROR;
  }
  else
  {
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
  if(HAL_SD_WriteBlocks(&uSdHandle, (uint8_t *)pData, WriteAddr, NumOfBlocks, Timeout) != HAL_OK)
  {
    return MSD_ERROR;
  }
  else
  {
    return MSD_OK;
  }
}
#else /* STM32L1xx */
/**
  * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read
  * @param  BlockSize: SD card data block size, that should be 512
  * @param  NumOfBlocks: Number of SD blocks to read
  * @retval SD status
  */
uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumOfBlocks)
{
  if(HAL_SD_ReadBlocks(&uSdHandle, (uint8_t *)pData, ReadAddr, BlockSize, NumOfBlocks) != SD_OK)
  {
    return MSD_ERROR;
  }
  else
  {
    return MSD_OK;
  }
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in polling mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written
  * @param  BlockSize: SD card data block size, that should be 512
  * @param  NumOfBlocks: Number of SD blocks to write
  * @retval SD status
  */
uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumOfBlocks)
{
  if(HAL_SD_WriteBlocks(&uSdHandle, (uint8_t *)pData, WriteAddr, BlockSize, NumOfBlocks) != SD_OK)
  {
    return MSD_ERROR;
  }
  else
  {
    return MSD_OK;
  }
}
#endif /* !STM32L1xx */

/**
  * @brief  Erases the specified memory area of the given SD card.
  * @param  StartAddr: Start byte address
  * @param  EndAddr: End byte address
  * @retval SD status
  */
uint8_t BSP_SD_Erase(uint64_t StartAddr, uint64_t EndAddr)
{
  if(HAL_SD_Erase(&uSdHandle, StartAddr, EndAddr) != SD_OK)
  {
    return MSD_ERROR;
  }
  else
  {
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
  UNUSED(hsd);
  UNUSED(Params);
  GPIO_InitTypeDef gpio_init_structure;

  /* Enable SDIO clock */
  SD_CLK_ENABLE();

  /* Enable GPIOs clock */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* Common GPIO configuration */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = SD_GPIO_SPEED;
#ifndef STM32F1xx
  gpio_init_structure.Alternate = SD_AF;
#endif
  /* GPIOC configuration */
  gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;

  HAL_GPIO_Init(GPIOC, &gpio_init_structure);

  /* GPIOD configuration */
  gpio_init_structure.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOD, &gpio_init_structure);

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
  GPIO_InitTypeDef  gpio_init_structure;

  /* GPIO configuration in input for uSD_Detect signal */
  gpio_init_structure.Pin       = SD_detect_gpio_pin;
  gpio_init_structure.Mode      = GPIO_MODE_INPUT;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = SD_GPIO_SPEED;
  HAL_GPIO_Init(SD_detect_gpio_port, &gpio_init_structure);
}

/**
  * @brief  DeInitializes the SD MSP.
  * @param  hsd: SD handle
  * @param  Params : pointer on additional configuration parameters, can be NULL.
  */
__weak void BSP_SD_MspDeInit(SD_HandleTypeDef *hsd, void *Params)
{
    UNUSED(hsd);
    UNUSED(Params);

    /* DeInit GPIO pins can be done in the application
       (by surcharging this __weak function) */

    /* Disable SDIO clock */
    SD_CLK_DISABLE();

    /* GPOI pins clock and DMA cloks can be shut down in the applic
       by surcgarging this __weak function */
}

#ifndef STM32L1xx
/**
  * @brief  Gets the current SD card data status.
  * @retval Data transfer state.
  *          This value can be one of the following values:
  *            @arg  SD_TRANSFER_OK: No data transfer is acting
  *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
  */
uint8_t BSP_SD_GetCardState(void)
{
  return((HAL_SD_GetCardState(&uSdHandle) == HAL_SD_CARD_TRANSFER ) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}
#else /* STM32L1xx */
/**
  * @brief  Gets the current SD card data status.
  * @retval Data transfer state.
  *          This value can be one of the following values:
  *            @arg  SD_TRANSFER_OK: No data transfer is acting
  *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
  *            @arg  SD_TRANSFER_ERROR: Data transfer error
  */
HAL_SD_TransferStateTypedef BSP_SD_GetStatus(void)
{
  return(HAL_SD_GetStatus(&uSdHandle));
}
#endif

/**
  * @brief  Get SD information about specific SD card.
  * @param  CardInfo: Pointer to HAL_SD_CardInfoTypedef structure
  */
void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo)
{
  /* Get SD card Information */
  HAL_SD_Get_CardInfo(&uSdHandle, CardInfo);
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
