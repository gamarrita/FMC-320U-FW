/**
 * @file    fm_main_input_recognizer.c
 * @brief   Product main mechanical-key SHORT/LONG recognizer implementation.
 */
#include "fm_main_input_recognizer.h"

#include <stddef.h>

#include "fm_main_input_adapter.h"

static void fm_main_input_recognizer_output_clear_(
    fm_main_input_recognizer_output_t *p_output);
static bool fm_main_input_recognizer_key_is_valid_(
    fm_board_keyboard_key_t key);
static fm_status_t fm_main_input_recognizer_start_(
    fm_main_input_recognizer_t *p_recognizer,
    fm_board_keyboard_key_t key,
    fm_main_input_recognizer_output_t *p_output);
static fm_status_t fm_main_input_recognizer_finish_(
    fm_main_input_recognizer_t *p_recognizer,
    fm_board_keyboard_key_t key,
    fm_main_input_recognizer_output_t *p_output);

void FM_MAIN_INPUT_RECOGNIZER_Init(
    fm_main_input_recognizer_t *p_recognizer)
{
    if (p_recognizer == NULL)
    {
        return;
    }

    p_recognizer->active = false;
    p_recognizer->long_emitted = false;
    p_recognizer->active_key = FM_BOARD_KEYBOARD_KEY_COUNT;
}

fm_status_t FM_MAIN_INPUT_RECOGNIZER_HandleKeyboard(
    fm_main_input_recognizer_t *p_recognizer,
    fm_board_keyboard_key_t key,
    fm_board_keyboard_edge_t edge,
    fm_main_input_recognizer_output_t *p_output)
{
    if (p_output != NULL)
    {
        fm_main_input_recognizer_output_clear_(p_output);
    }

    if ((p_recognizer == NULL) || (p_output == NULL) ||
        !fm_main_input_recognizer_key_is_valid_(key))
    {
        return FM_STATUS_EINVAL;
    }

    switch (edge)
    {
    case FM_BOARD_KEYBOARD_EDGE_RISING:
        return fm_main_input_recognizer_start_(p_recognizer, key, p_output);

    case FM_BOARD_KEYBOARD_EDGE_FALLING:
        return fm_main_input_recognizer_finish_(p_recognizer, key, p_output);

    default:
        return FM_STATUS_EINVAL;
    }
}

fm_status_t FM_MAIN_INPUT_RECOGNIZER_HandleHoldTimeout(
    fm_main_input_recognizer_t *p_recognizer,
    fm_main_input_recognizer_output_t *p_output)
{
    if (p_output != NULL)
    {
        fm_main_input_recognizer_output_clear_(p_output);
    }

    if ((p_recognizer == NULL) || (p_output == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    if (!p_recognizer->active || p_recognizer->long_emitted)
    {
        return FM_STATUS_ESTATE;
    }

    if (!FM_MAIN_INPUT_ADAPTER_EventFromBoardKey(
            p_recognizer->active_key,
            FMC_INPUT_ACTION_LONG,
            &p_output->runtime_event))
    {
        return FM_STATUS_EINVAL;
    }

    p_recognizer->long_emitted = true;
    p_output->runtime_event_valid = true;

    return FM_STATUS_OK;
}

static void fm_main_input_recognizer_output_clear_(
    fm_main_input_recognizer_output_t *p_output)
{
    if (p_output == NULL)
    {
        return;
    }

    p_output->timer_action = FM_MAIN_INPUT_RECOGNIZER_TIMER_NONE;
    p_output->runtime_event_valid = false;
    p_output->runtime_event.kind = FMC_RUNTIME_EVENT_NONE;
}

static bool fm_main_input_recognizer_key_is_valid_(
    fm_board_keyboard_key_t key)
{
    switch (key)
    {
    case FM_BOARD_KEYBOARD_KEY_DOWN:
    case FM_BOARD_KEYBOARD_KEY_UP:
    case FM_BOARD_KEYBOARD_KEY_ENTER:
    case FM_BOARD_KEYBOARD_KEY_ESC:
        return true;

    case FM_BOARD_KEYBOARD_KEY_COUNT:
    default:
        return false;
    }
}

static fm_status_t fm_main_input_recognizer_start_(
    fm_main_input_recognizer_t *p_recognizer,
    fm_board_keyboard_key_t key,
    fm_main_input_recognizer_output_t *p_output)
{
    fm_status_t status = FM_STATUS_OK;

    if (p_recognizer->active)
    {
        status = FM_STATUS_ESTATE;
    }

    p_recognizer->active = true;
    p_recognizer->long_emitted = false;
    p_recognizer->active_key = key;
    p_output->timer_action = FM_MAIN_INPUT_RECOGNIZER_TIMER_START;

    return status;
}

static fm_status_t fm_main_input_recognizer_finish_(
    fm_main_input_recognizer_t *p_recognizer,
    fm_board_keyboard_key_t key,
    fm_main_input_recognizer_output_t *p_output)
{
    if (!p_recognizer->active || (p_recognizer->active_key != key))
    {
        return FM_STATUS_ESTATE;
    }

    p_output->timer_action = FM_MAIN_INPUT_RECOGNIZER_TIMER_CANCEL;

    if (!p_recognizer->long_emitted)
    {
        if (!FM_MAIN_INPUT_ADAPTER_EventFromBoardKey(
                key,
                FMC_INPUT_ACTION_SHORT,
                &p_output->runtime_event))
        {
            return FM_STATUS_EINVAL;
        }

        p_output->runtime_event_valid = true;
    }

    p_recognizer->active = false;
    p_recognizer->long_emitted = false;
    p_recognizer->active_key = FM_BOARD_KEYBOARD_KEY_COUNT;

    return FM_STATUS_OK;
}
