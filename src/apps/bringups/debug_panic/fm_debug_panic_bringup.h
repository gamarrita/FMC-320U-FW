#ifndef FM_DEBUG_PANIC_BRINGUP_H_
#define FM_DEBUG_PANIC_BRINGUP_H_

/**
 * @brief Run the panic bring-up application.
 *
 * @details
 *  - This app is intentionally controlled and deterministic.
 *  - It selects one controlled failure path and then stops inside the shared
 *    panic infrastructure from fm_debug.
 *  - The goal is to let humans and agents correlate UART messages, LEDs, and
 *    debugger state with a known failure source.
 */
void FM_DebugPanicBringup_Run(void);

#endif /* FM_DEBUG_PANIC_BRINGUP_H_ */
