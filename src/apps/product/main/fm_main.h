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
 *   Runtime dispatch runs only from the `FM_MAIN_Main()` owner loop after the
 *   app-level short/long recognizer accepts an input action.
 *
 *   A ThreadX periodic timer also publishes `FM_MAIN_EVENT_PERIODIC_REFRESH`
 *   once per second into the same owner queue. The refresh source provides a
 *   regular low-power wake deadline and a future measurement/presentation
 *   update cadence.
 *
 *   A separate one-shot ThreadX timer supports mechanical-key hold recognition:
 *   RISING starts the hold, timeout emits one `LONG`, and FALLING emits
 *   `SHORT` only when no `LONG` was already emitted.
 */

#ifndef FM_MAIN_H_
#define FM_MAIN_H_

/**
 * @brief Initialize the reduced product app runtime wiring.
 *
 * Configures board, RTC, debug, the owner event queue, the 1 second periodic
 * refresh timer, and the one-shot key-hold timer.
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
 * the keyboard IRQ publisher, then waits on the app-level event queue. Keyboard
 * events pass through the app-level short/long recognizer before dispatching
 * semantic input to runtime. Periodic refresh events currently run a no-op
 * placeholder reserved for later refresh work. This function does not create
 * another ThreadX thread.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void FM_MAIN_Main(void);

#endif /* FM_MAIN_H_ */

/*** end of file ***/
