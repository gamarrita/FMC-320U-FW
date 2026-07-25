/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    lptim.c
  * @brief   This file provides code for the configuration
  *          of the LPTIM instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "lptim.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

LPTIM_HandleTypeDef hlptim1;
LPTIM_HandleTypeDef hlptim4;

/* LPTIM1 init function */
void MX_LPTIM1_Init(void)
{

  /* USER CODE BEGIN LPTIM1_Init 0 */

  /* USER CODE END LPTIM1_Init 0 */

  /* USER CODE BEGIN LPTIM1_Init 1 */

  /* USER CODE END LPTIM1_Init 1 */
  hlptim1.Instance = LPTIM1;
  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV16;
  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim1.Init.Period = 65535;
  hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  hlptim1.Init.RepetitionCounter = 0;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPTIM1_Init 2 */

  /* USER CODE END LPTIM1_Init 2 */

}
/* LPTIM4 init function */
void MX_LPTIM4_Init(void)
{

  /* USER CODE BEGIN LPTIM4_Init 0 */

  /* USER CODE END LPTIM4_Init 0 */

  /* USER CODE BEGIN LPTIM4_Init 1 */

  /* USER CODE END LPTIM4_Init 1 */
  hlptim4.Instance = LPTIM4;
  hlptim4.Init.Clock.Source = LPTIM_CLOCKSOURCE_ULPTIM;
  hlptim4.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
  hlptim4.Init.UltraLowPowerClock.Polarity = LPTIM_CLOCKPOLARITY_FALLING;
  hlptim4.Init.UltraLowPowerClock.SampleTime = LPTIM_CLOCKSAMPLETIME_DIRECTTRANSITION;
  hlptim4.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim4.Init.Period = 65535;
  hlptim4.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim4.Init.CounterSource = LPTIM_COUNTERSOURCE_EXTERNAL;
  hlptim4.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim4.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  hlptim4.Init.RepetitionCounter = 0;
  if (HAL_LPTIM_Init(&hlptim4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPTIM4_Init 2 */

  /* USER CODE END LPTIM4_Init 2 */

}

void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* lptimHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(lptimHandle->Instance==LPTIM1)
  {
  /* USER CODE BEGIN LPTIM1_MspInit 0 */

  /* USER CODE END LPTIM1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
    PeriphClkInit.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* LPTIM1 clock enable */
    __HAL_RCC_LPTIM1_CLK_ENABLE();

    /* LPTIM1 interrupt Init */
    HAL_NVIC_SetPriority(LPTIM1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
  /* USER CODE BEGIN LPTIM1_MspInit 1 */

  /* USER CODE END LPTIM1_MspInit 1 */
  }
  else if(lptimHandle->Instance==LPTIM4)
  {
  /* USER CODE BEGIN LPTIM4_MspInit 0 */

  /* USER CODE END LPTIM4_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPTIM34;
    PeriphClkInit.Lptim34ClockSelection = RCC_LPTIM34CLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* LPTIM4 clock enable */
    __HAL_RCC_LPTIM4_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**LPTIM4 GPIO Configuration
    PD13     ------> LPTIM4_IN1
    */
    GPIO_InitStruct.Pin = FMC_PRIMARY_PULSE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_LPTIM4;
    HAL_GPIO_Init(FMC_PRIMARY_PULSE_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN LPTIM4_MspInit 1 */

  /* USER CODE END LPTIM4_MspInit 1 */
  }
}

void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* lptimHandle)
{

  if(lptimHandle->Instance==LPTIM1)
  {
  /* USER CODE BEGIN LPTIM1_MspDeInit 0 */

  /* USER CODE END LPTIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LPTIM1_CLK_DISABLE();

    /* LPTIM1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(LPTIM1_IRQn);
  /* USER CODE BEGIN LPTIM1_MspDeInit 1 */

  /* USER CODE END LPTIM1_MspDeInit 1 */
  }
  else if(lptimHandle->Instance==LPTIM4)
  {
  /* USER CODE BEGIN LPTIM4_MspDeInit 0 */

  /* USER CODE END LPTIM4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LPTIM4_CLK_DISABLE();

    /**LPTIM4 GPIO Configuration
    PD13     ------> LPTIM4_IN1
    */
    HAL_GPIO_DeInit(FMC_PRIMARY_PULSE_GPIO_Port, FMC_PRIMARY_PULSE_Pin);

  /* USER CODE BEGIN LPTIM4_MspDeInit 1 */

  /* USER CODE END LPTIM4_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

