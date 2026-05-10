/**
 * @file    fm_board.h
 * @brief   Public board-level API for shared board services.
 */
#ifndef FM_BOARD_H
#define FM_BOARD_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Initialize the shared board baseline for normal apps.
 *
 * @note Call once before using board services from this module.
 */
void FM_BOARD_Init(void);

/** @brief Sample jumper or config that enables UART debug messages. */
bool FM_BOARD_DebugMsgEnabled(void);

/**
 * @brief Sample jumper or config that enables debug LEDs.
 *
 * @note Returns the current enable state and turns board debug LEDs off when
 *       that state is disabled.
 */
bool FM_BOARD_DebugLedsEnabled(void);

/** @brief Turn on the error indicator LED. */
void FM_BOARD_LedErrorOn(void);
/** @brief Turn off the error indicator LED. */
void FM_BOARD_LedErrorOff(void);

/** @brief Turn on the run/status LED. */
void FM_BOARD_LedRunOn(void);
/** @brief Turn off the run/status LED. */
void FM_BOARD_LedRunOff(void);

/** @brief Turn on the signal/activity LED. */
void FM_BOARD_LedSignalOn(void);
/** @brief Turn off the signal/activity LED. */
void FM_BOARD_LedSignalOff(void);

/**
 * @brief Transmit a buffer through the board debug UART channel.
 *
 * @param p_data      Buffer to transmit.
 * @param len         Number of bytes to send.
 * @param timeout_ms  Blocking transmit timeout in milliseconds.
 *
 * @return true on successful transmit, false on invalid input or backend failure.
 *
 * @warning Blocking call. Foreground only. Not IRQ-safe.
 */
bool FM_BOARD_DebugUartTransmit(const uint8_t *p_data, uint32_t len, uint32_t timeout_ms);

/** @brief Initialize the DWT cycle counter if supported on this target. */
bool FM_BOARD_DwtInit(void);

/** @brief Read the current DWT cycle count, or 0 when unavailable. */
uint32_t FM_BOARD_DwtGetCycles(void);

/**
 * @brief Handle a board-level RTC wakeup IRQ event.
 *
 * @warning IRQ context. Keep this path non-blocking.
 */
void FM_BOARD_OnRtcWakeupIrq(void);

#endif /* FM_BOARD_H */

