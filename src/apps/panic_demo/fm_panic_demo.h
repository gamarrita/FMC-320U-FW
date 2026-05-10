#ifndef FM_PANIC_DEMO_H_
#define FM_PANIC_DEMO_H_

/**
 * @brief Run the panic demonstration application.
 *
 * @details
 *  - This app is intentionally didactic.
 *  - It selects one controlled failure path and then stops inside the shared
 *    panic infrastructure from fm_debug.
 *  - The goal is to let humans and agents correlate UART messages, LEDs, and
 *    debugger state with a known failure source.
 */
void FM_PanicDemo_Run(void);

#endif /* FM_PANIC_DEMO_H_ */
