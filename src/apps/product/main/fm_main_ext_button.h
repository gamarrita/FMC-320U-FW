/**
 * @file    fm_main_ext_button.h
 * @brief   RTOS-neutral external-button press and stable-release recognizer.
 *
 * One instance owns the independent arming state of one external button. A
 * press emits at most one semantic SHORT action and disarms the instance. The
 * instance arms only after a released level has remained stable through the
 * caller-owned 100 ms timer.
 *
 * This module does not access GPIO, HAL, ThreadX, queues, UI, runtime, or
 * backlight. Callers execute the returned timer instruction and serialize
 * timeout delivery.
 */
#ifndef FM_MAIN_EXT_BUTTON_H
#define FM_MAIN_EXT_BUTTON_H

#include <stdbool.h>

#include "fm_status.h"
#include "fmc_input.h"

typedef enum
{
    FM_MAIN_EXT_BUTTON_TIMER_NONE = 0,
    FM_MAIN_EXT_BUTTON_TIMER_START,
    FM_MAIN_EXT_BUTTON_TIMER_RESTART
} fm_main_ext_button_timer_action_t;

typedef struct
{
    fm_main_ext_button_timer_action_t timer_action;
    bool input_valid;
    fmc_input_event_t input;
} fm_main_ext_button_output_t;

typedef struct
{
    fmc_input_key_t key;
    bool armed;
    bool released_sample_seen;
} fm_main_ext_button_t;

/**
 * @brief Initialize one external-button recognizer from its sampled level.
 *
 * The button starts disarmed and requests its stable-release sampling timer.
 * A released initial sample becomes the first of the two samples required to
 * establish a stable release; a pressed initial sample does not.
 *
 * @param p_button Caller-owned recognizer state.
 * @param key `FMC_INPUT_KEY_EXT_1` or `FMC_INPUT_KEY_EXT_2`.
 * @param pressed Current board-normalized pressed state.
 * @param p_output Caller-owned output instruction.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` for an invalid pointer or semantic key.
 */
fm_status_t FM_MAIN_EXT_BUTTON_Init(
    fm_main_ext_button_t *p_button,
    fmc_input_key_t key,
    bool pressed,
    fm_main_ext_button_output_t *p_output);

/**
 * @brief Handle one physical press transition.
 *
 * An armed instance emits one SHORT and disarms. Every accepted press resets
 * stable-release sampling and requests a fresh timer interval. Further calls
 * while disarmed do not emit another input.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when an argument is invalid.
 */
fm_status_t FM_MAIN_EXT_BUTTON_HandlePress(
    fm_main_ext_button_t *p_button,
    fm_main_ext_button_output_t *p_output);

/**
 * @brief Consume one caller-owned stable-release sample timeout.
 *
 * A pressed sample resets release qualification. Two released samples one
 * caller-owned timer interval apart arm the button. Until then the output
 * requests another sample interval. A timeout received while already armed is
 * an accepted stale no-op.
 *
 * @param p_button Caller-owned recognizer state.
 * @param pressed Current board-normalized pressed state.
 * @param p_output Caller-owned output instruction.
 *
 * @return `FM_STATUS_OK` on success or for a stale timeout.
 * @return `FM_STATUS_EINVAL` when an argument is invalid.
 */
fm_status_t FM_MAIN_EXT_BUTTON_HandleSampleTimeout(
    fm_main_ext_button_t *p_button,
    bool pressed,
    fm_main_ext_button_output_t *p_output);

#endif /* FM_MAIN_EXT_BUTTON_H */
