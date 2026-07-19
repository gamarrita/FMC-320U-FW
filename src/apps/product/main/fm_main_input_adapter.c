/**
 * @file    fm_main_input_adapter.c
 * @brief   Product app input adapter implementation.
 */
#include "fm_main_input_adapter.h"

#include <stddef.h>

static bool fm_main_input_adapter_key_from_board_(
    fm_board_keyboard_key_t board_key,
    fmc_input_key_t *p_key);

bool FM_MAIN_INPUT_ADAPTER_ShortEventFromBoardKey(
    fm_board_keyboard_key_t board_key,
    fmc_runtime_event_t *p_event)
{
    fmc_input_key_t input_key;

    if ((p_event == NULL) ||
        !fm_main_input_adapter_key_from_board_(board_key, &input_key))
    {
        return false;
    }

    p_event->kind = FMC_RUNTIME_EVENT_INPUT;
    p_event->data.input.key = input_key;
    p_event->data.input.action = FMC_INPUT_ACTION_SHORT;

    return true;
}

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

    case FM_BOARD_KEYBOARD_KEY_COUNT:
    default:
        return false;
    }
}
