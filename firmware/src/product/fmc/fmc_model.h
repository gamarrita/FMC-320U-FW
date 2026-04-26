/**
 * @file    fmc_model.h
 * @brief   Public FMC domain model contract candidate.
 *
 * This header defines the semantic model of the flow computer domain.
 *
 * The intent of this layer is to define:
 * - total roles and their reset policy
 * - the shared measurement configuration behind totals and rate views
 * - the raw pulse counters that back total recomputation
 * - product constraints that shape valid canonical state
 *
 * This layer intentionally does not define:
 * - LCD rows, alpha fields, indicators, or strings
 * - menu/setup flow
 * - persistence layout
 * - acquisition or pulse/capture math
 * - log storage format
 * - unit-conversion or derived visible-volume queries
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
#ifndef FMC_MODEL_H
#define FMC_MODEL_H

#include <stdint.h>

typedef enum
{
    FMC_MODEL_OK = 0,
    FMC_MODEL_EINVAL,
    FMC_MODEL_ERANGE,
    FMC_MODEL_ESTATE,
    FMC_MODEL_ENOTSUP
} fmc_model_status_t;

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
    FMC_MODEL_TOTAL_ACM = 0,
    FMC_MODEL_TOTAL_TTL
} fmc_model_total_t;

/**
 * @brief Reset privilege required by one total role.
 *
 * This describes semantic policy only.
 * It does not define the UI or authentication mechanism that enforces it.
 */
typedef enum
{
    FMC_MODEL_RESET_USER_ALLOWED = 0,
    FMC_MODEL_RESET_PRIVILEGED
} fmc_model_reset_policy_t;

/**
 * @brief Active volume unit of the instrument.
 *
 * For this product line, ACM, TTL, and RATE share the same active volume unit.
 *
 * Display strings such as `BR` or `--` are presentation concerns and are not
 * owned by this model enum.
 *
 * `FMC_MODEL_VOLUME_UNIT_UNSUPPORTED` preserves the product-level placeholder
 * for a selected unit that is not supported by the physical conversion table.
 * It is still operational: the active conversion factor is `1`, and the loaded
 * calibration is expected to already match the desired custom unit.
 */
typedef enum
{
    FMC_MODEL_VOLUME_UNIT_UNSUPPORTED = 0,
    FMC_MODEL_VOLUME_UNIT_L,
    FMC_MODEL_VOLUME_UNIT_M3,
    FMC_MODEL_VOLUME_UNIT_GAL_US,
    FMC_MODEL_VOLUME_UNIT_BBL_US,
    FMC_MODEL_VOLUME_UNIT_KG,
    /** Equivalent cubic meter: product relation 1 L = 1 equivalent m3. */
    FMC_MODEL_VOLUME_UNIT_EQUIV_M3
} fmc_model_volume_unit_t;

/**
 * @brief Default calibration unit used by the normal physical-conversion path.
 *
 * The model keeps calibration unit explicit to avoid hiding the legacy liter
 * anchor, but this first contract does not implement operator-selectable
 * calibration units.
 */
#define FMC_MODEL_CALIBRATION_UNIT_DEFAULT    FMC_MODEL_VOLUME_UNIT_L

/**
 * @brief Product range for editable calibration factors.
 *
 * The legacy UI stored calibration as an unsigned fixed-point value with three
 * decimals and eight editable digits. The semantic range is preserved here even
 * though this contract exposes the value as `double`.
 *
 * Unit: pulses per calibration volume unit.
 */
#define FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_MIN        1.0
#define FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_MAX        99999.999
#define FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_DEFAULT    1.0

/**
 * @brief Active time base used by the instantaneous rate view.
 *
 * The time base qualifies RATE only.
 * It does not create a separate volume-unit family.
 */
typedef enum
{
    FMC_MODEL_TIME_BASE_SECOND = 0,
    FMC_MODEL_TIME_BASE_MINUTE,
    FMC_MODEL_TIME_BASE_HOUR,
    FMC_MODEL_TIME_BASE_DAY
} fmc_model_time_base_t;

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
} fmc_model_total_state_t;

/**
 * @brief Shared measurement configuration behind totals and rate.
 *
 * Canonical ownership rules:
 * - calibration is stored as pulses per calibration volume unit
 * - editable calibration values use the
 *   `FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_*` product range
 * - the UI does not currently expose calibration volume-unit selection
 * - volume unit is shared by ACM, TTL, and RATE
 * - time base qualifies RATE
 * - known physical conversions are normally derived from a liter-anchored
 *   calibration
 * - unsupported/custom units use conversion factor `1`; in that case the
 *   loaded calibration is expected to already be pulses per desired custom unit
 *
 * Derived operative pulses-per-active-unit views should not be stored here.
 * They belong to unit/rate calculation helpers over this model state.
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
    fmc_model_volume_unit_t calibration_volume_unit;
    fmc_model_volume_unit_t active_volume_unit;
    fmc_model_time_base_t active_time_base;
} fmc_model_measurement_t;

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
 * - visible totals, operative pulses-per-active-unit, and rate are derived
 *   behavior, not stored canonical state
 */
typedef struct
{
    fmc_model_measurement_t measurement;
    fmc_model_total_state_t acm;
    fmc_model_total_state_t ttl;
} fmc_model_t;

/**
 * @brief Initialize one FMC model value with conservative defaults.
 *
 * This helper prepares a copyable model value only.
 * It does not imply persistence, acquisition startup, or LCD output.
 */
void FMC_MODEL_Init(fmc_model_t *p_model);

/**
 * @brief Return the reset policy associated with one total role.
 *
 * Intended current policy:
 * - ACM: user allowed
 * - TTL: privileged UI/service flow
 *
 * This is informational policy for callers.
 * `FMC_MODEL_ResetTotal()` does not authenticate or authorize the caller.
 */
fmc_model_reset_policy_t FMC_MODEL_GetResetPolicy(fmc_model_total_t p_total);

/**
 * @brief Return a pointer to one total state selected by role.
 *
 * This helper keeps ACM and TTL modeled as the same primitive state shape.
 *
 * @return Pointer to the selected total state on success.
 * @return NULL when arguments are invalid.
 */
fmc_model_total_state_t *FMC_MODEL_GetTotalState(fmc_model_t *p_model,
                                                 fmc_model_total_t p_total);

/**
 * @brief Return a read-only pointer to one total state selected by role.
 *
 * @return Pointer to the selected total state on success.
 * @return NULL when arguments are invalid.
 */
const fmc_model_total_state_t *FMC_MODEL_GetTotalStateConst(
    const fmc_model_t *p_model,
    fmc_model_total_t p_total);

/**
 * @brief Reset one total according to its semantic reset policy.
 *
 * This operation modifies the total backing pulse counter.
 * UI, password, or privilege enforcement remains outside this contract. In
 * practice, the same primitive reset operation can serve ACM and TTL once the
 * caller has reached the correct UI/service flow.
 */
fmc_model_status_t FMC_MODEL_ResetTotal(fmc_model_t *p_model,
                                        fmc_model_total_t p_total);

#endif /* FMC_MODEL_H */
