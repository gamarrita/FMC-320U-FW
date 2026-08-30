/**
 * @file    fm_main_backlight.h
 * @brief   RTOS-neutral FMC backlight activation and expiry coordination.
 *
 * The caller owns the timer and physical output. This module commits one
 * wrap-safe expiry deadline per activation request and decides whether a timer
 * callback belongs to the current interval. Callers must serialize access when
 * activation and timeout handling can run from different contexts.
 */
#ifndef FM_MAIN_BACKLIGHT_H
#define FM_MAIN_BACKLIGHT_H

#include <stdbool.h>
#include <stdint.h>

#include "fm_status.h"

typedef enum
{
    FM_MAIN_BACKLIGHT_TIMER_NONE = 0,
    FM_MAIN_BACKLIGHT_TIMER_RESTART
} fm_main_backlight_timer_action_t;

typedef struct
{
    fm_main_backlight_timer_action_t timer_action;
    uint32_t timer_ticks;
    uint32_t generation;
    bool turn_on;
    bool turn_off;
} fm_main_backlight_output_t;

typedef struct
{
    uint32_t duration_ticks;
    uint32_t deadline_ticks;
    uint32_t generation;
    bool active;
} fm_main_backlight_t;

/**
 * @brief Initialize one safely-off backlight coordination state.
 *
 * `p_duration_ticks` must fit within half of the 32-bit tick range so deadline
 * comparisons remain unambiguous across wrap.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` for a null state or invalid duration.
 */
fm_status_t FM_MAIN_BACKLIGHT_Init(fm_main_backlight_t *p_backlight,
                                   uint32_t p_duration_ticks);

/**
 * @brief Commit a fresh expiry and request timer restart followed by output-on.
 *
 * The state deadline and generation are committed before `turn_on` is returned.
 * The caller must successfully restart its one-shot timer before applying the
 * output-on instruction.
 *
 * @param p_backlight Initialized caller-owned state.
 * @param p_now_ticks Current monotonic 32-bit tick value.
 * @param p_output Caller-owned action output.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` for invalid state or output pointer.
 */
fm_status_t FM_MAIN_BACKLIGHT_RequestActivation(
    fm_main_backlight_t *p_backlight,
    uint32_t p_now_ticks,
    fm_main_backlight_output_t *p_output);

/**
 * @brief Evaluate one direct timer callback against the committed deadline.
 *
 * A callback at or after the current deadline requests output-off exactly once.
 * An inactive or early callback is a stale no-op and cannot defeat a newer
 * activation request.
 *
 * @param p_backlight Initialized caller-owned state.
 * @param p_now_ticks Current monotonic 32-bit tick value.
 * @param p_output Caller-owned action output.
 *
 * @return `FM_STATUS_OK` on success or for a stale callback.
 * @return `FM_STATUS_EINVAL` for invalid state or output pointer.
 */
fm_status_t FM_MAIN_BACKLIGHT_HandleTimeout(
    fm_main_backlight_t *p_backlight,
    uint32_t p_now_ticks,
    fm_main_backlight_output_t *p_output);

#endif /* FM_MAIN_BACKLIGHT_H */
