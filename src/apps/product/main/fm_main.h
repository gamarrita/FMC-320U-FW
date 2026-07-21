/**
 * @file    fm_main.h
 * @brief   Reduced FMC product app entry under ThreadX.
 *
 * @details
 *   This module wires board services to the RTOS-neutral `fmc_runtime`
 *   contract. It owns the app-level ThreadX queue used to serialize keyboard
 *   IRQ delivery. The existing `FM_APP` ThreadX thread executes
 *   `FM_MAIN_Main()` and is the only owner of the live `fmc_runtime_t`.
 *
 *   Keyboard ISRs only publish board-level events into the private queue.
 *   Runtime dispatch runs only from the `FM_MAIN_Main()` owner loop.
 */

#ifndef FM_MAIN_H_
#define FM_MAIN_H_

/**
 * @brief Initialize the reduced product app runtime wiring.
 *
 * Configures board, RTC, debug, and the keyboard delivery queue.
 *
 * @warning Foreground startup only.
 * @warning Call once after ThreadX has started.
 * @warning Initialization failures are fatal contract violations.
 */
void FM_MAIN_Init(void);

/**
 * @brief Run the reduced product runtime owner loop.
 *
 * Calls `FM_MAIN_Init()` once, initializes the local `fmc_runtime_t`, registers
 * the keyboard IRQ publisher, then waits on the keyboard queue and dispatches
 * accepted semantic input events. This function does not create another
 * ThreadX thread.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void FM_MAIN_Main(void);

#endif /* FM_MAIN_H_ */

/*** end of file ***/
