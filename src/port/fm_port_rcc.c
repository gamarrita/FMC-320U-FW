/**
 * @file    fm_port_rcc.c
 * @brief   Platform RCC helpers for system clock configuration.
 */

#include "fm_port_rcc.h"

#include "main.h"

void FM_PORT_RCC_Init(void)
{
    RCC_OscInitTypeDef osc_init = { 0 };
    RCC_ClkInitTypeDef clk_init = { 0 };

    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE4) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    osc_init.OscillatorType = RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;
    osc_init.LSEState = RCC_LSE_ON;
    osc_init.MSIState = RCC_MSI_ON;
    osc_init.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    osc_init.MSIClockRange = RCC_MSIRANGE_1;
    osc_init.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&osc_init) != HAL_OK)
    {
        Error_Handler();
    }

    clk_init.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
            | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK3;
    clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk_init.APB1CLKDivider = RCC_HCLK_DIV1;
    clk_init.APB2CLKDivider = RCC_HCLK_DIV1;
    clk_init.APB3CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&clk_init, FLASH_LATENCY_1) != HAL_OK)
    {
        Error_Handler();
    }
}
