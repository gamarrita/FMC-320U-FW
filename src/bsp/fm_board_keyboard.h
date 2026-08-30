/**
 * @file    fm_board_keyboard.h
 * @brief   Board keyboard contract.
 *
 * This module names the physical keyboard fitted to the FMC-320U board and
 * hides the CubeMX GPIO pin labels from higher layers. It does not define menu
 * actions, long-press policy, debounce policy, or any RTOS delivery mechanism.
 *
 * The current interrupt bridge logs observed key edges through `fm_debug` for
 * bring-up. Consumers that need product input semantics should build on the
 * semantic key mapping instead of depending on debug log strings.
 */
#ifndef FM_BOARD_KEYBOARD_H
#define FM_BOARD_KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    /** Physical DOWN key. */
    FM_BOARD_KEYBOARD_KEY_DOWN = 0,
    /** Physical UP key. */
    FM_BOARD_KEYBOARD_KEY_UP,
    /** Physical ENTER key. */
    FM_BOARD_KEYBOARD_KEY_ENTER,
    /** Physical ESC key. */
    FM_BOARD_KEYBOARD_KEY_ESC,
    /** Active-low external button 1. */
    FM_BOARD_KEYBOARD_KEY_EXT_1,
    /** Active-low external button 2. */
    FM_BOARD_KEYBOARD_KEY_EXT_2,
    /** Number of semantic board keys; not a valid key. */
    FM_BOARD_KEYBOARD_KEY_COUNT
} fm_board_keyboard_key_t;

typedef enum
{
    /** Physical control has entered its pressed state. */
    FM_BOARD_KEYBOARD_TRANSITION_PRESSED = 0,
    /** Physical control has entered its released state. */
    FM_BOARD_KEYBOARD_TRANSITION_RELEASED
} fm_board_keyboard_transition_t;

/**
 * @brief Board keyboard event callback.
 *
 * @param key  Board keyboard key identity.
 * @param transition Physical pressed/released transition after board polarity
 *                   translation.
 *
 * @warning Runs in IRQ context. Keep work bounded and non-blocking.
 */
typedef void (*fm_board_keyboard_callback_t)(
    fm_board_keyboard_key_t key,
    fm_board_keyboard_transition_t transition);

/**
 * @brief Initialize the board keyboard interrupt path.
 *
 * @details Registers the keyboard as the current GPIO EXTI consumer and
 *          initializes the CubeMX-generated GPIO/EXTI configuration.
 *
 * The registered EXTI callback runs in IRQ context and keeps work bounded by
 * using ISR-safe debug logging and the optional board keyboard callback.
 */
void FM_BOARD_KeyboardInit(void);

/**
 * @brief Register the board keyboard event callback.
 *
 * The callback receives board-level key and pressed/released values after GPIO
 * pin and active-polarity mapping. GPIO, HAL, and EXTI details remain hidden
 * inside the board/port layers.
 *
 * @param p_callback Callback to run from the keyboard IRQ path, or `NULL` to
 *        unregister.
 *
 * @warning The callback runs in IRQ context.
 */
void FM_BOARD_KeyboardSetCallback(
    fm_board_keyboard_callback_t p_callback);

/**
 * @brief Map a CubeMX GPIO pin value to a board keyboard key.
 *
 * @param gpio_pin CubeMX/HAL GPIO pin mask, such as KEY_DOWN_Pin.
 * @param p_key    Caller-owned destination for the mapped semantic key.
 *
 * @return true when the pin belongs to the board keyboard, false otherwise.
 * @return false when `p_key` is `NULL`; in that case no output is written.
 */
bool FM_BOARD_KeyboardKeyFromGpioPin(uint16_t gpio_pin,
                                     fm_board_keyboard_key_t *p_key);

/**
 * @brief Sample whether one physical keyboard control is currently pressed.
 *
 * Active-high mechanical keys and active-low external buttons are normalized
 * to the same board-level pressed state.
 *
 * @param key Board keyboard identity to sample.
 * @param p_pressed Caller-owned pressed-state destination.
 *
 * @return `true` when @p key is valid and @p p_pressed was updated.
 * @return `false` for an invalid key or `NULL` destination.
 *
 * @warning Foreground use only. The GPIO path must be initialized first.
 */
bool FM_BOARD_KeyboardIsPressed(fm_board_keyboard_key_t key,
                                bool *p_pressed);

#endif /* FM_BOARD_KEYBOARD_H */
