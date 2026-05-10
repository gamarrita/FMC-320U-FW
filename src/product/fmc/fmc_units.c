/**
 * @file    fmc_units.c
 * @brief   FMC product unit policy implementation.
 */
#include "fmc_units.h"

#include <stddef.h>

/* Returns true for units whose conversion is a physical liters-per-unit value. */
static bool fmc_units_is_physical_conversion_(fmc_model_volume_unit_t p_unit);

/* Returns true for valid units whose conversion is intentionally 1:1. */
static bool fmc_units_is_one_to_one_(fmc_model_volume_unit_t p_unit);

bool FMC_UNITS_IsVolumeUnitValid(fmc_model_volume_unit_t p_unit)
{
    switch (p_unit)
    {
    case FMC_MODEL_VOLUME_UNIT_CUSTOM:
    case FMC_MODEL_VOLUME_UNIT_L:
    case FMC_MODEL_VOLUME_UNIT_M3:
    case FMC_MODEL_VOLUME_UNIT_GAL_US:
    case FMC_MODEL_VOLUME_UNIT_BBL_US:
    case FMC_MODEL_VOLUME_UNIT_KG:
    case FMC_MODEL_VOLUME_UNIT_EQUIV_M3:
        return true;

    default:
        return false;
    }
}

fmc_model_volume_unit_t FMC_UNITS_NormalizeVolumeUnit(
    fmc_model_volume_unit_t p_unit)
{
    if (FMC_UNITS_IsVolumeUnitValid(p_unit))
    {
        return p_unit;
    }

    return FMC_MODEL_VOLUME_UNIT_L;
}

fm_status_t FMC_UNITS_GetVolumeUnitKind(fmc_model_volume_unit_t p_unit,
                                        fmc_units_volume_unit_kind_t *p_kind)
{
    if (p_kind == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    if (fmc_units_is_physical_conversion_(p_unit))
    {
        *p_kind = FMC_UNITS_VOLUME_UNIT_KIND_PHYSICAL;
        return FM_STATUS_OK;
    }

    if (fmc_units_is_one_to_one_(p_unit))
    {
        *p_kind = FMC_UNITS_VOLUME_UNIT_KIND_ONE_TO_ONE;
        return FM_STATUS_OK;
    }

    return FM_STATUS_EINVAL;
}

fm_status_t FMC_UNITS_GetLitersPerUnit(fmc_model_volume_unit_t p_unit,
                                       double *p_liters_per_unit)
{
    if (p_liters_per_unit == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    switch (p_unit)
    {
    case FMC_MODEL_VOLUME_UNIT_CUSTOM:
    case FMC_MODEL_VOLUME_UNIT_L:
    case FMC_MODEL_VOLUME_UNIT_KG:
    case FMC_MODEL_VOLUME_UNIT_EQUIV_M3:
        *p_liters_per_unit = 1.0;
        return FM_STATUS_OK;

    case FMC_MODEL_VOLUME_UNIT_M3:
        *p_liters_per_unit = 1000.0;
        return FM_STATUS_OK;

    case FMC_MODEL_VOLUME_UNIT_GAL_US:
        *p_liters_per_unit = 3.78541;
        return FM_STATUS_OK;

    case FMC_MODEL_VOLUME_UNIT_BBL_US:
        *p_liters_per_unit = 158.987304;
        return FM_STATUS_OK;

    default:
        return FM_STATUS_EINVAL;
    }
}

fm_status_t FMC_UNITS_CalcPulsesPerActiveUnit(
    const fmc_model_measurement_t *p_measurement,
    double *p_pulses_per_active_unit)
{
    fmc_model_volume_unit_t active_unit;
    double liters_per_unit;
    fm_status_t status;

    if ((p_measurement == NULL) || (p_pulses_per_active_unit == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    if ((p_measurement->calibration_pulses_per_unit <
         FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_MIN) ||
        (p_measurement->calibration_pulses_per_unit >
         FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_MAX))
    {
        return FM_STATUS_ERANGE;
    }

    if (FMC_UNITS_NormalizeVolumeUnit(
            p_measurement->calibration_volume_unit) != FMC_MODEL_VOLUME_UNIT_L)
    {
        return FM_STATUS_ENOTSUP;
    }

    active_unit = FMC_UNITS_NormalizeVolumeUnit(
        p_measurement->active_volume_unit);

    status = FMC_UNITS_GetLitersPerUnit(active_unit, &liters_per_unit);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    *p_pulses_per_active_unit =
        p_measurement->calibration_pulses_per_unit * liters_per_unit;

    return FM_STATUS_OK;
}

static bool fmc_units_is_physical_conversion_(fmc_model_volume_unit_t p_unit)
{
    switch (p_unit)
    {
    case FMC_MODEL_VOLUME_UNIT_L:
    case FMC_MODEL_VOLUME_UNIT_M3:
    case FMC_MODEL_VOLUME_UNIT_GAL_US:
    case FMC_MODEL_VOLUME_UNIT_BBL_US:
        return true;

    default:
        return false;
    }
}

static bool fmc_units_is_one_to_one_(fmc_model_volume_unit_t p_unit)
{
    switch (p_unit)
    {
    case FMC_MODEL_VOLUME_UNIT_CUSTOM:
    case FMC_MODEL_VOLUME_UNIT_KG:
    case FMC_MODEL_VOLUME_UNIT_EQUIV_M3:
        return true;

    default:
        return false;
    }
}
