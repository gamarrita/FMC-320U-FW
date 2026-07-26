/**
 * @file    fm_main_acquisition.h
 * @brief   Product-main adapter from physical observations to FMC runtime.
 *
 * This adapter owns the independent pulse-delta and frequency-observation
 * baselines used by the product main owner loop. It does not read hardware,
 * schedule observations, own product totals or RATE, or emit diagnostics.
 */
#ifndef FM_MAIN_ACQUISITION_H
#define FM_MAIN_ACQUISITION_H

#include <stdint.h>

#include "fmc_runtime.h"
#include "fm_status.h"
#include "frequency_observation.h"
#include "pulse_delta.h"

/**
 * @brief Caller-owned acquisition state for one product-main instance.
 */
typedef struct
{
    pulse_delta_observer_t pulse_delta_observer;
    frequency_observation_t frequency_observer;
} fm_main_acquisition_t;

/**
 * @brief Initialize acquisition with the accepted zero counter baseline.
 *
 * Existing state is overwritten. Passing `NULL` has no effect.
 *
 * @param p_acquisition Acquisition state owned by the caller.
 */
void FM_MAIN_ACQUISITION_Init(fm_main_acquisition_t *p_acquisition);

/**
 * @brief Process one trusted counter observation through the runtime boundary.
 *
 * Forms exactly one modulo-16-bit pulse delta and dispatches exactly one
 * `FMC_RUNTIME_EVENT_PULSE_DELTA`, including when the delta is zero.
 * `fmc_runtime` and `fmc_service` retain ownership of zero-event semantics and
 * canonical ACM/TTL total updates.
 *
 * @param p_acquisition Initialized acquisition state owned by the caller.
 * @param current_count Trusted current 16-bit counter observation.
 * @param p_runtime Initialized runtime owned and serialized by the caller.
 *
 * @return `FM_STATUS_OK` when observation and dispatch both succeed.
 * @return `FM_STATUS_EINVAL` when an input pointer is `NULL`.
 * @return Any non-OK status returned by delta formation or runtime dispatch.
 */
fm_status_t FM_MAIN_ACQUISITION_ProcessObservation(
    fm_main_acquisition_t *p_acquisition,
    uint16_t current_count,
    fmc_runtime_t *p_runtime);

/**
 * @brief Process one trusted pulse/time sample through the runtime boundary.
 *
 * The frequency observer decides whether the sample produces a result. When
 * it does, this operation dispatches exactly one
 * `FMC_RUNTIME_EVENT_FREQUENCY_RESULT`; an admitted early sample produces no
 * event. This operation does not affect the independent pulse-delta baseline.
 *
 * @param p_acquisition Initialized acquisition state owned by the caller.
 * @param p_sample Trusted cumulative count and monotonic timestamp sample.
 * @param p_runtime Initialized runtime owned and serialized by the caller.
 *
 * @return `FM_STATUS_OK` when observation and any required dispatch succeed.
 * @return `FM_STATUS_EINVAL` when an input pointer is `NULL`.
 * @return Any non-OK observer or runtime status unchanged.
 */
fm_status_t FM_MAIN_ACQUISITION_ProcessFrequencyObservation(
    fm_main_acquisition_t *p_acquisition,
    const frequency_observation_sample_t *p_sample,
    fmc_runtime_t *p_runtime);

#endif /* FM_MAIN_ACQUISITION_H */
