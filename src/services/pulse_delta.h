/**
 * @file    pulse_delta.h
 * @brief   Stateful modulo-16-bit pulse-delta formation.
 *
 * This module converts successive trusted 16-bit counter observations into
 * widened raw pulse deltas. It owns only the previous observation baseline;
 * it does not read hardware, publish events, update totals, measure time, or
 * infer frequency or quality.
 */
#ifndef PULSE_DELTA_H
#define PULSE_DELTA_H

#include <stdint.h>

#include "fm_status.h"

/**
 * @brief Caller-owned state for one observed pulse counter.
 *
 * Treat fields as module-owned after initialization.
 */
typedef struct
{
    uint16_t previous_count;
} pulse_delta_observer_t;

/**
 * @brief Initialize one observer with the accepted zero baseline.
 *
 * Existing state is overwritten. Passing `NULL` has no effect.
 *
 * @param p_observer Observer state owned by the caller.
 */
void PULSE_DELTA_Init(pulse_delta_observer_t *p_observer);

/**
 * @brief Restore one observer to the accepted zero baseline.
 *
 * This begins a new accumulation interval. Passing `NULL` has no effect.
 *
 * @param p_observer Observer state owned by the caller.
 */
void PULSE_DELTA_Reset(pulse_delta_observer_t *p_observer);

/**
 * @brief Form one raw delta from a trusted 16-bit counter observation.
 *
 * The calculation is `(current_count - previous_count) modulo 65,536`.
 * The resulting `uint16_t` delta is widened to `uint64_t`, and
 * `current_count` becomes the next baseline. A zero delta is valid.
 *
 * The true number of pulses since the previous observation must be in the
 * inclusive range `0..65,535`; violations are indistinguishable from valid
 * modulo deltas and are not detected here.
 *
 * @param p_observer Initialized observer state owned by the caller.
 * @param current_count Trusted current 16-bit counter observation.
 * @param p_pulse_delta Output raw pulse delta in the range `0..65,535`.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when `p_observer` or `p_pulse_delta` is `NULL`.
 */
fm_status_t PULSE_DELTA_Observe(pulse_delta_observer_t *p_observer,
                                uint16_t current_count,
                                uint64_t *p_pulse_delta);

#endif /* PULSE_DELTA_H */
