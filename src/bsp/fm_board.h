/**
 * @file    fm_board.h
 * @brief   Public board-level API for shared board services.
 *
 * This module is the board-facing facade used by apps and product code. It
 * owns common board bring-up sequencing over port-layer services, debug jumper
 * policy, debug LEDs, the board debug UART path, DWT timestamp access, and
 * board-level RTC wakeup forwarding.
 *
 * `FM_BOARD_Init()` must be called before using the board services that touch
 * GPIO, USART, SPI, PCF8553 control lines, backlight, or DWT. Functions do not
 * provide internal synchronization.
 */
#ifndef FM_BOARD_H
#define FM_BOARD_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Initialize the shared board baseline for normal apps.
 *
 * Configures clocks, debug GPIO, USART1 debug transport, SPI1 device
 * transport, PCF8553 control GPIO, a safely-off LCD backlight, and DWT cycle
 * counting.
 *
 * This function may call lower-layer initialization paths that stop in
 * `Error_Handler()` on unrecoverable HAL failures.
 *
 * @note Call once from foreground startup before using board services.
 */
void FM_BOARD_Init(void);

/**
 * @brief Sample the board policy that enables UART debug messages.
 *
 * @return `true` when debug UART messages are enabled.
 * @return `false` otherwise.
 */
bool FM_BOARD_DebugMsgEnabled(void);

/**
 * @brief Sample jumper or config that enables debug LEDs.
 *
 * @note Returns the current enable state and turns board debug LEDs off when
 *       that state is disabled.
 *
 * @return `true` when debug LEDs are enabled.
 * @return `false` otherwise.
 */
bool FM_BOARD_DebugLedsEnabled(void);

/** @brief Turn on the error indicator LED. Foreground use only. */
void FM_BOARD_LedErrorOn(void);
/** @brief Turn off the error indicator LED. Foreground use only. */
void FM_BOARD_LedErrorOff(void);

/** @brief Turn on the run/status LED. Foreground use only. */
void FM_BOARD_LedRunOn(void);
/** @brief Turn off the run/status LED. Foreground use only. */
void FM_BOARD_LedRunOff(void);

/** @brief Turn on the signal/activity LED. Foreground use only. */
void FM_BOARD_LedSignalOn(void);
/** @brief Turn off the signal/activity LED. Foreground use only. */
void FM_BOARD_LedSignalOff(void);

/** @brief Turn on the LCD backlight. Foreground use only. */
void FM_BOARD_BacklightOn(void);

/** @brief Turn off the LCD backlight. Foreground use only. */
void FM_BOARD_BacklightOff(void);

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

/**
 * @brief Initialize the DWT cycle counter if supported on this target.
 *
 * @return `true` when DWT cycle counting is available and enabled.
 * @return `false` otherwise.
 */
bool FM_BOARD_DwtInit(void);

/**
 * @brief Read the current DWT cycle count.
 *
 * @return Current cycle count.
 * @return 0 when cycle counting is unavailable.
 */
uint32_t FM_BOARD_DwtGetCycles(void);

/**
 * @brief Handle a board-level RTC wakeup IRQ event.
 *
 * @warning IRQ context. Keep this path non-blocking.
 */
void FM_BOARD_OnRtcWakeupIrq(void);

#endif /* FM_BOARD_H */

