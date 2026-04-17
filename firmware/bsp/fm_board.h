/**
 * @file    fm_board.h
 * @brief   Public board-level API for shared board services.
 *
 * @details
 *  - Facade for shared board bring-up and board-facing runtime helpers.
 *  - Applications use this surface; low-level hardware lives in fm_port_* modules.
 *  - FM_BOARD_Init owns the common baseline sequence for normal apps.
 */
#ifndef FM_BOARD_H
#define FM_BOARD_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Initialize the common board baseline for normal apps.
 *
 * @details
 *  - This owns the shared port-layer bring-up sequence used across apps.
 *  - The current baseline includes debug GPIO, USART1, SPI1, PCF8553 control,
 *    and DWT support.
 *  - Device-specific functional initialization remains outside this API.
 */
void FM_BOARD_Init(void);

/** @brief Sample jumper or config that enables UART debug messages. */
bool FM_BOARD_DebugMsgEnabled(void);

/** @brief Sample jumper or config that enables debug LEDs. */
bool FM_BOARD_DebugLedsEnabled(void);

/** @brief Drive the error indicator LED. */
void FM_BOARD_LedErrorOn(void);
void FM_BOARD_LedErrorOff(void);

/** @brief Drive the run/status LED. */
void FM_BOARD_LedRunOn(void);
void FM_BOARD_LedRunOff(void);

/** @brief Drive the signal/activity LED. */
void FM_BOARD_LedSignalOn(void);
void FM_BOARD_LedSignalOff(void);

/**
 * @brief Blocking transmit through the board debug UART channel.
 *
 * @details
 *  - This API represents the board-level debug output path.
 *  - On this board, the debug UART is exposed to the host PC through the
 *    ST-Link Virtual COM Port (VCP).
 *  - Callers use this semantic board resource; the concrete UART peripheral
 *    and HAL transport live in the port layer.
 *
 * @param p_data      Buffer to transmit.
 * @param len         Number of bytes to send.
 * @param timeout_ms  Blocking transmit timeout in milliseconds.
 *
 * @return true on successful transmit, false on invalid input or backend failure.
 */
bool FM_BOARD_DebugUartTransmit(const uint8_t *p_data, uint32_t len, uint32_t timeout_ms);

/** @brief Initialize the DWT cycle counter if present. */
bool FM_BOARD_DwtInit(void);

/** @brief Read the current DWT cycle count (0 if unavailable). */
uint32_t FM_BOARD_DwtGetCycles(void);

/** @brief Handle a board-level RTC wakeup IRQ event. */
void FM_BOARD_OnRtcWakeupIrq(void);

#endif /* FM_BOARD_H */

