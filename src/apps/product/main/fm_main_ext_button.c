/**
 * @file    fm_main_ext_button.c
 * @brief   External-button press and stable-release recognizer.
 */
#include "fm_main_ext_button.h"

#include <stddef.h>

static bool fm_main_ext_button_key_is_valid_(fmc_input_key_t key);
static bool fm_main_ext_button_is_valid_(
    const fm_main_ext_button_t *p_button);
static void fm_main_ext_button_output_clear_(
    fm_main_ext_button_output_t *p_output);

fm_status_t FM_MAIN_EXT_BUTTON_Init(
    fm_main_ext_button_t *p_button,
    fmc_input_key_t key,
    bool pressed,
    fm_main_ext_button_output_t *p_output)
{
    if (p_output != NULL)
    {
        fm_main_ext_button_output_clear_(p_output);
    }

    if ((p_button == NULL) || (p_output == NULL) ||
        !fm_main_ext_button_key_is_valid_(key))
    {
        return FM_STATUS_EINVAL;
    }

    p_button->key = key;
    p_button->armed = false;
    p_button->released_sample_seen = !pressed;
    p_output->timer_action = FM_MAIN_EXT_BUTTON_TIMER_START;

    return FM_STATUS_OK;
}

fm_status_t FM_MAIN_EXT_BUTTON_HandlePress(
    fm_main_ext_button_t *p_button,
    fm_main_ext_button_output_t *p_output)
{
    if (p_output != NULL)
    {
        fm_main_ext_button_output_clear_(p_output);
    }

    if (!fm_main_ext_button_is_valid_(p_button) || (p_output == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    if (p_button->armed)
    {
        p_button->armed = false;
        p_output->input.key = p_button->key;
        p_output->input.action = FMC_INPUT_ACTION_SHORT;
        p_output->input_valid = true;
    }

    p_button->released_sample_seen = false;
    p_output->timer_action = FM_MAIN_EXT_BUTTON_TIMER_RESTART;

    return FM_STATUS_OK;
}

fm_status_t FM_MAIN_EXT_BUTTON_HandleSampleTimeout(
    fm_main_ext_button_t *p_button,
    bool pressed,
    fm_main_ext_button_output_t *p_output)
{
    if (p_output != NULL)
    {
        fm_main_ext_button_output_clear_(p_output);
    }

    if (!fm_main_ext_button_is_valid_(p_button) || (p_output == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    if (p_button->armed)
    {
        return FM_STATUS_OK;
    }

    if (pressed)
    {
        p_button->released_sample_seen = false;
        p_output->timer_action = FM_MAIN_EXT_BUTTON_TIMER_RESTART;
        return FM_STATUS_OK;
    }

    if (!p_button->released_sample_seen)
    {
        p_button->released_sample_seen = true;
        p_output->timer_action = FM_MAIN_EXT_BUTTON_TIMER_RESTART;
        return FM_STATUS_OK;
    }

    p_button->released_sample_seen = false;
    p_button->armed = true;

    return FM_STATUS_OK;
}

static bool fm_main_ext_button_key_is_valid_(fmc_input_key_t key)
{
    return (key == FMC_INPUT_KEY_EXT_1) ||
           (key == FMC_INPUT_KEY_EXT_2);
}

static bool fm_main_ext_button_is_valid_(
    const fm_main_ext_button_t *p_button)
{
    return (p_button != NULL) &&
           fm_main_ext_button_key_is_valid_(p_button->key);
}

static void fm_main_ext_button_output_clear_(
    fm_main_ext_button_output_t *p_output)
{
    p_output->timer_action = FM_MAIN_EXT_BUTTON_TIMER_NONE;
    p_output->input_valid = false;
    p_output->input.key = FMC_INPUT_KEY_COUNT;
    p_output->input.action = FMC_INPUT_ACTION_COUNT;
}
