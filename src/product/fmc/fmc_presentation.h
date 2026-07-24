/**
 * @file    fmc_presentation.h
 * @brief   RTOS-neutral FMC Phase 6A presentation contract.
 *
 * This module owns the bounded startup presentation sequence and composes
 * semantic display frames. It does not access LCD hardware, own timers,
 * calculate TTL or RATE, or acquire measurement windows.
 */

#ifndef FMC_PRESENTATION_H
#define FMC_PRESENTATION_H

#include <stdbool.h>
#include <stdint.h>

#include "fm_status.h"
#include "fmc_input.h"
#include "fmc_model.h"

#define FMC_PRESENTATION_TOP_TEXT_SIZE       10U
#define FMC_PRESENTATION_BOTTOM_TEXT_SIZE    10U
#define FMC_PRESENTATION_ALPHA_TEXT_SIZE     3U
#define FMC_PRESENTATION_LITERS_LEGEND       "Lt"

typedef enum
{
    FMC_PRESENTATION_STATE_NOT_STARTED = 0,
    FMC_PRESENTATION_STATE_ALL_SEGMENTS,
    FMC_PRESENTATION_STATE_FIRMWARE_VERSION,
    FMC_PRESENTATION_STATE_TTL_RATE
} fmc_presentation_state_t;

/**
 * @brief Coherent values supplied to the Phase 6A presentation.
 *
 * TTL and RATE are accepted product values. Presentation does not derive them
 * or own the RATE observation window. Phase 6A accepts only liters, minutes,
 * one fractional digit, and non-negative values.
 */
typedef struct
{
    double ttl;
    double rate;
    fmc_model_volume_unit_t volume_unit;
    fmc_model_time_base_t rate_time_base;
    uint8_t ttl_fractional_digits;
    uint8_t rate_fractional_digits;
} fmc_presentation_snapshot_t;

/**
 * @brief Semantic display frame independent from LCD mapping.
 *
 * When `all_segments` is true, all other fields are ignored and the sink must
 * activate every software-controllable physical LCD segment.
 * A TTL/RATE frame using liters carries
 * `FMC_PRESENTATION_LITERS_LEGEND` in `alpha_text`.
 */
typedef struct
{
    bool all_segments;
    char top_text[FMC_PRESENTATION_TOP_TEXT_SIZE];
    char bottom_text[FMC_PRESENTATION_BOTTOM_TEXT_SIZE];
    char alpha_text[FMC_PRESENTATION_ALPHA_TEXT_SIZE];
    bool indicator_ttl;
    bool indicator_rate;
    bool indicator_slash;
    bool indicator_minute;
    bool ttl_overflow;
    bool rate_overflow;
} fmc_presentation_frame_t;

typedef fm_status_t (*fmc_presentation_sink_t)(
    const fmc_presentation_frame_t *p_frame,
    void *p_context);

typedef struct
{
    fmc_presentation_state_t state;
    fmc_presentation_snapshot_t snapshot;
    fmc_presentation_sink_t sink;
    void *sink_context;
} fmc_presentation_t;

/**
 * @brief Populate the provisional valid Phase 6A snapshot.
 *
 * @param p_snapshot Caller-owned destination.
 */
void FMC_PRESENTATION_MakeDummySnapshot(
    fmc_presentation_snapshot_t *p_snapshot);

/**
 * @brief Initialize a presentation instance without showing a frame.
 *
 * @param p_presentation Caller-owned presentation state.
 * @param p_snapshot Initial coherent snapshot.
 * @param p_sink Frame sink used for every presentation attempt.
 * @param p_sink_context Opaque sink context.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` for invalid pointers or unsupported snapshot
 *         content.
 */
fm_status_t FMC_PRESENTATION_Init(
    fmc_presentation_t *p_presentation,
    const fmc_presentation_snapshot_t *p_snapshot,
    fmc_presentation_sink_t p_sink,
    void *p_sink_context);

/**
 * @brief Present the initial all-segments state.
 *
 * State changes only after the sink accepts the frame.
 *
 * @return `FM_STATUS_OK` when the frame was presented.
 * @return `FM_STATUS_ESTATE` when already started.
 * @return Other sink or formatting errors unchanged.
 */
fm_status_t FMC_PRESENTATION_Start(
    fmc_presentation_t *p_presentation);

/**
 * @brief Advance one startup state after timeout or accepted ESC.
 *
 * All-segments advances to firmware version, and firmware version advances to
 * TTL/RATE. TTL/RATE remains stable. State changes only after successful frame
 * delivery.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_ESTATE` before startup.
 * @return Other sink or formatting errors unchanged.
 */
fm_status_t FMC_PRESENTATION_Advance(
    fmc_presentation_t *p_presentation);

/**
 * @brief Apply one semantic input to the bounded startup sequence.
 *
 * A SHORT ESC advances a temporary startup state. Other input, LONG ESC, and
 * input received in TTL/RATE have no presentation consequence.
 *
 * @return `FM_STATUS_OK` on success or when no action is required.
 * @return `FM_STATUS_EINVAL` for invalid pointers.
 * @return Other presentation errors unchanged.
 */
fm_status_t FMC_PRESENTATION_HandleInput(
    fmc_presentation_t *p_presentation,
    const fmc_input_event_t *p_input);

/**
 * @brief Present a fresh coherent TTL/RATE snapshot.
 *
 * This operation is valid only in the stable TTL/RATE state. The stored
 * snapshot is replaced only after successful frame delivery.
 *
 * @return `FM_STATUS_OK` when the refreshed frame was presented.
 * @return `FM_STATUS_EINVAL` for invalid pointers or unsupported values.
 * @return `FM_STATUS_ESTATE` outside TTL/RATE.
 * @return Other sink or formatting errors unchanged.
 */
fm_status_t FMC_PRESENTATION_Refresh(
    fmc_presentation_t *p_presentation,
    const fmc_presentation_snapshot_t *p_snapshot);

/**
 * @brief Return the current successfully presented state.
 *
 * @return Current state, or `FMC_PRESENTATION_STATE_NOT_STARTED` for `NULL`.
 */
fmc_presentation_state_t FMC_PRESENTATION_GetState(
    const fmc_presentation_t *p_presentation);

#endif /* FMC_PRESENTATION_H */
