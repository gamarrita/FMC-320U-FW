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
    /** Number of semantic board keys; not a valid key. */
    FM_BOARD_KEYBOARD_KEY_COUNT
} fm_board_keyboard_key_t;

typedef enum
{
    /** Board keyboard rising edge as reported by the port EXTI adapter. */
    FM_BOARD_KEYBOARD_EDGE_RISING = 0,
    /** Board keyboard falling edge as reported by the port EXTI adapter. */
    FM_BOARD_KEYBOARD_EDGE_FALLING
} fm_board_keyboard_edge_t;

/**
 * @brief Board keyboard event callback.
 *
 * @param key  Board keyboard key identity.
 * @param edge Board keyboard edge observed for that key.
 *
 * @warning Runs in IRQ context. Keep work bounded and non-blocking.
 */
typedef void (*fm_board_keyboard_callback_t)(
    fm_board_keyboard_key_t key,
    fm_board_keyboard_edge_t edge);

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
 * The callback receives board-level key and edge values after GPIO pin mapping.
 * GPIO, HAL, and EXTI details remain hidden inside the board/port layers.
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

#endif /* FM_BOARD_KEYBOARD_H */
