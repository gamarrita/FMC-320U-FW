/**
 * @file    fmc_volume.h
 * @brief   Pure FMC accumulated-volume calculation helpers.
 *
 * This module converts pulse-backed ACM/TTL state into active volume units
 * using the current measurement configuration.
 *
 * It intentionally does not:
 * - store or cache visible volumes,
 * - reset counters or enforce reset authorization,
 * - choose display decimals,
 * - format strings,
 * - write LCD output,
 * - own persistence, acquisition, RTOS state, or UI flow.
 */
#ifndef FMC_VOLUME_H
#define FMC_VOLUME_H

#include "fmc_model.h"
#include "fm_status.h"

/**
 * @brief Calculate visible volume from one pulse-backed total state.
 *
 * The result uses the active volume unit from the measurement configuration.
 * The conversion policy is delegated to `fmc_units.*`.
 *
 * Conceptually:
 * `volume = total pulses / pulses per active volume unit`
 *
 * @param p_measurement Active measurement configuration.
 * @param p_total Pulse-backed total state to convert.
 * @param p_volume Output volume in active volume units.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when any pointer is `NULL`.
 * @return `FM_STATUS_ERANGE` when the measurement configuration is outside
 *         supported numeric range.
 * @return `FM_STATUS_ENOTSUP` when the measurement configuration is valid but
 *         not supported by the current unit-conversion slice.
 */
fm_status_t FMC_VOLUME_CalcFromTotal(
    const fmc_model_measurement_t *p_measurement,
    const fmc_model_total_state_t *p_total,
    double *p_volume);

/**
 * @brief Calculate visible volume for one total role in a full FMC model.
 *
 * This is a convenience wrapper over `FMC_VOLUME_CalcFromTotal()`.
 * The selected role determines whether the ACM or TTL pulse counter is used.
 *
 * @param p_model FMC model containing measurement and total state.
 * @param p_role Total role to calculate.
 * @param p_volume Output volume in active volume units.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when arguments are invalid.
 * @return `FM_STATUS_ERANGE` when the measurement configuration is outside
 *         supported numeric range.
 * @return `FM_STATUS_ENOTSUP` when the measurement configuration is valid but
 *         not supported by the current unit-conversion slice.
 */
fm_status_t FMC_VOLUME_CalcByTotalRole(const fmc_model_t *p_model,
                                       fmc_model_total_t p_role,
                                       double *p_volume);

#endif /* FMC_VOLUME_H */
