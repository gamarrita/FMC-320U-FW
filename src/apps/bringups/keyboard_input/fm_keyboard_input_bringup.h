/**
 * @file    fm_keyboard_input_bringup.h
 * @brief   Keyboard GPIO EXTI bring-up contract.
 *
 * This app initializes board/debug services and the board keyboard EXTI bridge.
 * It validates short-press release observation by emitting ISR-queued debug
 * messages for configured keys.
 */
#ifndef FM_KEYBOARD_INPUT_BRINGUP_H
#define FM_KEYBOARD_INPUT_BRINGUP_H

/**
 * @brief Run the human-observed keyboard short-press bring-up.
 *
 * Expected human setup: enable debug UART messages, open the ST-LINK VCP at
 * 115200 8N1, and press/release DOWN, UP, ENTER, and ESC. The app reports
 * readiness over UART and then repeatedly flushes deferred key events while
 * blinking the run LED.
 *
 * Fatal UART startup failures enter `FM_DEBUG_PanicMsg()`.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void FM_KeyboardInputBringup_Run(void);

#endif /* FM_KEYBOARD_INPUT_BRINGUP_H */
