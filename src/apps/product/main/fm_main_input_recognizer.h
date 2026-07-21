/**
 * @file    fm_main_input_recognizer.h
 * @brief   Product main mechanical-key SHORT/LONG recognizer.
 *
 * This app-level helper interprets board keyboard edges and produces semantic
 * FMC runtime input events. It does not own ThreadX timers or queues: callers
 * execute the requested timer action in their own composition layer.
 */
#ifndef FM_MAIN_INPUT_RECOGNIZER_H
#define FM_MAIN_INPUT_RECOGNIZER_H

#include <stdbool.h>

#include "fm_board_keyboard.h"
#include "fm_status.h"
#include "fmc_runtime.h"

typedef enum
{
    FM_MAIN_INPUT_RECOGNIZER_TIMER_NONE = 0,
    FM_MAIN_INPUT_RECOGNIZER_TIMER_START,
    FM_MAIN_INPUT_RECOGNIZER_TIMER_CANCEL
} fm_main_input_recognizer_timer_action_t;

typedef struct
{
    fm_main_input_recognizer_timer_action_t timer_action;
    bool runtime_event_valid;
    fmc_runtime_event_t runtime_event;
} fm_main_input_recognizer_output_t;

typedef struct
{
    bool active;
    bool long_emitted;
    fm_board_keyboard_key_t active_key;
} fm_main_input_recognizer_t;

/**
 * @brief Initialize a mechanical-key recognizer instance.
 *
 * @param p_recognizer Caller-owned recognizer state.
 */
void FM_MAIN_INPUT_RECOGNIZER_Init(
    fm_main_input_recognizer_t *p_recognizer);

/**
 * @brief Handle one board keyboard edge.
 *
 * RISING starts a key hold and requests timer start. FALLING completes the
 * hold; it emits SHORT only when no LONG was already emitted for that hold.
 *
 * @param p_recognizer Caller-owned recognizer state.
 * @param key Board key identity.
 * @param edge Board edge identity.
 * @param p_output Caller-owned output destination.
 *
 * @return `FM_STATUS_OK` when the edge was accepted.
 * @return `FM_STATUS_EINVAL` when an argument, key, or edge is invalid.
 * @return `FM_STATUS_ESTATE` when the edge sequence is abnormal but contained.
 */
fm_status_t FM_MAIN_INPUT_RECOGNIZER_HandleKeyboard(
    fm_main_input_recognizer_t *p_recognizer,
    fm_board_keyboard_key_t key,
    fm_board_keyboard_edge_t edge,
    fm_main_input_recognizer_output_t *p_output);

/**
 * @brief Handle one key-hold timeout.
 *
 * Emits one LONG for the currently active hold. Duplicate or stale timeout
 * delivery is reported as an abnormal state and produces no runtime event.
 *
 * @param p_recognizer Caller-owned recognizer state.
 * @param p_output Caller-owned output destination.
 *
 * @return `FM_STATUS_OK` when LONG was emitted.
 * @return `FM_STATUS_EINVAL` when any pointer is `NULL`.
 * @return `FM_STATUS_ESTATE` when no active hold can consume the timeout.
 */
fm_status_t FM_MAIN_INPUT_RECOGNIZER_HandleHoldTimeout(
    fm_main_input_recognizer_t *p_recognizer,
    fm_main_input_recognizer_output_t *p_output);

#endif /* FM_MAIN_INPUT_RECOGNIZER_H */
