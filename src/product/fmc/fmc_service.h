/**
 * @file    fmc_service.h
 * @brief   FMC live-state service boundary.
 *
 * `fmc_service` owns one caller-allocated live FMC model instance. It is the
 * boundary that acquisition/runtime code updates and presentation/runtime code
 * reads through snapshots.
 *
 * The service performs no HAL access, RTOS calls, dynamic allocation, logging,
 * or blocking waits. It does not capture pulses, enforce reset authorization,
 * persist state, format display text, or dispatch UI events.
 *
 * Call `FMC_SERVICE_Init()` before first use. The module does not detect an
 * uninitialized service object.
 *
 * Concurrency:
 * - functions are not internally synchronized
 * - callers must serialize access when the same service can be touched from
 *   multiple contexts
 * - no public function is designed to be called concurrently with another
 *   function on the same service instance
 */
#ifndef FMC_SERVICE_H
#define FMC_SERVICE_H

#include <stdint.h>

#include "fmc_model.h"
#include "fm_status.h"

/**
 * @brief Owner of one live FMC state instance.
 *
 * The caller owns the storage and lifetime of this object. The contained
 * `model` remains visible so low-level tests and future runtime wiring can
 * inspect or seed state deliberately; normal consumers should prefer the public
 * service operations and snapshots.
 */
typedef struct
{
    fmc_model_t model;
} fmc_service_t;

/**
 * @brief Read-only copy of the current service state plus derived views.
 *
 * `model` is a by-value copy of the canonical state at the time the snapshot is
 * taken. `acm_volume` and `ttl_volume` are derived from that copy's measurement
 * configuration and pulse totals, in the active volume unit configured in
 * `model.measurement.active_volume_unit`.
 */
typedef struct
{
    fmc_model_t model;
    double acm_volume;
    double ttl_volume;
} fmc_service_snapshot_t;

/**
 * @brief Initialize one service instance with FMC model defaults.
 *
 * Existing state in `p_service` is overwritten.
 *
 * @param p_service Service object owned by the caller. `NULL` is ignored.
 */
void FMC_SERVICE_Init(fmc_service_t *p_service);

/**
 * @brief Add an acquired pulse delta to both runtime totals.
 *
 * `p_pulse_delta` is a raw pulse count observed since the last accepted update.
 * Both ACM and TTL accumulate measured flow; their reset policy differs, not
 * their accumulation source.
 *
 * On overflow or invalid arguments, neither total is modified.
 *
 * @param p_service Initialized service object owned by the caller.
 * @param p_pulse_delta Raw pulse count to add to ACM and TTL.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when `p_service` is `NULL`.
 * @return `FM_STATUS_ERANGE` if either pulse counter would overflow.
 */
fm_status_t FMC_SERVICE_AddPulseDelta(fmc_service_t *p_service,
                                      uint64_t p_pulse_delta);

/**
 * @brief Reset one total role after the caller has enforced reset policy.
 *
 * This function does not authenticate the reset request. Callers must apply the
 * policy reported by `FMC_MODEL_GetResetPolicy()` before resetting privileged
 * totals.
 *
 * @param p_service Initialized service object owned by the caller.
 * @param p_total Total role to reset.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when `p_service` is `NULL` or `p_total` is not a
 *         valid total role.
 */
fm_status_t FMC_SERVICE_ResetTotal(fmc_service_t *p_service,
                                   fmc_model_total_t p_total);

/**
 * @brief Copy the live model and derived visible total values.
 *
 * The snapshot owns no pointers into `p_service`; later service updates do not
 * change a previously returned snapshot.
 *
 * Derived volumes use the current measurement configuration. Measurement
 * validation errors from the unit/volume helpers are returned to the caller.
 *
 * @param p_service Initialized service object to copy from.
 * @param p_snapshot Caller-owned output snapshot.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when any pointer is `NULL`.
 * @return `FM_STATUS_ERANGE` when measurement configuration is outside the
 *         supported numeric range.
 * @return `FM_STATUS_ENOTSUP` when measurement configuration is valid but not
 *         supported by the current unit-conversion slice.
 */
fm_status_t FMC_SERVICE_GetSnapshot(const fmc_service_t *p_service,
                                    fmc_service_snapshot_t *p_snapshot);

#endif /* FMC_SERVICE_H */
