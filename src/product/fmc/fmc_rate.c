/**
 * @file    fmc_rate.c
 * @brief   Pure FMC rate calculation implementation.
 */
#include "fmc_rate.h"

#include <stddef.h>

#include "fmc_units.h"

/* Return the number of seconds represented by one valid rate time base. */
static fm_status_t fmc_rate_get_seconds_per_time_base_(
    fmc_model_time_base_t p_time_base,
    double *p_seconds_per_time_base);

fm_status_t FMC_RATE_Calc(const fmc_model_measurement_t *p_measurement,
                          uint64_t p_pulse_delta,
                          double p_elapsed_seconds,
                          double *p_rate)
{
    double pulses_per_active_unit;
    double seconds_per_time_base;
    fm_status_t status;

    if ((p_measurement == NULL) || (p_rate == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    if (!(p_elapsed_seconds > 0.0))
    {
        return FM_STATUS_ERANGE;
    }

    status = FMC_UNITS_CalcPulsesPerActiveUnit(
        p_measurement,
        &pulses_per_active_unit);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = fmc_rate_get_seconds_per_time_base_(
        p_measurement->active_time_base,
        &seconds_per_time_base);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    *p_rate = ((double) p_pulse_delta / pulses_per_active_unit) *
              (seconds_per_time_base / p_elapsed_seconds);

    return FM_STATUS_OK;
}

static fm_status_t fmc_rate_get_seconds_per_time_base_(
    fmc_model_time_base_t p_time_base,
    double *p_seconds_per_time_base)
{
    switch (p_time_base)
    {
    case FMC_MODEL_TIME_BASE_SECOND:
        *p_seconds_per_time_base = 1.0;
        return FM_STATUS_OK;

    case FMC_MODEL_TIME_BASE_MINUTE:
        *p_seconds_per_time_base = 60.0;
        return FM_STATUS_OK;

    case FMC_MODEL_TIME_BASE_HOUR:
        *p_seconds_per_time_base = 3600.0;
        return FM_STATUS_OK;

    case FMC_MODEL_TIME_BASE_DAY:
        *p_seconds_per_time_base = 86400.0;
        return FM_STATUS_OK;

    default:
        return FM_STATUS_EINVAL;
    }
}
