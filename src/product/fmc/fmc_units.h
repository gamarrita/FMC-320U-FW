/**
 * @file    fmc_units.h
 * @brief   FMC product unit policy helpers.
 *
 * This module owns product-specific unit conversion policy. It does not own
 * pulse counters, totalizers, rate windows, LCD labels, persistence, or RTOS
 * state.
 */
#ifndef FMC_UNITS_H
#define FMC_UNITS_H

#include <stdbool.h>

#include "fmc_model.h"
#include "fm_status.h"

typedef enum
{
    /** Unit has a physical liters-per-unit conversion. */
    FMC_UNITS_VOLUME_UNIT_KIND_PHYSICAL = 0,
    /** Unit uses the loaded calibration as already expressed in that unit. */
    FMC_UNITS_VOLUME_UNIT_KIND_ONE_TO_ONE
} fmc_units_volume_unit_kind_t;

/**
 * @brief Return true when the volume-unit enum value is part of this product.
 *
 * This is a pure query. It has no initialization requirement and no side
 * effects.
 *
 * @param p_unit Volume-unit enum value to check.
 *
 * @return `true` when the value is a supported product unit.
 * @return `false` otherwise.
 */
bool FMC_UNITS_IsVolumeUnitValid(fmc_model_volume_unit_t p_unit);

/**
 * @brief Recover an invalid or corrupt volume-unit value to a safe default.
 *
 * Current recovery policy:
 * - valid units are returned unchanged
 * - invalid units are normalized to liters
 *
 * @param p_unit Volume-unit enum value to normalize.
 *
 * @return `p_unit` when it is valid.
 * @return `FMC_MODEL_VOLUME_UNIT_L` when `p_unit` is invalid.
 */
fmc_model_volume_unit_t FMC_UNITS_NormalizeVolumeUnit(
    fmc_model_volume_unit_t p_unit);

/**
 * @brief Classify a valid volume unit by conversion policy.
 *
 * Physical units are converted from the liter calibration path. One-to-one
 * units use the loaded calibration as already expressed in the desired unit.
 *
 * @param p_unit Unit to classify.
 * @param p_kind Caller-owned output classification.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when `p_kind` is `NULL` or `p_unit` is invalid.
 */
fm_status_t FMC_UNITS_GetVolumeUnitKind(fmc_model_volume_unit_t p_unit,
                                        fmc_units_volume_unit_kind_t *p_kind);

/**
 * @brief Return the physical liters represented by one active unit.
 *
 * One-to-one product units return `1.0` because their conversion is performed
 * outside the computer by loading a calibration in the desired unit.
 *
 * @param p_unit Unit to query.
 * @param p_liters_per_unit Caller-owned output value in liters per unit.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when `p_liters_per_unit` is `NULL` or `p_unit`
 *         is invalid.
 */
fm_status_t FMC_UNITS_GetLitersPerUnit(fmc_model_volume_unit_t p_unit,
                                       double *p_liters_per_unit);

/**
 * @brief Calculate pulses per active volume unit.
 *
 * Current limitation:
 * - calibration in liters is supported
 * - other valid calibration units are reserved for a future calibration-unit
 *   refactor and currently return `FM_STATUS_ENOTSUP`
 *
 * Invalid active units are normalized to liters before calculation.
 *
 * @param p_measurement Measurement configuration to evaluate.
 * @param p_pulses_per_active_unit Caller-owned output value, in pulses per
 *                                 active volume unit.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` when any pointer is `NULL`.
 * @return `FM_STATUS_ERANGE` when calibration pulses per unit is outside the
 *         product-supported range.
 * @return `FM_STATUS_ENOTSUP` when the calibration unit is valid but not
 *         supported by the current conversion slice.
 */
fm_status_t FMC_UNITS_CalcPulsesPerActiveUnit(
    const fmc_model_measurement_t *p_measurement,
    double *p_pulses_per_active_unit);

#endif /* FMC_UNITS_H */
