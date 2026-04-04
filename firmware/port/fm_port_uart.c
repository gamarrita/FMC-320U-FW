/**
 * @file    fm_port_uart.c
 * @brief   Platform UART helpers for debug transport.
 */

#include <fm_port_uart.h>
#include "main.h"

static UART_HandleTypeDef huart1;

/* Internal accessor for the configured UART handle. */
static UART_HandleTypeDef *fm_port_uart_handle_get_(void)
{
    return &huart1;
}

void FM_PORT_UART_Init(void)
{
    UART_HandleTypeDef *huart = fm_port_uart_handle_get_();

    huart->Instance = USART1;
    huart->Init.BaudRate = 115200;
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
    huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart->Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(huart) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_UARTEx_SetTxFifoThreshold(huart, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_UARTEx_SetRxFifoThreshold(huart, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_UARTEx_DisableFifoMode(huart) != HAL_OK)
    {
        Error_Handler();
    }
}

bool FM_PORT_UART_Transmit(const uint8_t *p_data, uint32_t len, uint32_t timeout_ms)
{
    if ((p_data == NULL) || (len == 0U))
    {
        return false;
    }

    return (HAL_UART_Transmit(fm_port_uart_handle_get_(), (uint8_t *) p_data, len, timeout_ms) == HAL_OK);
}
