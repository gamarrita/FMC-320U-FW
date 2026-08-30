/**
 * @file    fmc_runtime.h
 * @brief   RTOS-neutral FMC runtime event boundary.
 *
 * `fmc_runtime` owns one `fmc_service_t` instance and applies product-level
 * events to it. It is the boundary a future main loop, scheduler, or RTOS
 * thread can drive without exposing HAL, LCD, timer, or queue types
 * to the FMC domain.
 *
 * The runtime currently controls:
 * - live FMC service state
 * - whether presentation has a pending update from a fresh snapshot
 *
 * The runtime does not capture pulses, debounce keys, render LCD segments,
 * authenticate privileged operations, persist state, allocate memory, block,
 * or call an RTOS/HAL API.
 *
 * Call `FMC_RUNTIME_Init()` before first use. The module does not detect an
 * uninitialized runtime object.
 *
 * Concurrency:
 * - functions are not internally synchronized
 * - callers must serialize access to the same runtime instance
 * - public functions are not designed to be called directly from ISR context;
 *   an ISR should publish work through a serialized adapter before dispatch
 */
#ifndef FMC_RUNTIME_H
#define FMC_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

#include "frequency_observation.h"
#include "fmc_service.h"
#include "fm_status.h"

/**
 * @brief Measurement/runtime events accepted by the minimal runtime boundary.
 *
 * Events describe measurement and totalization changes, not hardware input or
 * menu navigation. Presentation invalidation remains an explicit owner signal.
 */
typedef enum
{
    /** No operation; useful when a caller wants an explicit idle dispatch. */
    FMC_RUNTIME_EVENT_NONE = 0,
    /**
     * Add `pulse_delta` raw sensor pulses to the live totals.
     * A zero delta is accepted as a no-op.
     */
    FMC_RUNTIME_EVENT_PULSE_DELTA,
    /**
     * Apply one already admitted frequency-observation result to RATE state.
     */
    FMC_RUNTIME_EVENT_FREQUENCY_RESULT,
    /** Reset the ACM total after the caller has authorized the operation. */
    FMC_RUNTIME_EVENT_RESET_ACM,
    /** Reset the TTL total after the caller has authorized the operation. */
    FMC_RUNTIME_EVENT_RESET_TTL,
    /** Mark the current presentation view as no longer up to date. */
    FMC_RUNTIME_EVENT_PRESENTATION_INVALIDATE
} fmc_runtime_event_kind_t;

/**
 * @brief One event consumed by `FMC_RUNTIME_Dispatch()`.
 *
 * The active payload is selected by `kind`:
 * - `data.pulse_delta` for `FMC_RUNTIME_EVENT_PULSE_DELTA`
 * - `data.frequency_result` for `FMC_RUNTIME_EVENT_FREQUENCY_RESULT`
 *
 * Payload fields for other event kinds are ignored.
 */
typedef struct
{
    fmc_runtime_event_kind_t kind;
    union
    {
        uint64_t pulse_delta;
        frequency_observation_result_t frequency_result;
    } data;
} fmc_runtime_event_t;

/**
 * @brief Current runtime RATE value and observation quality.
 *
 * `value`, when present, uses the active volume unit and active RATE time base
 * from the runtime-owned measurement configuration. The value is current and
 * usable only when `quality` is `FREQUENCY_OBSERVATION_QUALITY_VALID`.
 *
 * `value_present` distinguishes a valid numeric zero from no calculated RATE.
 * `STALE` and `INVALID` may retain a prior value for diagnostics, but that
 * retained value is not current or usable. `UNAVAILABLE` has no value present.
 */
typedef struct
{
    double value;
    frequency_observation_quality_t quality;
    bool value_present;
} fmc_runtime_rate_state_t;

/**
 * @brief Caller-owned runtime state for one FMC instrument instance.
 *
 * The contained `service` is public so product tests and future wiring can
 * seed configuration deliberately. Normal consumers should prefer dispatch,
 * snapshot, and presentation-update APIs rather than editing nested state
 * directly.
 */
typedef struct
{
    fmc_service_t service;
    fmc_runtime_rate_state_t rate;
    bool presentation_update_pending;
} fmc_runtime_t;

/**
 * @brief Initialize one runtime instance and its owned FMC service.
 *
 * Existing runtime state is overwritten and any pending presentation update is
 * cleared.
 *
 * @param p_runtime Runtime object owned by the caller. `NULL` is ignored.
 */
void FMC_RUNTIME_Init(fmc_runtime_t *p_runtime);

/**
 * @brief Apply one product event to the runtime state.
 *
 * Nonzero pulse and reset events update the owned service through
 * `fmc_service`. A successful nonzero pulse or reset event marks presentation
 * update as pending so a caller can present visible values from a snapshot.
 * A zero pulse delta is a successful no-op: totals and presentation-pending
 * state remain unchanged. A presentation-invalidate event only sets that
 * pending flag.
 *
 * A frequency-result event consumes its by-value
 * `frequency_observation_result_t` payload. `VALID` invokes pure RATE
 * mathematics using the runtime-owned measurement configuration and the
 * supplied actual `elapsed_us`. Other recognized qualities update RATE quality
 * without invoking RATE mathematics. Every successfully applied frequency
 * result marks presentation update as pending.
 *
 * RATE calculation is atomic: a calculation error, invalid quality enum, or
 * other rejected frequency event leaves RATE and presentation-pending state
 * unchanged. Observation `INVALID` is never synthesized from a RATE error.
 *
 * Unsupported event kinds are rejected without modifying runtime state. Errors
 * returned by the owned service are propagated to the caller.
 *
 * @param p_runtime Initialized runtime object owned by the caller.
 * @param p_event Event to apply. The pointed object is only read during the
 *        call and may be stack allocated by the caller.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when any pointer is `NULL` or the event kind is
 *         invalid.
 * @return `FM_STATUS_ERANGE` when the owned service reports a range error,
 *         such as pulse-counter overflow, or RATE calculation reports invalid
 *         numeric input.
 * @return `FM_STATUS_ENOTSUP` when RATE calculation reports a valid but
 *         currently unsupported measurement configuration.
 */
fm_status_t FMC_RUNTIME_Dispatch(fmc_runtime_t *p_runtime,
                                 const fmc_runtime_event_t *p_event);

/**
 * @brief Copy current RATE value, presence, and quality together.
 *
 * The returned state owns no pointers into `p_runtime`; later dispatches do
 * not change a previously returned value.
 *
 * @param p_runtime Initialized runtime object to inspect.
 * @param p_rate_state Caller-owned output state.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when any pointer is `NULL`.
 */
fm_status_t FMC_RUNTIME_GetRateState(
    const fmc_runtime_t *p_runtime,
    fmc_runtime_rate_state_t *p_rate_state);

/**
 * @brief Copy the live FMC service state and derived visible values.
 *
 * The snapshot owns no pointers into `p_runtime`; later runtime dispatches do
 * not change a previously returned snapshot.
 *
 * @param p_runtime Initialized runtime object to copy from.
 * @param p_snapshot Caller-owned output snapshot.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when any pointer is `NULL`.
 * @return `FM_STATUS_ERANGE` when service measurement configuration is outside
 *         the supported numeric range.
 * @return `FM_STATUS_ENOTSUP` when service measurement configuration is valid
 *         but not supported by the current unit-conversion slice.
 */
fm_status_t FMC_RUNTIME_GetSnapshot(const fmc_runtime_t *p_runtime,
                                    fmc_service_snapshot_t *p_snapshot);

/**
 * @brief Report whether presentation should update from a fresh snapshot.
 *
 * The flag stays set until the caller clears it after handling the update.
 *
 * @param p_runtime Initialized runtime object to inspect.
 *
 * @return `true` when a presentation update is pending.
 * @return `false` when no presentation update is pending or `p_runtime` is
 *         `NULL`.
 */
bool FMC_RUNTIME_PresentationUpdateIsPending(
    const fmc_runtime_t *p_runtime);

/**
 * @brief Clear a pending presentation update after it has been handled.
 *
 * @param p_runtime Initialized runtime object owned by the caller.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when `p_runtime` is `NULL`.
 */
fm_status_t FMC_RUNTIME_ClearPresentationUpdatePending(
    fmc_runtime_t *p_runtime);

#endif /* FMC_RUNTIME_H */
