/**
 * @file    fmc_volume.c
 * @brief   Pure FMC accumulated-volume calculation implementation.
 */
#include "fmc_volume.h"

#include <stddef.h>

#include "fmc_units.h"

fm_status_t FMC_VOLUME_CalcFromTotal(
    const fmc_model_measurement_t *p_measurement,
    const fmc_model_total_state_t *p_total,
    double *p_volume)
{
    double pulses_per_active_unit;
    fm_status_t status;

    if ((p_measurement == NULL) || (p_total == NULL) || (p_volume == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    status = FMC_UNITS_CalcPulsesPerActiveUnit(p_measurement,
                                               &pulses_per_active_unit);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    *p_volume = (double) p_total->pulses / pulses_per_active_unit;

    return FM_STATUS_OK;
}

fm_status_t FMC_VOLUME_CalcByTotalRole(const fmc_model_t *p_model,
                                       fmc_model_total_t p_role,
                                       double *p_volume)
{
    const fmc_model_total_state_t *total;

    if ((p_model == NULL) || (p_volume == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    total = FMC_MODEL_GetTotalStateConst(p_model, p_role);
    if (total == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    return FMC_VOLUME_CalcFromTotal(&p_model->measurement, total, p_volume);
}
