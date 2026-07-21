/**
 * @file    fm_main.h
 * @brief   Reduced FMC product app entry under ThreadX.
 *
 * @details
 *   This module wires board services to the RTOS-neutral `fmc_runtime`
 *   contract. It owns the app-level ThreadX queue used to serialize keyboard
 *   IRQ delivery and the dedicated runtime owner thread that dispatches
 *   provisional mechanical-key `SHORT` events.
 */

#ifndef FM_MAIN_H_
#define FM_MAIN_H_

/**
 * @brief Initialize the reduced product app runtime wiring.
 *
 * Configures board, RTC, debug, the keyboard delivery queue, the dedicated
 * runtime owner thread, and the board keyboard IRQ callback.
 *
 * @warning Foreground startup only.
 * @warning Call once after ThreadX has started.
 * @warning Initialization failures are fatal contract violations.
 */
void FM_MAIN_Init(void);

/**
 * @brief Run the reduced product app supervisor loop.
 *
 * Calls `FM_MAIN_Init()` once, then periodically flushes deferred debug events
 * while the dedicated runtime owner thread blocks on keyboard input.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void FM_MAIN_Main(void);

#endif /* FM_MAIN_H_ */

/*** end of file ***/
