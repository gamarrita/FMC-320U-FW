/**
 * @file    frequency_observation.h
 * @brief   Stateful pulse/time window admission for frequency observations.
 *
 * This RTOS-neutral, hardware-independent module owns one frequency baseline.
 * It forms modulo-16-bit pulse deltas over measured monotonic time, reports
 * explicit observation quality, and does not read hardware, schedule calls,
 * calculate RATE, update totals, or infer product activity.
 */

#ifndef FREQUENCY_OBSERVATION_H
#define FREQUENCY_OBSERVATION_H

#include <stdbool.h>
#include <stdint.h>

#include "fm_status.h"

/** Inclusive lower bound for one admissible observation window. */
#define FREQUENCY_OBSERVATION_MIN_ELAPSED_US   (900000ULL)

/** Inclusive upper bound for one admissible observation window. */
#define FREQUENCY_OBSERVATION_MAX_ELAPSED_US   (1100000ULL)

/**
 * @brief Quality of one newly reported frequency observation.
 */
typedef enum
{
    /** A complete pulse/time window is available. */
    FREQUENCY_OBSERVATION_QUALITY_VALID = 0,
    /** No complete usable window has yet been formed. */
    FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE,
    /** A previous valid window was not refreshed within the admitted time. */
    FREQUENCY_OBSERVATION_QUALITY_STALE,
    /** A supplied sample violated the trusted monotonic-time contract. */
    FREQUENCY_OBSERVATION_QUALITY_INVALID
} frequency_observation_quality_t;

/**
 * @brief One trusted cumulative counter and monotonic-time sample.
 */
typedef struct
{
    /** Stable cumulative 16-bit pulse-counter observation. */
    uint16_t pulse_count;
    /** Monotonic sample timestamp in microseconds. */
    uint64_t timestamp_us;
} frequency_observation_sample_t;

/**
 * @brief One newly reported observation result.
 *
 * `pulse_delta` and `elapsed_us` are usable only when `quality` is
 * `FREQUENCY_OBSERVATION_QUALITY_VALID`.
 */
typedef struct
{
    frequency_observation_quality_t quality;
    uint64_t pulse_delta;
    uint64_t elapsed_us;
} frequency_observation_result_t;

/**
 * @brief Caller-owned state for one independent frequency observer.
 *
 * Treat every field as module-owned after initialization.
 */
typedef struct
{
    uint16_t previous_count;
    uint64_t previous_timestamp_us;
    bool baseline_available;
    bool valid_window_seen;
} frequency_observation_t;

/**
 * @brief Initialize an observer without an active baseline.
 *
 * Existing state is overwritten. Passing `NULL` has no effect.
 *
 * @param p_observer Observer state owned by the caller.
 */
void FREQUENCY_OBSERVATION_Init(frequency_observation_t *p_observer);

/**
 * @brief Discard the active baseline and all prior quality history.
 *
 * The next trusted sample establishes a new baseline and reports
 * `UNAVAILABLE`. Passing `NULL` has no effect.
 *
 * @param p_observer Observer state owned by the caller.
 */
void FREQUENCY_OBSERVATION_Reset(frequency_observation_t *p_observer);

/**
 * @brief Consume one trusted counter/timestamp pair.
 *
 * The first sample establishes the baseline and reports `UNAVAILABLE`.
 * Subsequent samples use modulo-65,536 pulse subtraction and exact integer
 * timestamp subtraction.
 *
 * An elapsed time in the inclusive range
 * `FREQUENCY_OBSERVATION_MIN_ELAPSED_US` through
 * `FREQUENCY_OBSERVATION_MAX_ELAPSED_US` reports `VALID` and advances the
 * baseline. A zero pulse delta is valid.
 *
 * An earlier sample produces no new result and does not advance the baseline;
 * `*p_result_available` is set to false and `*p_result` is unchanged. A later
 * sample resynchronizes the baseline and reports `STALE` after a prior valid
 * window or `UNAVAILABLE` otherwise.
 *
 * An equal or decreasing timestamp reports `INVALID`, discards the baseline,
 * and requires a new baseline before another valid window can be formed.
 *
 * The true pulse delta between accepted baselines must be no greater than
 * 65,535. A complete or multiple counter wrap is not detectable.
 *
 * @param p_observer Initialized observer state owned by the caller.
 * @param p_sample Trusted cumulative counter and monotonic timestamp sample.
 * @param p_result_available Receives whether this call produced a new result.
 * @param p_result Receives the new result when available.
 *
 * @return `FM_STATUS_OK` when the sample was processed.
 * @return `FM_STATUS_EINVAL` when any pointer is `NULL`.
 *
 * @warning Serialized foreground use only. The module is not thread-safe or
 *          IRQ-safe.
 */
fm_status_t FREQUENCY_OBSERVATION_Observe(
    frequency_observation_t *p_observer,
    const frequency_observation_sample_t *p_sample,
    bool *p_result_available,
    frequency_observation_result_t *p_result);

/**
 * @brief Report a producer-detected incoherent or failed physical sample.
 *
 * Reports `INVALID`, discards the active baseline, and clears prior valid
 * history. The next trusted sample establishes a new baseline.
 *
 * @param p_observer Initialized observer state owned by the caller.
 * @param p_result Receives the invalid-quality result.
 *
 * @return `FM_STATUS_OK` when invalidation was recorded.
 * @return `FM_STATUS_EINVAL` when any pointer is `NULL`.
 */
fm_status_t FREQUENCY_OBSERVATION_Invalidate(
    frequency_observation_t *p_observer,
    frequency_observation_result_t *p_result);

#endif /* FREQUENCY_OBSERVATION_H */
