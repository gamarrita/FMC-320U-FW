/**
 * @file    fmc_runtime.h
 * @brief   RTOS-neutral FMC runtime event boundary.
 *
 * `fmc_runtime` owns one `fmc_service_t` instance and applies product-level
 * events to it. It is the boundary a future main loop, scheduler, or RTOS
 * thread can drive without exposing HAL, LCD, keyboard, timer, or queue types
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
 *   an ISR should publish input through a serialized adapter before dispatch
 */
#ifndef FMC_RUNTIME_H
#define FMC_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

#include "fmc_input.h"
#include "fmc_service.h"
#include "fm_status.h"

/**
 * @brief Product-level events accepted by the minimal runtime boundary.
 *
 * Events describe observable product requests, not their hardware source. For
 * example, presentation can be invalidated by a key action, an edit operation,
 * or a periodic tick.
 */
typedef enum
{
    /** No operation; useful when a caller wants an explicit idle dispatch. */
    FMC_RUNTIME_EVENT_NONE = 0,
    /** Add `pulse_delta` raw sensor pulses to the live totals. */
    FMC_RUNTIME_EVENT_PULSE_DELTA,
    /** Reset the ACM total after the caller has authorized the operation. */
    FMC_RUNTIME_EVENT_RESET_ACM,
    /** Reset the TTL total after the caller has authorized the operation. */
    FMC_RUNTIME_EVENT_RESET_TTL,
    /** Deliver one already recognized semantic product input event. */
    FMC_RUNTIME_EVENT_INPUT,
    /** Mark the current presentation view as no longer up to date. */
    FMC_RUNTIME_EVENT_PRESENTATION_INVALIDATE
} fmc_runtime_event_kind_t;

/**
 * @brief One event consumed by `FMC_RUNTIME_Dispatch()`.
 *
 * The active payload is selected by `kind`:
 * - `data.pulse_delta` for `FMC_RUNTIME_EVENT_PULSE_DELTA`
 * - `data.input` for `FMC_RUNTIME_EVENT_INPUT`
 *
 * Payload fields for other event kinds are ignored.
 */
typedef struct
{
    fmc_runtime_event_kind_t kind;
    union
    {
        uint64_t pulse_delta;
        fmc_input_event_t input;
    } data;
} fmc_runtime_event_t;

/**
 * @brief Caller-owned runtime state for one FMC instrument instance.
 *
 * The contained `service` is public so product tests and future wiring can
 * seed configuration deliberately. Normal consumers should prefer dispatch,
 * snapshot, and presentation-update APIs rather than editing nested state
 * directly.
 *
 * `last_input` is valid only when `last_input_valid` is `true`. It preserves
 * the most recent accepted semantic input for the current minimal runtime
 * slice; it is not a queue and does not implement menu navigation.
 */
typedef struct
{
    fmc_service_t service;
    fmc_input_event_t last_input;
    bool presentation_update_pending;
    bool last_input_valid;
} fmc_runtime_t;

/**
 * @brief Initialize one runtime instance and its owned FMC service.
 *
 * Existing runtime state is overwritten and any pending presentation update is
 * cleared. No semantic input is considered accepted after initialization.
 *
 * @param p_runtime Runtime object owned by the caller. `NULL` is ignored.
 */
void FMC_RUNTIME_Init(fmc_runtime_t *p_runtime);

/**
 * @brief Apply one product event to the runtime state.
 *
 * Pulse and reset events update the owned service through `fmc_service`. A
 * successful pulse or reset event marks presentation update as pending so a
 * caller can present visible values from a snapshot. A presentation-invalidate
 * event only sets that pending flag.
 *
 * A valid input event preserves key/action identity in `last_input` and marks
 * presentation update as pending. The current slice does not map input to menu
 * navigation, backlight, wake, reset, or edit behavior.
 *
 * Unsupported event kinds are rejected without modifying runtime state. Errors
 * returned by the owned service are propagated to the caller.
 *
 * @param p_runtime Initialized runtime object owned by the caller.
 * @param p_event Event to apply. The pointed object is only read during the
 *        call and may be stack allocated by the caller.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when any pointer is `NULL`, the event kind is
 *         invalid, or an input key/action combination is invalid.
 * @return `FM_STATUS_ERANGE` when the owned service reports a range error,
 *         such as pulse-counter overflow.
 */
fm_status_t FMC_RUNTIME_Dispatch(fmc_runtime_t *p_runtime,
                                 const fmc_runtime_event_t *p_event);

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
