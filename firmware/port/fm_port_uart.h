/**
 * @file    fm_port_uart.h
 * @brief   Platform UART helpers for debug transport.
 */

#ifndef FM_PORT_UART_H
#define FM_PORT_UART_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   Configure the debug UART peripheral.
 */
void FM_PORT_UART_Init(void);

/**
 * @brief   Transmit a debug buffer over the configured UART.
 *
 * @param   p_data      Buffer to transmit.
 * @param   len         Number of bytes to send.
 * @param   timeout_ms  HAL transmit timeout in milliseconds.
 *
 * @return  true on successful transmit, false on invalid input or HAL failure.
 */
bool FM_PORT_UART_Transmit(const uint8_t *p_data, uint32_t len, uint32_t timeout_ms);

#endif /* FM_PORT_UART_H */
