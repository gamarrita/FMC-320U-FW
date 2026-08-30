/**
 * @file    fm_main_backlight.c
 * @brief   FMC backlight activation and expiry coordination.
 */
#include "fm_main_backlight.h"

#include <stddef.h>

#define FM_MAIN_BACKLIGHT_MAX_DURATION_TICKS (UINT32_MAX / 2U)

static bool fm_main_backlight_is_valid_(
    const fm_main_backlight_t *p_backlight);
static bool fm_main_backlight_deadline_reached_(uint32_t p_now_ticks,
                                                uint32_t p_deadline_ticks);
static void fm_main_backlight_output_clear_(
    fm_main_backlight_output_t *p_output);

fm_status_t FM_MAIN_BACKLIGHT_Init(fm_main_backlight_t *p_backlight,
                                   uint32_t p_duration_ticks)
{
    if ((p_backlight == NULL) || (p_duration_ticks == 0U) ||
        (p_duration_ticks > FM_MAIN_BACKLIGHT_MAX_DURATION_TICKS))
    {
        return FM_STATUS_EINVAL;
    }

    p_backlight->duration_ticks = p_duration_ticks;
    p_backlight->deadline_ticks = 0U;
    p_backlight->generation = 0U;
    p_backlight->active = false;

    return FM_STATUS_OK;
}

fm_status_t FM_MAIN_BACKLIGHT_RequestActivation(
    fm_main_backlight_t *p_backlight,
    uint32_t p_now_ticks,
    fm_main_backlight_output_t *p_output)
{
    if (p_output != NULL)
    {
        fm_main_backlight_output_clear_(p_output);
    }

    if (!fm_main_backlight_is_valid_(p_backlight) || (p_output == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    p_backlight->generation++;
    p_backlight->deadline_ticks =
        p_now_ticks + p_backlight->duration_ticks;
    p_backlight->active = true;

    p_output->timer_action = FM_MAIN_BACKLIGHT_TIMER_RESTART;
    p_output->timer_ticks = p_backlight->duration_ticks;
    p_output->generation = p_backlight->generation;
    p_output->turn_on = true;

    return FM_STATUS_OK;
}

fm_status_t FM_MAIN_BACKLIGHT_HandleTimeout(
    fm_main_backlight_t *p_backlight,
    uint32_t p_now_ticks,
    fm_main_backlight_output_t *p_output)
{
    if (p_output != NULL)
    {
        fm_main_backlight_output_clear_(p_output);
    }

    if (!fm_main_backlight_is_valid_(p_backlight) || (p_output == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    p_output->generation = p_backlight->generation;

    if (!p_backlight->active ||
        !fm_main_backlight_deadline_reached_(
            p_now_ticks,
            p_backlight->deadline_ticks))
    {
        return FM_STATUS_OK;
    }

    p_backlight->active = false;
    p_output->turn_off = true;

    return FM_STATUS_OK;
}

static bool fm_main_backlight_is_valid_(
    const fm_main_backlight_t *p_backlight)
{
    return (p_backlight != NULL) &&
           (p_backlight->duration_ticks != 0U) &&
           (p_backlight->duration_ticks <=
            FM_MAIN_BACKLIGHT_MAX_DURATION_TICKS);
}

static bool fm_main_backlight_deadline_reached_(uint32_t p_now_ticks,
                                                uint32_t p_deadline_ticks)
{
    return ((p_now_ticks - p_deadline_ticks) <=
            FM_MAIN_BACKLIGHT_MAX_DURATION_TICKS);
}

static void fm_main_backlight_output_clear_(
    fm_main_backlight_output_t *p_output)
{
    p_output->timer_action = FM_MAIN_BACKLIGHT_TIMER_NONE;
    p_output->timer_ticks = 0U;
    p_output->generation = 0U;
    p_output->turn_on = false;
    p_output->turn_off = false;
}
