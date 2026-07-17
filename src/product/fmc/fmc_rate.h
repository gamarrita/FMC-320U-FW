/**
 * @file    fmc_rate.h
 * @brief   Pure FMC rate calculation helpers.
 *
 * This module converts an already captured pulse/time window into an active
 * volume-unit rate. It does not capture pulses, read clocks, retain runtime
 * state, format values, or own synchronization.
 */
#ifndef FMC_RATE_H
#define FMC_RATE_H

#include <stdint.h>

#include "fmc_model.h"
#include "fm_status.h"

/**
 * @brief Calculate rate from a pulse delta and elapsed time window.
 *
 * The result uses the active volume unit and active time base from the
 * measurement configuration. For example, liters plus a minute time base
 * produces liters per minute.
 *
 * @param p_measurement Active measurement configuration.
 * @param p_pulse_delta Pulses observed during the window.
 * @param p_elapsed_seconds Positive duration of the window in seconds.
 * @param p_rate Output rate in active volume units per active time base.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when any pointer is `NULL`, the active time base
 *         is invalid, or measurement validation reports invalid arguments.
 * @return `FM_STATUS_ERANGE` when `p_elapsed_seconds` is not positive or
 *         measurement validation reports an out-of-range calibration.
 * @return `FM_STATUS_ENOTSUP` when measurement validation reports an
 *         unsupported calibration unit.
 */
fm_status_t FMC_RATE_Calc(const fmc_model_measurement_t *p_measurement,
                          uint64_t p_pulse_delta,
                          double p_elapsed_seconds,
                          double *p_rate);

#endif /* FMC_RATE_H */
