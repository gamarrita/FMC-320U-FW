/**
 * @file    fm_board_keyboard.h
 * @brief   Board keyboard contract.
 *
 * This module names the physical keyboard fitted to the FMC-320U board and
 * hides the CubeMX GPIO pin labels from higher layers. It does not define menu
 * actions, long-press policy, debounce policy, or any RTOS delivery mechanism.
 */
#ifndef FM_BOARD_KEYBOARD_H
#define FM_BOARD_KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    FM_BOARD_KEYBOARD_KEY_DOWN = 0,
    FM_BOARD_KEYBOARD_KEY_UP,
    FM_BOARD_KEYBOARD_KEY_ENTER,
    FM_BOARD_KEYBOARD_KEY_ESC,
    FM_BOARD_KEYBOARD_KEY_COUNT
} fm_board_keyboard_key_t;

/**
 * @brief Initialize the board keyboard interrupt path.
 *
 * @details Registers the keyboard as the current GPIO EXTI consumer and
 *          initializes the CubeMX-generated GPIO/EXTI configuration.
 */
void FM_BOARD_KeyboardInit(void);

/**
 * @brief Map a CubeMX GPIO pin value to a board keyboard key.
 *
 * @param gpio_pin CubeMX/HAL GPIO pin mask, such as KEY_DOWN_Pin.
 * @param p_key    Destination for the mapped semantic key.
 *
 * @return true when the pin belongs to the board keyboard, false otherwise.
 */
bool FM_BOARD_KeyboardKeyFromGpioPin(uint16_t gpio_pin,
                                     fm_board_keyboard_key_t *p_key);

#endif /* FM_BOARD_KEYBOARD_H */
