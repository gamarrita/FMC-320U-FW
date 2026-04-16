/**
 * @file    fm_port_usart1.h
 * @brief   Platform USART1 helpers for debug transport.
 */

#ifndef FM_PORT_USART1_H
#define FM_PORT_USART1_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   Configure USART1 for the board debug channel.
 *
 * @details
 *  - This is a technical backend API for the MCU USART1 instance.
 *  - On the current hardware, USART1 feeds the ST-Link Virtual COM Port seen
 *    by the host PC.
 *  - The board-level meaning of that path is exposed by
 *    FM_BOARD_DebugUartTransmit().
 */
void FM_PORT_Usart1_Init(void);

/**
 * @brief   Transmit a debug buffer over the configured USART1 backend.
 *
 * @details
 *  - This function performs the technical USART1 transfer only.
 *  - It does not define board policy or debug enable rules.
 *  - On the current board, USART1 is the backend behind the ST-Link Virtual
 *    COM Port used for debug messages.
 *
 * @param   p_data      Buffer to transmit.
 * @param   len         Number of bytes to send.
 * @param   timeout_ms  HAL transmit timeout in milliseconds.
 *
 * @return  true on successful transmit, false on invalid input or HAL failure.
 */
bool FM_PORT_Usart1_Transmit(const uint8_t *p_data, uint32_t len, uint32_t timeout_ms);

#endif /* FM_PORT_USART1_H */
