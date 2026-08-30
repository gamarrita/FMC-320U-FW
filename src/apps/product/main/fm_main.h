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
 *   Semantic UI input runs only from the `FM_MAIN_Main()` owner loop after the
 *   applicable recognizer accepts an input action. Measurement runtime does
 *   not retain menu input.
 *
 *   A ThreadX periodic timer publishes `FM_MAIN_EVENT_PERIODIC_REFRESH` once
 *   per second into the same owner queue. The owner performs independent
 *   pulse-delta and pulse/time observations, dispatches their runtime events,
 *   emits optional compact live evidence, and refreshes the stable TTL/RATE
 *   view from canonical runtime state.
 *
 *   A separate one-shot timer owns the nominal three-second dwell of each
 *   temporary startup presentation. It starts only after successful LCD
 *   presentation and publishes its timeout through the same owner queue.
 *
 *   A separate one-shot ThreadX timer supports mechanical-key hold recognition:
 *   PRESSED starts the hold, timeout emits one `LONG`, and RELEASED emits
 *   `SHORT` only when no `LONG` was already emitted. Two further one-shot
 *   timers independently rearm EXT_1 and EXT_2 after 100 ms released.
 *
 *   A further one-shot timer owns the ten-second backlight interval. Startup
 *   and every physical PRESSED transition request activation before semantic
 *   filtering. The owner commits and restarts the interval before turning the
 *   board output on; the timer callback applies current expiry directly.
 */

#ifndef FM_MAIN_H_
#define FM_MAIN_H_

/**
 * @brief Initialize the reduced product app runtime wiring.
 *
 * Configures board, RTC, debug, the owner event queue, the 1 second periodic
 * refresh timer, the presentation dwell timer, the key-hold timer, both
 * external-button stable-release timers, and the backlight one-shot timer.
 *
 * @warning Foreground startup only.
 * @warning Call once after ThreadX has started.
 * @warning Initialization failures are fatal contract violations.
 */
void FM_MAIN_Init(void);

/**
 * @brief Run the reduced product runtime owner loop.
 *
 * Calls `FM_MAIN_Init()` once, initializes the local runtime and acquisition
 * observers, starts the configured frequency-time and pulse counters,
 * establishes the frequency baseline, registers the keyboard IRQ publisher,
 * then waits on the app-level event queue. Keyboard events pass
 * through the applicable app-level recognizer before routing semantic input
 * directly to UI. Periodic events perform pulse totalization, update RATE, and
 * refresh whichever user-menu state is active from a live runtime snapshot.
 * Physical press activity independently restarts the backlight interval even
 * when the corresponding semantic input is rejected or is a UI no-op. This
 * function does not create another ThreadX thread.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void FM_MAIN_Main(void);

#endif /* FM_MAIN_H_ */

/*** end of file ***/
