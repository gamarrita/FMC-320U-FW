/**
 * @file    fm_main_input_adapter.h
 * @brief   Product app adapter from board keyboard keys to FMC semantic input.
 *
 * This module belongs to the product app composition layer. It may include both
 * board-facing keyboard contracts and FMC input contracts, but it does not
 * expose GPIO, EXTI, HAL, ThreadX, queue, timer, debounce, wake, backlight, or
 * menu consequences.
 *
 * It can populate semantic input events with a caller-selected action;
 * recognizing when an action is SHORT or LONG belongs to the applicable
 * product-main recognizer.
 */
#ifndef FM_MAIN_INPUT_ADAPTER_H
#define FM_MAIN_INPUT_ADAPTER_H

#include <stdbool.h>

#include "fm_board_keyboard.h"
#include "fmc_input.h"

/**
 * @brief Translate one board key/action into a semantic input event.
 *
 * Invalid keys, invalid key/action combinations, and `NULL` outputs are
 * rejected without writing `p_input`.
 *
 * @param board_key Mechanical board key identity.
 * @param action Semantic input action selected by the caller.
 * @param p_input Caller-owned semantic input destination.
 *
 * @return `true` when a semantic input was produced.
 * @return `false` when any input is invalid.
 */
bool FM_MAIN_INPUT_ADAPTER_InputFromBoardKey(
    fm_board_keyboard_key_t board_key,
    fmc_input_action_t action,
    fmc_input_event_t *p_input);

/**
 * @brief Translate one board key into a semantic SHORT input event.
 *
 * Invalid keys and `NULL` outputs are rejected without writing `p_input`.
 *
 * @param board_key Mechanical board key identity.
 * @param p_input Caller-owned semantic input destination.
 *
 * @return `true` when a semantic input was produced.
 * @return `false` when `board_key` is invalid or `p_event` is `NULL`.
 */
bool FM_MAIN_INPUT_ADAPTER_ShortInputFromBoardKey(
    fm_board_keyboard_key_t board_key,
    fmc_input_event_t *p_input);

#endif /* FM_MAIN_INPUT_ADAPTER_H */
