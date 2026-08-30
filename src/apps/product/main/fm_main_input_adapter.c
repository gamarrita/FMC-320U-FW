/**
 * @file    fm_main_input_adapter.c
 * @brief   Product app input adapter implementation.
 */
#include "fm_main_input_adapter.h"

#include <stddef.h>

static bool fm_main_input_adapter_key_from_board_(
    fm_board_keyboard_key_t board_key,
    fmc_input_key_t *p_key);

bool FM_MAIN_INPUT_ADAPTER_InputFromBoardKey(
    fm_board_keyboard_key_t board_key,
    fmc_input_action_t action,
    fmc_input_event_t *p_input)
{
    fmc_input_key_t input_key;

    if ((p_input == NULL) ||
        ((action != FMC_INPUT_ACTION_SHORT) &&
         (action != FMC_INPUT_ACTION_LONG)) ||
        !fm_main_input_adapter_key_from_board_(board_key, &input_key) ||
        (((input_key == FMC_INPUT_KEY_EXT_1) ||
          (input_key == FMC_INPUT_KEY_EXT_2)) &&
         (action != FMC_INPUT_ACTION_SHORT)))
    {
        return false;
    }

    p_input->key = input_key;
    p_input->action = action;

    return true;
}

bool FM_MAIN_INPUT_ADAPTER_ShortInputFromBoardKey(
    fm_board_keyboard_key_t board_key,
    fmc_input_event_t *p_input)
{
    return FM_MAIN_INPUT_ADAPTER_InputFromBoardKey(
        board_key,
        FMC_INPUT_ACTION_SHORT,
        p_input);
}

/**
 * @brief Translate a supported board key into an FMC semantic input key.
 *
 * @param board_key Mechanical board key identity.
 * @param p_key Caller-owned semantic key destination.
 *
 * @return `true` when `board_key` is supported and @p p_key was updated.
 * @return `false` when `board_key` is unsupported or @p p_key is `NULL`.
 */
static bool fm_main_input_adapter_key_from_board_(
    fm_board_keyboard_key_t board_key,
    fmc_input_key_t *p_key)
{
    if (p_key == NULL)
    {
        return false;
    }

    switch (board_key)
    {
    case FM_BOARD_KEYBOARD_KEY_DOWN:
        *p_key = FMC_INPUT_KEY_DOWN;
        return true;

    case FM_BOARD_KEYBOARD_KEY_UP:
        *p_key = FMC_INPUT_KEY_UP;
        return true;

    case FM_BOARD_KEYBOARD_KEY_ENTER:
        *p_key = FMC_INPUT_KEY_ENTER;
        return true;

    case FM_BOARD_KEYBOARD_KEY_ESC:
        *p_key = FMC_INPUT_KEY_ESC;
        return true;

    case FM_BOARD_KEYBOARD_KEY_EXT_1:
        *p_key = FMC_INPUT_KEY_EXT_1;
        return true;

    case FM_BOARD_KEYBOARD_KEY_EXT_2:
        *p_key = FMC_INPUT_KEY_EXT_2;
        return true;

    case FM_BOARD_KEYBOARD_KEY_COUNT:
    default:
        return false;
    }
}
