/**
 * @file    fm_main_input_adapter.h
 * @brief   Product app adapter from board keyboard keys to FMC runtime input.
 *
 * This module belongs to the product app composition layer. It may include both
 * board-facing keyboard contracts and FMC runtime contracts, but it does not
 * expose GPIO, EXTI, HAL, ThreadX, queue, timer, debounce, wake, backlight, or
 * menu consequences.
 *
 * The current slice supports only mechanical board keys. It can populate
 * runtime input events with a caller-selected semantic action; recognizing
 * when an action is SHORT or LONG belongs to the product main owner loop.
 * External pushbuttons are intentionally outside this module's current scope.
 */
#ifndef FM_MAIN_INPUT_ADAPTER_H
#define FM_MAIN_INPUT_ADAPTER_H

#include <stdbool.h>

#include "fm_board_keyboard.h"
#include "fmc_input.h"
#include "fmc_runtime.h"

/**
 * @brief Translate one mechanical board key/action into a runtime input event.
 *
 * On success, `p_event` is populated with `FMC_RUNTIME_EVENT_INPUT` and the
 * matching FMC semantic key/action pair. Invalid keys, invalid actions, and
 * `NULL` outputs are rejected without writing `p_event`.
 *
 * @param board_key Mechanical board key identity.
 * @param action Semantic input action selected by the caller.
 * @param p_event Caller-owned runtime event destination.
 *
 * @return `true` when a runtime event was produced.
 * @return `false` when any input is invalid.
 */
bool FM_MAIN_INPUT_ADAPTER_EventFromBoardKey(
    fm_board_keyboard_key_t board_key,
    fmc_input_action_t action,
    fmc_runtime_event_t *p_event);

/**
 * @brief Translate one mechanical board key into a runtime SHORT input event.
 *
 * On success, `p_event` is populated with `FMC_RUNTIME_EVENT_INPUT` and the
 * matching FMC semantic key/action pair. Invalid keys and `NULL` outputs are
 * rejected without writing `p_event`.
 *
 * @param board_key Mechanical board key identity.
 * @param p_event Caller-owned runtime event destination.
 *
 * @return `true` when a runtime event was produced.
 * @return `false` when `board_key` is invalid or `p_event` is `NULL`.
 */
bool FM_MAIN_INPUT_ADAPTER_ShortEventFromBoardKey(
    fm_board_keyboard_key_t board_key,
    fmc_runtime_event_t *p_event);

#endif /* FM_MAIN_INPUT_ADAPTER_H */
