/**
 * @file    fm_debug_panic_bringup.h
 * @brief   Debug panic and fault path bring-up contract.
 *
 * This app initializes board/debug services, emits a UART banner, then triggers
 * one compile-time-selected fatal path. It is for calibrating UART, LED, and
 * debugger behavior around known failures.
 */
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
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void FM_DebugPanicBringup_Run(void);

#endif /* FM_DEBUG_PANIC_BRINGUP_H_ */
