/**
 * @file    fm_port_usart1.c
 * @brief   Platform USART1 helpers for debug transport.
 */

#include <fm_port_usart1.h>
#include "main.h"

static UART_HandleTypeDef huart1;

/* Internal accessor for the configured USART1 handle. */
static UART_HandleTypeDef *fm_port_usart1_handle_get_(void)
{
    return &huart1;
}

/**
 * @brief Initialize the USART1 interface used by the board debug channel.
 *
 * @details
 *  - Configures USART1 as the backend transport for the board debug UART.
 *  - On this board, that path is routed to the ST-Link Virtual COM Port (VCP)
 *    presented to the host PC.
 *  - Hardware meaning stays in the board layer; this function only applies the
 *    concrete peripheral configuration.
 */
void FM_PORT_Usart1_Init(void)
{
    UART_HandleTypeDef *huart = fm_port_usart1_handle_get_();

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

/**
 * @brief Transmit data over the configured USART1 debug backend.
 *
 * @details
 *  - Executes a blocking HAL UART transmit on the configured USART1 instance.
 *  - This is the technical transport behind the board debug UART channel.
 *  - Higher-level meaning such as debug output over ST-Link VCP belongs to
 *    the board layer API that calls this backend.
 *
 * @param p_data      Buffer to transmit.
 * @param len         Number of bytes to send.
 * @param timeout_ms  HAL transmit timeout in milliseconds.
 *
 * @return true on successful transmit, false on invalid input or HAL failure.
 */
bool FM_PORT_Usart1_Transmit(const uint8_t *p_data, uint32_t len, uint32_t timeout_ms)
{
    if ((p_data == NULL) || (len == 0U))
    {
        return false;
    }

    return (HAL_UART_Transmit(fm_port_usart1_handle_get_(), (uint8_t *) p_data, len, timeout_ms) == HAL_OK);
}
