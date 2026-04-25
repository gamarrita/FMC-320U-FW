/**
 * @file    fm_fmc_model.h
 * @brief   Public FMC domain model contract candidate.
 *
 * This header defines the semantic model of the flow computer domain.
 *
 * The intent of this layer is to define:
 * - total roles and their reset policy
 * - the shared measurement configuration behind totals and rate views
 * - the raw pulse counters that back total recomputation
 * - derived engineering queries over the canonical state
 *
 * This layer intentionally does not define:
 * - LCD rows, alpha fields, indicators, or strings
 * - menu/setup flow
 * - persistence layout
 * - acquisition or pulse/capture math
 * - log storage format
 *
 * Design rule:
 * - the model types are plain copyable value types
 * - this is intentional so the product can later keep multiple copies of the
 *   same model shape:
 *   - active environment
 *   - edit buffer
 *   - factory/default copy
 *
 * Numeric representation note:
 * - the semantic contract does not expose scaled fixed-point types
 * - implementations may still use scaled math internally if needed
 * - editable/persistent decimal representation may later move to a dedicated
 *   numeric library
 */
#ifndef FM_FMC_MODEL_H
#define FM_FMC_MODEL_H

#include <stdint.h>

typedef enum
{
    FM_FMC_MODEL_OK = 0,
    FM_FMC_MODEL_EINVAL,
    FM_FMC_MODEL_ERANGE,
    FM_FMC_MODEL_ESTATE,
    FM_FMC_MODEL_ENOTSUP
} fm_fmc_model_status_t;

/**
 * @brief Named total roles exposed by the instrument.
 *
 * `ACM` and `TTL` are intentionally modeled as two roles over the same
 * primitive total state shape.
 *
 * Their main semantic difference is reset policy, not data shape.
 */
typedef enum
{
    FM_FMC_MODEL_TOTAL_ACM = 0,
    FM_FMC_MODEL_TOTAL_TTL
} fm_fmc_model_total_t;

/**
 * @brief Reset privilege required by one total role.
 *
 * This describes semantic policy only.
 * It does not define the UI or authentication mechanism that enforces it.
 */
typedef enum
{
    FM_FMC_MODEL_RESET_USER_ALLOWED = 0,
    FM_FMC_MODEL_RESET_PRIVILEGED
} fm_fmc_model_reset_policy_t;

/**
 * @brief Active volume unit of the instrument.
 *
 * For this product line, ACM, TTL, and RATE share the same active volume unit.
 *
 * Display strings such as `BR` or `--` are presentation concerns and are not
 * owned by this model enum.
 *
 * `FM_FMC_MODEL_VOLUME_UNIT_UNSUPPORTED` preserves the product-level placeholder
 * for a selected unit that is not supported by the physical conversion table.
 * It is still operational: the active conversion factor is `1`, and the loaded
 * calibration is expected to already match the desired custom unit.
 */
typedef enum
{
    FM_FMC_MODEL_VOLUME_UNIT_UNSUPPORTED = 0,
    FM_FMC_MODEL_VOLUME_UNIT_L,
    FM_FMC_MODEL_VOLUME_UNIT_M3,
    FM_FMC_MODEL_VOLUME_UNIT_GAL_US,
    FM_FMC_MODEL_VOLUME_UNIT_BBL_US,
    FM_FMC_MODEL_VOLUME_UNIT_KG,
    FM_FMC_MODEL_VOLUME_UNIT_EQUIV_M3
} fm_fmc_model_volume_unit_t;

/**
 * @brief Default calibration unit used by the normal physical-conversion path.
 *
 * The model keeps calibration unit explicit to avoid hiding the legacy liter
 * anchor, but this first contract does not implement operator-selectable
 * calibration units.
 */
#define FM_FMC_MODEL_CALIBRATION_UNIT_DEFAULT    FM_FMC_MODEL_VOLUME_UNIT_L

/**
 * @brief Active time base used by the instantaneous rate view.
 *
 * The time base qualifies RATE only.
 * It does not create a separate volume-unit family.
 */
typedef enum
{
    FM_FMC_MODEL_TIME_BASE_SECOND = 0,
    FM_FMC_MODEL_TIME_BASE_MINUTE,
    FM_FMC_MODEL_TIME_BASE_HOUR,
    FM_FMC_MODEL_TIME_BASE_DAY
} fm_fmc_model_time_base_t;

/**
 * @brief Primitive total state shared by ACM and TTL.
 *
 * The pulse counter is the canonical state for one total role.
 * Visible volume is derived from this state and the active measurement
 * environment.
 *
 * Semantic rule:
 * - this is runtime backing state
 * - it is not editable configuration
 * - the visible total value should be queried or rendered as a derived view
 * - if unit or calibration changes, implementations may recompute visible
 *   totals from `pulses` and the active derived factor
 */
typedef struct
{
    uint64_t pulses;
} fm_fmc_model_total_state_t;

/**
 * @brief Shared measurement configuration behind totals and rate.
 *
 * Canonical ownership rules:
 * - calibration is stored as pulses per calibration volume unit
 * - the UI does not currently expose calibration volume-unit selection
 * - volume unit is shared by ACM, TTL, and RATE
 * - time base qualifies RATE
 * - known physical conversions are normally derived from a liter-anchored
 *   calibration
 * - unsupported/custom units use conversion factor `1`; in that case the
 *   loaded calibration is expected to already be pulses per desired custom unit
 *
 * Derived factors such as K-factor views should be treated as queries over this
 * configuration, not as equal-rank canonical state.
 *
 * Important distinction:
 * - the stored calibration may use a different conceptual unit than the active
 *   displayed unit
 * - the active environment factor is derived from calibration plus the active
 *   volume unit
 * - no general calibration-unit conversion matrix is implemented in this
 *   contract
 */
typedef struct
{
    double calibration_pulses_per_unit;
    fm_fmc_model_volume_unit_t calibration_volume_unit;
    fm_fmc_model_volume_unit_t active_volume_unit;
    fm_fmc_model_time_base_t active_time_base;
} fm_fmc_model_measurement_t;

/**
 * @brief Full copyable FMC model state.
 *
 * This shape is intended to be reused for:
 * - active environment
 * - edit buffer
 * - factory/default copies
 *
 * No hidden singleton ownership is assumed by this type.
 *
 * Ownership rule:
 * - `measurement` contains the active environment configuration used to derive
 *   runtime values
 * - `acm` and `ttl` contain runtime backing pulse counters
 * - visible totals and rate are derived behavior, not stored canonical state
 */
typedef struct
{
    fm_fmc_model_measurement_t measurement;
    fm_fmc_model_total_state_t acm;
    fm_fmc_model_total_state_t ttl;
} fm_fmc_model_t;

/**
 * @brief Initialize one FMC model value with conservative defaults.
 *
 * This helper prepares a copyable model value only.
 * It does not imply persistence, acquisition startup, or LCD output.
 */
void FM_FMC_ModelInit(fm_fmc_model_t *p_model);

/**
 * @brief Return the reset policy associated with one total role.
 *
 * Intended current policy:
 * - ACM: user allowed
 * - TTL: privileged UI/service flow
 *
 * This is informational policy for callers.
 * `FM_FMC_ModelResetTotal()` does not authenticate or authorize the caller.
 */
fm_fmc_model_reset_policy_t FM_FMC_ModelGetResetPolicy(
    fm_fmc_model_total_t p_total);

/**
 * @brief Return a pointer to one total state selected by role.
 *
 * This helper keeps ACM and TTL modeled as the same primitive state shape.
 *
 * @return Pointer to the selected total state on success.
 * @return NULL when arguments are invalid.
 */
fm_fmc_model_total_state_t *FM_FMC_ModelGetTotalState(
    fm_fmc_model_t *p_model,
    fm_fmc_model_total_t p_total);

/**
 * @brief Return a read-only pointer to one total state selected by role.
 *
 * @return Pointer to the selected total state on success.
 * @return NULL when arguments are invalid.
 */
const fm_fmc_model_total_state_t *FM_FMC_ModelGetTotalStateConst(
    const fm_fmc_model_t *p_model,
    fm_fmc_model_total_t p_total);

/**
 * @brief Reset one total according to its semantic reset policy.
 *
 * This operation modifies the total backing pulse counter.
 * UI, password, or privilege enforcement remains outside this contract. In
 * practice, the same primitive reset operation can serve ACM and TTL once the
 * caller has reached the correct UI/service flow.
 */
fm_fmc_model_status_t FM_FMC_ModelResetTotal(fm_fmc_model_t *p_model,
                                             fm_fmc_model_total_t p_total);

/**
 * @brief Return the derived K-factor for the active volume unit.
 *
 * This is a derived engineering query over the canonical calibration state.
 *
 * The result is expressed as pulses per active volume unit.
 *
 * This helper exists because visible totals and rate are expected to use an
 * active environment factor derived from:
 * - calibration in pulses per calibration unit
 * - calibration volume unit
 * - active volume unit
 */
fm_fmc_model_status_t FM_FMC_ModelGetActiveKFactor(
    const fm_fmc_model_t *p_model,
    double *p_pulses_per_unit);

/**
 * @brief Return the derived visible total volume for one total role.
 *
 * This is a derived query over:
 * - selected total pulse counter
 * - active K-factor
 *
 * The result is expressed in the active volume unit.
 */
fm_fmc_model_status_t FM_FMC_ModelGetTotalVolume(
    const fm_fmc_model_t *p_model,
    fm_fmc_model_total_t p_total,
    double *p_volume);

#endif /* FM_FMC_MODEL_H */
