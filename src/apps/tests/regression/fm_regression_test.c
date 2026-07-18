/**
 * @file    fm_regression_test.c
 * @brief   Repeatable verification app for pure model, unit, rate, volume,
 *          and display-format slices.
 */
#include "fm_regression_test.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "display_format.h"
#include "fm_board.h"
#include "fm_board_keyboard.h"
#include "fm_debug.h"
#include "fm_port_time.h"
#include "fmc_input.h"
#include "fmc_model.h"
#include "fmc_rate.h"
#include "fmc_runtime.h"
#include "fmc_service.h"
#include "fmc_units.h"
#include "fmc_volume.h"
#include "main.h"

#define FM_REGRESSION_TEST_IDLE_MS   1000U
#define FM_REGRESSION_TEST_EPSILON   0.000001

typedef enum
{
    FM_REGRESSION_TEST_CASE_INIT_DEFAULTS = 0,
    FM_REGRESSION_TEST_CASE_TOTALS,
    FM_REGRESSION_TEST_CASE_UNIT_VALIDITY,
    FM_REGRESSION_TEST_CASE_UNIT_KIND,
    FM_REGRESSION_TEST_CASE_LITERS_PER_UNIT,
    FM_REGRESSION_TEST_CASE_PULSES_PER_ACTIVE_UNIT,
    FM_REGRESSION_TEST_CASE_ERROR_PATHS,
    FM_REGRESSION_TEST_CASE_RATE_WINDOWS,
    FM_REGRESSION_TEST_CASE_RATE_ERROR_PATHS,
    FM_REGRESSION_TEST_CASE_RUNTIME_EVENTS,
    FM_REGRESSION_TEST_CASE_RUNTIME_INPUT_EVENTS,
    FM_REGRESSION_TEST_CASE_RUNTIME_ERROR_PATHS,
    FM_REGRESSION_TEST_CASE_SERVICE_STATE,
    FM_REGRESSION_TEST_CASE_SERVICE_ERROR_PATHS,
    FM_REGRESSION_TEST_CASE_VOLUME_VALUES,
    FM_REGRESSION_TEST_CASE_VOLUME_ERROR_PATHS,
    FM_REGRESSION_TEST_CASE_DISPLAY_FORMAT_VALUES,
    FM_REGRESSION_TEST_CASE_DISPLAY_FORMAT_ERROR_PATHS,
    FM_REGRESSION_TEST_CASE_KEYBOARD_MAPPING,
    FM_REGRESSION_TEST_CASE_COUNT
} fm_regression_test_case_t;

typedef struct
{
    fmc_model_volume_unit_t unit;
    double expected_liters_per_unit;
} fm_regression_liters_case_t;

typedef struct
{
    uint16_t gpio_pin;
    fm_board_keyboard_key_t expected_key;
} fm_regression_keyboard_case_t;

/* Private function declarations */
static bool fm_regression_test_double_eq_(double p_actual,
                                               double p_expected);
static bool fm_regression_test_init_defaults_(void);
static bool fm_regression_test_totals_(void);
static bool fm_regression_test_unit_validity_(void);
static bool fm_regression_test_unit_kind_(void);
static bool fm_regression_test_liters_per_unit_(void);
static bool fm_regression_test_pulses_per_active_unit_(void);
static bool fm_regression_test_error_paths_(void);
static bool fm_regression_test_rate_windows_(void);
static bool fm_regression_test_rate_error_paths_(void);
static bool fm_regression_test_runtime_events_(void);
static bool fm_regression_test_runtime_input_events_(void);
static bool fm_regression_test_runtime_error_paths_(void);
static bool fm_regression_test_service_state_(void);
static bool fm_regression_test_service_error_paths_(void);
static bool fm_regression_test_volume_values_(void);
static bool fm_regression_test_volume_error_paths_(void);
static bool fm_regression_test_display_format_values_(void);
static bool fm_regression_test_display_format_error_paths_(void);
static bool fm_regression_test_keyboard_mapping_(void);
static bool fm_regression_test_run_case_(fm_regression_test_case_t p_case);
static void fm_regression_test_emit_case_(fm_regression_test_case_t p_case,
                                               bool p_passed);
static bool fm_regression_test_text_eq_(const char *p_actual,
                                             const char *p_expected);

/* Public function definitions */
void FM_RegressionTest_Run(void)
{
    fm_regression_test_case_t test_case;
    bool passed = true;

    FM_BOARD_Init();
    FM_DEBUG_Init();

    (void) FM_DEBUG_UartStr("REGRESSION_TEST:START\n");

    for (test_case = FM_REGRESSION_TEST_CASE_INIT_DEFAULTS;
         test_case < FM_REGRESSION_TEST_CASE_COUNT;
         test_case = (fm_regression_test_case_t) (test_case + 1U))
    {
        if (!fm_regression_test_run_case_(test_case))
        {
            passed = false;
            fm_regression_test_emit_case_(test_case, false);
            FM_DEBUG_Flush();
            FM_DEBUG_PanicMsg("REGRESSION_TEST:FAIL\n");
        }

        fm_regression_test_emit_case_(test_case, true);
        FM_DEBUG_Flush();
    }

    if (passed)
    {
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:PASS\n");
    }

    for (;;)
    {
        FM_DEBUG_LedRun(FM_DEBUG_LED_ON);
        FM_PORT_TIME_SleepMs(FM_REGRESSION_TEST_IDLE_MS);
        FM_DEBUG_LedRun(FM_DEBUG_LED_OFF);
        FM_PORT_TIME_SleepMs(FM_REGRESSION_TEST_IDLE_MS);
    }
}

/* Private function definitions */
static bool fm_regression_test_double_eq_(double p_actual,
                                               double p_expected)
{
    double diff = p_actual - p_expected;

    if (diff < 0.0)
    {
        diff = -diff;
    }

    return diff <= FM_REGRESSION_TEST_EPSILON;
}

static bool fm_regression_test_text_eq_(const char *p_actual,
                                             const char *p_expected)
{
    while ((*p_actual != '\0') && (*p_expected != '\0'))
    {
        if (*p_actual != *p_expected)
        {
            return false;
        }

        p_actual++;
        p_expected++;
    }

    return (*p_actual == '\0') && (*p_expected == '\0');
}

/*
 * Verifies the startup model defaults used by the pure FMC slices.
 *
 * This catches accidental changes to factory-like assumptions before flash or
 * configuration persistence become the runtime source of these values.
 */
static bool fm_regression_test_init_defaults_(void)
{
    fmc_model_t model;

    FMC_MODEL_Init(&model);

    return fm_regression_test_double_eq_(
               model.measurement.calibration_pulses_per_unit,
               FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_DEFAULT) &&
           (model.measurement.calibration_volume_unit ==
            FMC_MODEL_CALIBRATION_UNIT_DEFAULT) &&
           (model.measurement.active_volume_unit == FMC_MODEL_VOLUME_UNIT_L) &&
           (model.measurement.active_time_base == FMC_MODEL_TIME_BASE_SECOND) &&
           (model.acm.pulses == 0U) &&
           (model.ttl.pulses == 0U);
}

/*
 * Verifies the canonical ACM/TTL pulse counters and reset policy.
 *
 * ACM must be user-resettable because it represents the operator resettable
 * accumulated volume. TTL must exist as a separate privileged counter so later
 * UI/service code cannot accidentally treat it like ACM.
 */
static bool fm_regression_test_totals_(void)
{
    fmc_model_t model;
    fmc_model_total_state_t *acm;
    fmc_model_total_state_t *ttl;

    FMC_MODEL_Init(&model);

    acm = FMC_MODEL_GetTotalState(&model, FMC_MODEL_TOTAL_ACM);
    ttl = FMC_MODEL_GetTotalState(&model, FMC_MODEL_TOTAL_TTL);

    if ((acm == NULL) || (ttl == NULL))
    {
        return false;
    }

    acm->pulses = 123U;
    ttl->pulses = 456U;

    if (FMC_MODEL_GetResetPolicy(FMC_MODEL_TOTAL_ACM) !=
        FMC_MODEL_RESET_USER_ALLOWED)
    {
        return false;
    }

    if (FMC_MODEL_GetResetPolicy(FMC_MODEL_TOTAL_TTL) !=
        FMC_MODEL_RESET_PRIVILEGED)
    {
        return false;
    }

    if (FMC_MODEL_ResetTotal(&model, FMC_MODEL_TOTAL_ACM) != FM_STATUS_OK)
    {
        return false;
    }

    if ((model.acm.pulses != 0U) || (model.ttl.pulses != 456U))
    {
        return false;
    }

    if (FMC_MODEL_ResetTotal(&model, FMC_MODEL_TOTAL_TTL) != FM_STATUS_OK)
    {
        return false;
    }

    return (model.ttl.pulses == 0U) &&
           (FMC_MODEL_GetTotalState(&model,
                                    (fmc_model_total_t) 99) == NULL) &&
           (FMC_MODEL_GetTotalStateConst(&model,
                                         (fmc_model_total_t) 99) == NULL) &&
           (FMC_MODEL_ResetTotal(&model,
                                 (fmc_model_total_t) 99) == FM_STATUS_EINVAL);
}

/*
 * Verifies the product's accepted volume-unit enum values and the fallback for
 * invalid active units.
 *
 * This protects the current policy that invalid persisted/display selections
 * normalize to liters instead of breaking pure calculations.
 */
static bool fm_regression_test_unit_validity_(void)
{
    return FMC_UNITS_IsVolumeUnitValid(FMC_MODEL_VOLUME_UNIT_CUSTOM) &&
           FMC_UNITS_IsVolumeUnitValid(FMC_MODEL_VOLUME_UNIT_L) &&
           FMC_UNITS_IsVolumeUnitValid(FMC_MODEL_VOLUME_UNIT_M3) &&
           FMC_UNITS_IsVolumeUnitValid(FMC_MODEL_VOLUME_UNIT_GAL_US) &&
           FMC_UNITS_IsVolumeUnitValid(FMC_MODEL_VOLUME_UNIT_BBL_US) &&
           FMC_UNITS_IsVolumeUnitValid(FMC_MODEL_VOLUME_UNIT_KG) &&
           FMC_UNITS_IsVolumeUnitValid(FMC_MODEL_VOLUME_UNIT_EQUIV_M3) &&
           !FMC_UNITS_IsVolumeUnitValid((fmc_model_volume_unit_t) 99) &&
           (FMC_UNITS_NormalizeVolumeUnit((fmc_model_volume_unit_t) 99) ==
            FMC_MODEL_VOLUME_UNIT_L);
}

/*
 * Verifies which units are physical liquid-volume units and which ones are
 * one-to-one product display units.
 *
 * The distinction matters because physical units use liter conversion, while
 * CUSTOM/KG/EQUIV_M3 currently preserve the calibrated unit scale.
 */
static bool fm_regression_test_unit_kind_(void)
{
    fmc_units_volume_unit_kind_t kind;

    if ((FMC_UNITS_GetVolumeUnitKind(FMC_MODEL_VOLUME_UNIT_L, &kind) !=
         FM_STATUS_OK) ||
        (kind != FMC_UNITS_VOLUME_UNIT_KIND_PHYSICAL))
    {
        return false;
    }

    if ((FMC_UNITS_GetVolumeUnitKind(FMC_MODEL_VOLUME_UNIT_M3, &kind) !=
         FM_STATUS_OK) ||
        (kind != FMC_UNITS_VOLUME_UNIT_KIND_PHYSICAL))
    {
        return false;
    }

    if ((FMC_UNITS_GetVolumeUnitKind(FMC_MODEL_VOLUME_UNIT_GAL_US, &kind) !=
         FM_STATUS_OK) ||
        (kind != FMC_UNITS_VOLUME_UNIT_KIND_PHYSICAL))
    {
        return false;
    }

    if ((FMC_UNITS_GetVolumeUnitKind(FMC_MODEL_VOLUME_UNIT_BBL_US, &kind) !=
         FM_STATUS_OK) ||
        (kind != FMC_UNITS_VOLUME_UNIT_KIND_PHYSICAL))
    {
        return false;
    }

    if ((FMC_UNITS_GetVolumeUnitKind(FMC_MODEL_VOLUME_UNIT_CUSTOM, &kind) !=
         FM_STATUS_OK) ||
        (kind != FMC_UNITS_VOLUME_UNIT_KIND_ONE_TO_ONE))
    {
        return false;
    }

    if ((FMC_UNITS_GetVolumeUnitKind(FMC_MODEL_VOLUME_UNIT_KG, &kind) !=
         FM_STATUS_OK) ||
        (kind != FMC_UNITS_VOLUME_UNIT_KIND_ONE_TO_ONE))
    {
        return false;
    }

    if ((FMC_UNITS_GetVolumeUnitKind(FMC_MODEL_VOLUME_UNIT_EQUIV_M3, &kind) !=
         FM_STATUS_OK) ||
        (kind != FMC_UNITS_VOLUME_UNIT_KIND_ONE_TO_ONE))
    {
        return false;
    }

    return (FMC_UNITS_GetVolumeUnitKind((fmc_model_volume_unit_t) 99, &kind) ==
            FM_STATUS_EINVAL) &&
           (FMC_UNITS_GetVolumeUnitKind(FMC_MODEL_VOLUME_UNIT_L, NULL) ==
            FM_STATUS_EINVAL);
}

/*
 * Verifies the fixed liter conversion table for physical volume units.
 *
 * Rate and volume calculations depend on this table when converting from the
 * calibration unit to the active display unit.
 */
static bool fm_regression_test_liters_per_unit_(void)
{
    static const fm_regression_liters_case_t cases[] =
    {
        { FMC_MODEL_VOLUME_UNIT_CUSTOM, 1.0 },
        { FMC_MODEL_VOLUME_UNIT_L, 1.0 },
        { FMC_MODEL_VOLUME_UNIT_M3, 1000.0 },
        { FMC_MODEL_VOLUME_UNIT_GAL_US, 3.78541 },
        { FMC_MODEL_VOLUME_UNIT_BBL_US, 158.987304 },
        { FMC_MODEL_VOLUME_UNIT_KG, 1.0 },
        { FMC_MODEL_VOLUME_UNIT_EQUIV_M3, 1.0 }
    };
    uint8_t index;
    double liters_per_unit;

    for (index = 0U;
         index < (uint8_t) (sizeof(cases) / sizeof(cases[0]));
         index++)
    {
        if (FMC_UNITS_GetLitersPerUnit(cases[index].unit,
                                       &liters_per_unit) != FM_STATUS_OK)
        {
            return false;
        }

        if (!fm_regression_test_double_eq_(
                liters_per_unit,
                cases[index].expected_liters_per_unit))
        {
            return false;
        }
    }

    return (FMC_UNITS_GetLitersPerUnit((fmc_model_volume_unit_t) 99,
                                       &liters_per_unit) == FM_STATUS_EINVAL) &&
           (FMC_UNITS_GetLitersPerUnit(FMC_MODEL_VOLUME_UNIT_L, NULL) ==
            FM_STATUS_EINVAL);
}

/*
 * Verifies the derived K factor expressed as pulses per active unit.
 *
 * This is the shared conversion used by rate and visible-volume calculations:
 * once this value is correct, higher-level modules can divide pulses by it
 * without duplicating unit policy.
 */
static bool fm_regression_test_pulses_per_active_unit_(void)
{
    static const fm_regression_liters_case_t cases[] =
    {
        { FMC_MODEL_VOLUME_UNIT_L, 2.5 },
        { FMC_MODEL_VOLUME_UNIT_M3, 2500.0 },
        { FMC_MODEL_VOLUME_UNIT_GAL_US, 9.463525 },
        { FMC_MODEL_VOLUME_UNIT_BBL_US, 397.46826 },
        { FMC_MODEL_VOLUME_UNIT_CUSTOM, 2.5 },
        { FMC_MODEL_VOLUME_UNIT_KG, 2.5 },
        { FMC_MODEL_VOLUME_UNIT_EQUIV_M3, 2.5 }
    };
    fmc_model_measurement_t measurement;
    uint8_t index;
    double pulses_per_active_unit;

    measurement.calibration_pulses_per_unit = 2.5;
    measurement.calibration_volume_unit = FMC_MODEL_VOLUME_UNIT_L;
    measurement.active_time_base = FMC_MODEL_TIME_BASE_SECOND;

    for (index = 0U;
         index < (uint8_t) (sizeof(cases) / sizeof(cases[0]));
         index++)
    {
        measurement.active_volume_unit = cases[index].unit;

        if (FMC_UNITS_CalcPulsesPerActiveUnit(&measurement,
                                              &pulses_per_active_unit) !=
            FM_STATUS_OK)
        {
            return false;
        }

        if (!fm_regression_test_double_eq_(
                pulses_per_active_unit,
                cases[index].expected_liters_per_unit))
        {
            return false;
        }
    }

    measurement.active_volume_unit = (fmc_model_volume_unit_t) 99;

    return (FMC_UNITS_CalcPulsesPerActiveUnit(&measurement,
                                              &pulses_per_active_unit) ==
            FM_STATUS_OK) &&
           fm_regression_test_double_eq_(pulses_per_active_unit, 2.5);
}

/*
 * Verifies expected unit-conversion failure paths.
 *
 * These checks keep invalid pointers, out-of-range calibration, and currently
 * unsupported calibration units explicit instead of silently producing a value.
 */
static bool fm_regression_test_error_paths_(void)
{
    fmc_model_measurement_t measurement;
    double pulses_per_active_unit;

    measurement.calibration_pulses_per_unit =
        FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_MIN - 0.1;
    measurement.calibration_volume_unit = FMC_MODEL_VOLUME_UNIT_L;
    measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_L;
    measurement.active_time_base = FMC_MODEL_TIME_BASE_SECOND;

    if (FMC_UNITS_CalcPulsesPerActiveUnit(&measurement,
                                          &pulses_per_active_unit) !=
        FM_STATUS_ERANGE)
    {
        return false;
    }

    measurement.calibration_pulses_per_unit =
        FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_MAX + 0.1;

    if (FMC_UNITS_CalcPulsesPerActiveUnit(&measurement,
                                          &pulses_per_active_unit) !=
        FM_STATUS_ERANGE)
    {
        return false;
    }

    measurement.calibration_pulses_per_unit =
        FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_DEFAULT;
    measurement.calibration_volume_unit = FMC_MODEL_VOLUME_UNIT_M3;

    return (FMC_UNITS_CalcPulsesPerActiveUnit(&measurement,
                                              &pulses_per_active_unit) ==
            FM_STATUS_ENOTSUP) &&
           (FMC_UNITS_CalcPulsesPerActiveUnit(NULL,
                                              &pulses_per_active_unit) ==
            FM_STATUS_EINVAL) &&
           (FMC_UNITS_CalcPulsesPerActiveUnit(&measurement, NULL) ==
            FM_STATUS_EINVAL);
}

/*
 * Verifies instantaneous rate for each supported time base and representative
 * active-unit conversions.
 *
 * This keeps rate independent from acquisition/RTOS code: the test feeds a
 * pulse/time window and expects a pure calculated rate.
 */
static bool fm_regression_test_rate_windows_(void)
{
    static const fmc_model_time_base_t time_bases[] =
    {
        FMC_MODEL_TIME_BASE_SECOND,
        FMC_MODEL_TIME_BASE_MINUTE,
        FMC_MODEL_TIME_BASE_HOUR,
        FMC_MODEL_TIME_BASE_DAY
    };
    static const double expected_rates[] =
    {
        1.0,
        60.0,
        3600.0,
        86400.0
    };
    fmc_model_measurement_t measurement;
    uint8_t index;
    double rate;

    measurement.calibration_pulses_per_unit = 2.0;
    measurement.calibration_volume_unit = FMC_MODEL_VOLUME_UNIT_L;
    measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_L;

    for (index = 0U;
         index < (uint8_t) (sizeof(time_bases) / sizeof(time_bases[0]));
         index++)
    {
        measurement.active_time_base = time_bases[index];

        if ((FMC_RATE_Calc(&measurement, 4U, 2.0, &rate) != FM_STATUS_OK) ||
            !fm_regression_test_double_eq_(rate,
                                                expected_rates[index]))
        {
            return false;
        }
    }

    measurement.active_time_base = FMC_MODEL_TIME_BASE_MINUTE;
    measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_M3;

    if ((FMC_RATE_Calc(&measurement, 2000U, 10.0, &rate) != FM_STATUS_OK) ||
        !fm_regression_test_double_eq_(rate, 6.0))
    {
        return false;
    }

    measurement.calibration_pulses_per_unit = 6000.0;
    measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_L;

    if ((FMC_RATE_Calc(&measurement, 60U, 1.0, &rate) != FM_STATUS_OK) ||
        !fm_regression_test_double_eq_(rate, 0.6))
    {
        return false;
    }

    return (FMC_RATE_Calc(&measurement, 0U, 10.0, &rate) == FM_STATUS_OK) &&
           fm_regression_test_double_eq_(rate, 0.0);
}

/*
 * Verifies expected rate failure paths.
 *
 * This protects the contract for invalid arguments, non-positive time windows,
 * invalid time bases, and invalid measurement configuration.
 */
static bool fm_regression_test_rate_error_paths_(void)
{
    fmc_model_measurement_t measurement;
    double rate;

    measurement.calibration_pulses_per_unit =
        FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_DEFAULT;
    measurement.calibration_volume_unit = FMC_MODEL_VOLUME_UNIT_L;
    measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_L;
    measurement.active_time_base = FMC_MODEL_TIME_BASE_SECOND;

    if ((FMC_RATE_Calc(NULL, 1U, 1.0, &rate) != FM_STATUS_EINVAL) ||
        (FMC_RATE_Calc(&measurement, 1U, 1.0, NULL) != FM_STATUS_EINVAL) ||
        (FMC_RATE_Calc(&measurement, 1U, 0.0, &rate) != FM_STATUS_ERANGE) ||
        (FMC_RATE_Calc(&measurement, 1U, -1.0, &rate) != FM_STATUS_ERANGE))
    {
        return false;
    }

    measurement.active_time_base = (fmc_model_time_base_t) 99;
    if (FMC_RATE_Calc(&measurement, 1U, 1.0, &rate) != FM_STATUS_EINVAL)
    {
        return false;
    }

    measurement.active_time_base = FMC_MODEL_TIME_BASE_SECOND;
    measurement.calibration_pulses_per_unit =
        FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_MIN - 0.1;

    return FMC_RATE_Calc(&measurement, 1U, 1.0, &rate) == FM_STATUS_ERANGE;
}

/*
 * Verifies the minimal runtime event boundary over the live FMC service.
 *
 * Product events update service state and mark presentation dirty without
 * naming a keyboard, LCD driver, queue, or RTOS primitive.
 */
static bool fm_regression_test_runtime_events_(void)
{
    fmc_runtime_t runtime;
    fmc_service_snapshot_t snapshot;
    fmc_runtime_event_t event;

    FMC_RUNTIME_Init(&runtime);

    runtime.service.model.measurement.calibration_pulses_per_unit = 1000.0;
    runtime.service.model.measurement.calibration_volume_unit =
        FMC_MODEL_VOLUME_UNIT_L;
    runtime.service.model.measurement.active_volume_unit =
        FMC_MODEL_VOLUME_UNIT_L;

    event.kind = FMC_RUNTIME_EVENT_NONE;
    event.data.pulse_delta = 0U;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_OK) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime))
    {
        return false;
    }

    event.kind = FMC_RUNTIME_EVENT_PULSE_DELTA;
    event.data.pulse_delta = 2500U;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_OK) ||
        !FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        (FMC_RUNTIME_GetSnapshot(&runtime, &snapshot) != FM_STATUS_OK))
    {
        return false;
    }

    if ((snapshot.model.acm.pulses != 2500U) ||
        (snapshot.model.ttl.pulses != 2500U) ||
        !fm_regression_test_double_eq_(snapshot.acm_volume, 2.5) ||
        !fm_regression_test_double_eq_(snapshot.ttl_volume, 2.5))
    {
        return false;
    }

    if ((FMC_RUNTIME_ClearPresentationUpdatePending(&runtime) !=
         FM_STATUS_OK) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime))
    {
        return false;
    }

    event.kind = FMC_RUNTIME_EVENT_PRESENTATION_INVALIDATE;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_OK) ||
        !FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        (FMC_RUNTIME_ClearPresentationUpdatePending(&runtime) !=
         FM_STATUS_OK))
    {
        return false;
    }

    event.kind = FMC_RUNTIME_EVENT_RESET_TTL;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_OK) ||
        !FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        (FMC_RUNTIME_GetSnapshot(&runtime, &snapshot) != FM_STATUS_OK))
    {
        return false;
    }

    if ((snapshot.model.acm.pulses != 2500U) ||
        (snapshot.model.ttl.pulses != 0U) ||
        !fm_regression_test_double_eq_(snapshot.acm_volume, 2.5) ||
        !fm_regression_test_double_eq_(snapshot.ttl_volume, 0.0))
    {
        return false;
    }

    if (FMC_RUNTIME_ClearPresentationUpdatePending(&runtime) != FM_STATUS_OK)
    {
        return false;
    }

    event.kind = FMC_RUNTIME_EVENT_RESET_ACM;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_OK) ||
        !FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        (FMC_RUNTIME_GetSnapshot(&runtime, &snapshot) != FM_STATUS_OK))
    {
        return false;
    }

    return (snapshot.model.acm.pulses == 0U) &&
           (snapshot.model.ttl.pulses == 0U) &&
           fm_regression_test_double_eq_(snapshot.acm_volume, 0.0) &&
           fm_regression_test_double_eq_(snapshot.ttl_volume, 0.0);
}

/*
 * Verifies that semantic input identity reaches runtime without BSP/HAL types
 * and without changing service totals.
 */
static bool fm_regression_test_runtime_input_events_(void)
{
    static const fmc_input_key_t mechanical_keys[] =
    {
        FMC_INPUT_KEY_DOWN,
        FMC_INPUT_KEY_UP,
        FMC_INPUT_KEY_ENTER,
        FMC_INPUT_KEY_ESC
    };
    static const fmc_input_key_t external_keys[] =
    {
        FMC_INPUT_KEY_EXT_1,
        FMC_INPUT_KEY_EXT_2
    };
    static const fmc_input_action_t actions[] =
    {
        FMC_INPUT_ACTION_SHORT,
        FMC_INPUT_ACTION_LONG
    };
    fmc_runtime_t runtime;
    fmc_runtime_event_t event;
    uint8_t key_index;
    uint8_t action_index;

    FMC_RUNTIME_Init(&runtime);
    runtime.service.model.acm.pulses = 11U;
    runtime.service.model.ttl.pulses = 22U;

    event.kind = FMC_RUNTIME_EVENT_INPUT;

    for (key_index = 0U;
         key_index < (uint8_t) (sizeof(mechanical_keys) /
                                sizeof(mechanical_keys[0]));
         key_index++)
    {
        for (action_index = 0U;
             action_index < (uint8_t) (sizeof(actions) / sizeof(actions[0]));
             action_index++)
        {
            if (FMC_RUNTIME_ClearPresentationUpdatePending(&runtime) !=
                FM_STATUS_OK)
            {
                return false;
            }

            event.data.input.key = mechanical_keys[key_index];
            event.data.input.action = actions[action_index];

            if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_OK) ||
                !FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
                !runtime.last_input_valid ||
                (runtime.last_input.key != mechanical_keys[key_index]) ||
                (runtime.last_input.action != actions[action_index]) ||
                (runtime.service.model.acm.pulses != 11U) ||
                (runtime.service.model.ttl.pulses != 22U))
            {
                return false;
            }
        }
    }

    for (key_index = 0U;
         key_index < (uint8_t) (sizeof(external_keys) /
                                sizeof(external_keys[0]));
         key_index++)
    {
        if (FMC_RUNTIME_ClearPresentationUpdatePending(&runtime) !=
            FM_STATUS_OK)
        {
            return false;
        }

        event.data.input.key = external_keys[key_index];
        event.data.input.action = FMC_INPUT_ACTION_SHORT;

        if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_OK) ||
            !FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
            !runtime.last_input_valid ||
            (runtime.last_input.key != external_keys[key_index]) ||
            (runtime.last_input.action != FMC_INPUT_ACTION_SHORT) ||
            (runtime.service.model.acm.pulses != 11U) ||
            (runtime.service.model.ttl.pulses != 22U))
        {
            return false;
        }
    }

    return true;
}

/*
 * Verifies that invalid runtime calls and service errors stay explicit and do
 * not create new presentation work.
 */
static bool fm_regression_test_runtime_error_paths_(void)
{
    fmc_runtime_t runtime;
    fmc_service_snapshot_t snapshot;
    fmc_runtime_event_t event;

    FMC_RUNTIME_Init(&runtime);

    event.kind = FMC_RUNTIME_EVENT_PULSE_DELTA;
    event.data.pulse_delta = 1U;
    if ((FMC_RUNTIME_Dispatch(NULL, &event) != FM_STATUS_EINVAL) ||
        (FMC_RUNTIME_Dispatch(&runtime, NULL) != FM_STATUS_EINVAL) ||
        (FMC_RUNTIME_GetSnapshot(NULL, &snapshot) != FM_STATUS_EINVAL) ||
        (FMC_RUNTIME_GetSnapshot(&runtime, NULL) != FM_STATUS_EINVAL) ||
        FMC_RUNTIME_PresentationUpdateIsPending(NULL) ||
        (FMC_RUNTIME_ClearPresentationUpdatePending(NULL) !=
         FM_STATUS_EINVAL))
    {
        return false;
    }

    event.kind = (fmc_runtime_event_kind_t) 99;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_EINVAL) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime))
    {
        return false;
    }

    runtime.service.model.acm.pulses = UINT64_MAX;
    event.kind = FMC_RUNTIME_EVENT_PULSE_DELTA;
    event.data.pulse_delta = 1U;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_ERANGE) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        (runtime.service.model.acm.pulses != UINT64_MAX) ||
        (runtime.service.model.ttl.pulses != 0U))
    {
        return false;
    }

    event.kind = FMC_RUNTIME_EVENT_INPUT;
    event.data.input.key = FMC_INPUT_KEY_COUNT;
    event.data.input.action = FMC_INPUT_ACTION_SHORT;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_EINVAL) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        runtime.last_input_valid)
    {
        return false;
    }

    event.data.input.key = FMC_INPUT_KEY_DOWN;
    event.data.input.action = FMC_INPUT_ACTION_COUNT;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_EINVAL) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        runtime.last_input_valid)
    {
        return false;
    }

    event.data.input.key = (fmc_input_key_t) -1;
    event.data.input.action = FMC_INPUT_ACTION_SHORT;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_EINVAL) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        runtime.last_input_valid)
    {
        return false;
    }

    event.data.input.key = FMC_INPUT_KEY_DOWN;
    event.data.input.action = (fmc_input_action_t) -1;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_EINVAL) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        runtime.last_input_valid)
    {
        return false;
    }

    event.data.input.key = FMC_INPUT_KEY_EXT_1;
    event.data.input.action = FMC_INPUT_ACTION_LONG;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_EINVAL) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        runtime.last_input_valid)
    {
        return false;
    }

    event.data.input.key = FMC_INPUT_KEY_EXT_2;
    event.data.input.action = FMC_INPUT_ACTION_LONG;
    return (FMC_RUNTIME_Dispatch(&runtime, &event) == FM_STATUS_EINVAL) &&
           !FMC_RUNTIME_PresentationUpdateIsPending(&runtime) &&
           !runtime.last_input_valid;
}

/*
 * Verifies the live-state service boundary used by future runtime adapters.
 *
 * Acquisition deltas accumulate into both product totals. Presentation should
 * use snapshots rather than reaching into the service-owned model.
 */
static bool fm_regression_test_service_state_(void)
{
    fmc_service_t service;
    fmc_service_snapshot_t snapshot;

    FMC_SERVICE_Init(&service);

    service.model.measurement.calibration_pulses_per_unit = 1000.0;
    service.model.measurement.calibration_volume_unit = FMC_MODEL_VOLUME_UNIT_L;
    service.model.measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_L;

    if ((FMC_SERVICE_AddPulseDelta(&service, 2500U) != FM_STATUS_OK) ||
        (FMC_SERVICE_GetSnapshot(&service, &snapshot) != FM_STATUS_OK))
    {
        return false;
    }

    if ((snapshot.model.acm.pulses != 2500U) ||
        (snapshot.model.ttl.pulses != 2500U) ||
        !fm_regression_test_double_eq_(snapshot.acm_volume, 2.5) ||
        !fm_regression_test_double_eq_(snapshot.ttl_volume, 2.5))
    {
        return false;
    }

    if ((FMC_SERVICE_ResetTotal(&service, FMC_MODEL_TOTAL_ACM) !=
         FM_STATUS_OK) ||
        (FMC_SERVICE_GetSnapshot(&service, &snapshot) != FM_STATUS_OK))
    {
        return false;
    }

    return (snapshot.model.acm.pulses == 0U) &&
           (snapshot.model.ttl.pulses == 2500U) &&
           fm_regression_test_double_eq_(snapshot.acm_volume, 0.0) &&
           fm_regression_test_double_eq_(snapshot.ttl_volume, 2.5);
}

/*
 * Verifies that the service keeps invalid arguments, invalid total roles, and
 * pulse-counter overflow explicit.
 */
static bool fm_regression_test_service_error_paths_(void)
{
    fmc_service_t service;
    fmc_service_snapshot_t snapshot;

    FMC_SERVICE_Init(&service);

    if ((FMC_SERVICE_AddPulseDelta(NULL, 1U) != FM_STATUS_EINVAL) ||
        (FMC_SERVICE_ResetTotal(NULL, FMC_MODEL_TOTAL_ACM) !=
         FM_STATUS_EINVAL) ||
        (FMC_SERVICE_GetSnapshot(NULL, &snapshot) != FM_STATUS_EINVAL) ||
        (FMC_SERVICE_GetSnapshot(&service, NULL) != FM_STATUS_EINVAL) ||
        (FMC_SERVICE_ResetTotal(&service,
                                (fmc_model_total_t) 99) != FM_STATUS_EINVAL))
    {
        return false;
    }

    service.model.acm.pulses = UINT64_MAX;
    service.model.ttl.pulses = 0U;

    if (FMC_SERVICE_AddPulseDelta(&service, 1U) != FM_STATUS_ERANGE)
    {
        return false;
    }

    service.model.acm.pulses = 0U;
    service.model.ttl.pulses = UINT64_MAX;

    if (FMC_SERVICE_AddPulseDelta(&service, 1U) != FM_STATUS_ERANGE)
    {
        return false;
    }

    service.model.ttl.pulses = 0U;
    service.model.measurement.calibration_pulses_per_unit =
        FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_MIN - 0.1;

    return FMC_SERVICE_GetSnapshot(&service, &snapshot) == FM_STATUS_ERANGE;
}

/*
 * Verifies visible volume calculation from pulse counters.
 *
 * The test covers direct total-state conversion plus role-based ACM/TTL
 * selection. It intentionally stops before decimal selection or LCD formatting.
 */
static bool fm_regression_test_volume_values_(void)
{
    fmc_model_t model;
    fmc_model_total_state_t total;
    double volume;

    FMC_MODEL_Init(&model);

    total.pulses = 2500U;
    model.measurement.calibration_pulses_per_unit = 1000.0;
    model.measurement.calibration_volume_unit = FMC_MODEL_VOLUME_UNIT_L;
    model.measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_L;

    if ((FMC_VOLUME_CalcFromTotal(&model.measurement, &total, &volume) !=
         FM_STATUS_OK) ||
        !fm_regression_test_double_eq_(volume, 2.5))
    {
        return false;
    }

    model.measurement.active_volume_unit = (fmc_model_volume_unit_t) 99;

    if ((FMC_VOLUME_CalcFromTotal(&model.measurement, &total, &volume) !=
         FM_STATUS_OK) ||
        !fm_regression_test_double_eq_(volume, 2.5))
    {
        return false;
    }

    total.pulses = 1000000U;
    model.measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_M3;

    if ((FMC_VOLUME_CalcFromTotal(&model.measurement, &total, &volume) !=
         FM_STATUS_OK) ||
        !fm_regression_test_double_eq_(volume, 1.0))
    {
        return false;
    }

    total.pulses = 6000U;
    model.measurement.calibration_pulses_per_unit = 6000.0;
    model.measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_KG;

    if ((FMC_VOLUME_CalcFromTotal(&model.measurement, &total, &volume) !=
         FM_STATUS_OK) ||
        !fm_regression_test_double_eq_(volume, 1.0))
    {
        return false;
    }

    model.measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_EQUIV_M3;

    if ((FMC_VOLUME_CalcFromTotal(&model.measurement, &total, &volume) !=
         FM_STATUS_OK) ||
        !fm_regression_test_double_eq_(volume, 1.0))
    {
        return false;
    }

    model.measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_CUSTOM;

    if ((FMC_VOLUME_CalcFromTotal(&model.measurement, &total, &volume) !=
         FM_STATUS_OK) ||
        !fm_regression_test_double_eq_(volume, 1.0))
    {
        return false;
    }

    model.measurement.calibration_pulses_per_unit = 1000.0;
    model.measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_L;
    model.acm.pulses = 1234U;
    model.ttl.pulses = 5678U;

    if ((FMC_VOLUME_CalcByTotalRole(&model,
                                    FMC_MODEL_TOTAL_ACM,
                                    &volume) != FM_STATUS_OK) ||
        !fm_regression_test_double_eq_(volume, 1.234))
    {
        return false;
    }

    return (FMC_VOLUME_CalcByTotalRole(&model,
                                       FMC_MODEL_TOTAL_TTL,
                                       &volume) == FM_STATUS_OK) &&
           fm_regression_test_double_eq_(volume, 5.678);
}

/*
 * Verifies expected volume-calculation failure paths.
 *
 * The volume module delegates measurement validation to `fmc_units.*`, so these
 * checks make sure that errors propagate without hiding bad configuration.
 */
static bool fm_regression_test_volume_error_paths_(void)
{
    fmc_model_t model;
    fmc_model_total_state_t total;
    double volume;

    FMC_MODEL_Init(&model);
    total.pulses = 1U;

    if ((FMC_VOLUME_CalcFromTotal(NULL, &total, &volume) !=
         FM_STATUS_EINVAL) ||
        (FMC_VOLUME_CalcFromTotal(&model.measurement, NULL, &volume) !=
         FM_STATUS_EINVAL) ||
        (FMC_VOLUME_CalcFromTotal(&model.measurement, &total, NULL) !=
         FM_STATUS_EINVAL) ||
        (FMC_VOLUME_CalcByTotalRole(NULL,
                                    FMC_MODEL_TOTAL_ACM,
                                    &volume) != FM_STATUS_EINVAL) ||
        (FMC_VOLUME_CalcByTotalRole(&model,
                                    FMC_MODEL_TOTAL_ACM,
                                    NULL) != FM_STATUS_EINVAL) ||
        (FMC_VOLUME_CalcByTotalRole(&model,
                                    (fmc_model_total_t) 99,
                                    &volume) != FM_STATUS_EINVAL))
    {
        return false;
    }

    model.measurement.calibration_pulses_per_unit =
        FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_MIN - 0.1;

    if (FMC_VOLUME_CalcFromTotal(&model.measurement, &total, &volume) !=
        FM_STATUS_ERANGE)
    {
        return false;
    }

    model.measurement.calibration_pulses_per_unit =
        FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_DEFAULT;
    model.measurement.calibration_volume_unit = FMC_MODEL_VOLUME_UNIT_M3;

    return FMC_VOLUME_CalcFromTotal(&model.measurement, &total, &volume) ==
           FM_STATUS_ENOTSUP;
}

/*
 * Verifies reusable bounded display-field formatting.
 *
 * These checks keep numeric-to-text conversion above the LCD BSP but below
 * product presentation. The output strings are ready for `FM_LCD_WriteText()`.
 */
static bool fm_regression_test_display_format_values_(void)
{
    display_format_field_t field;
    char text[16];

    field.visible_width = 5U;
    field.fractional_digits = 0U;
    field.align = DISPLAY_FORMAT_ALIGN_RIGHT;
    field.pad_char = ' ';
    field.overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR;
    field.overflow_char = '-';

    if ((DISPLAY_FORMAT_Unsigned(123U, &field, text, sizeof(text)) !=
         FM_STATUS_OK) ||
        !fm_regression_test_text_eq_(text, "  123"))
    {
        return false;
    }

    if ((DISPLAY_FORMAT_Signed(-12, &field, text, sizeof(text)) !=
         FM_STATUS_OK) ||
        !fm_regression_test_text_eq_(text, "  -12"))
    {
        return false;
    }

    field.pad_char = '0';

    if ((DISPLAY_FORMAT_Signed(-12, &field, text, sizeof(text)) !=
         FM_STATUS_OK) ||
        !fm_regression_test_text_eq_(text, "-0012"))
    {
        return false;
    }

    field.visible_width = 7U;
    field.fractional_digits = 1U;
    field.pad_char = ' ';

    if ((DISPLAY_FORMAT_Scaled(12345, 2U, &field, text, sizeof(text)) !=
         FM_STATUS_OK) ||
        !fm_regression_test_text_eq_(text, "   123.5"))
    {
        return false;
    }

    field.visible_width = 8U;
    field.pad_char = '0';

    if ((DISPLAY_FORMAT_Scaled(12345, 2U, &field, text, sizeof(text)) !=
         FM_STATUS_OK) ||
        !fm_regression_test_text_eq_(text, "0000123.5"))
    {
        return false;
    }

    field.align = DISPLAY_FORMAT_ALIGN_LEFT;
    field.visible_width = 7U;
    field.pad_char = ' ';

    if ((DISPLAY_FORMAT_Scaled(12345, 2U, &field, text, sizeof(text)) !=
         FM_STATUS_OK) ||
        !fm_regression_test_text_eq_(text, "123.5   "))
    {
        return false;
    }

    field.visible_width = 5U;
    field.fractional_digits = 2U;
    field.align = DISPLAY_FORMAT_ALIGN_RIGHT;

    return (DISPLAY_FORMAT_Double(1.236, &field, text, sizeof(text)) ==
            FM_STATUS_OK) &&
           fm_regression_test_text_eq_(text, "  1.24");
}

/*
 * Verifies expected display-format failure paths.
 *
 * Overflow is reported as `FM_STATUS_ERANGE`. When the caller requests a fill
 * policy, the field is still populated so the UI can show a bounded marker.
 */
static bool fm_regression_test_display_format_error_paths_(void)
{
    display_format_field_t field;
    char text[16];

    field.visible_width = 3U;
    field.fractional_digits = 0U;
    field.align = DISPLAY_FORMAT_ALIGN_RIGHT;
    field.pad_char = ' ';
    field.overflow_policy = DISPLAY_FORMAT_OVERFLOW_FILL;
    field.overflow_char = '-';

    if ((DISPLAY_FORMAT_Unsigned(1234U, &field, text, sizeof(text)) !=
         FM_STATUS_ERANGE) ||
        !fm_regression_test_text_eq_(text, "---"))
    {
        return false;
    }

    if ((DISPLAY_FORMAT_Unsigned(1U, NULL, text, sizeof(text)) !=
         FM_STATUS_EINVAL) ||
        (DISPLAY_FORMAT_Unsigned(1U, &field, NULL, sizeof(text)) !=
         FM_STATUS_EINVAL) ||
        (DISPLAY_FORMAT_Unsigned(1U, &field, text, 0U) != FM_STATUS_EINVAL))
    {
        return false;
    }

    field.visible_width = 0U;

    if (DISPLAY_FORMAT_Unsigned(1U, &field, text, sizeof(text)) !=
        FM_STATUS_EINVAL)
    {
        return false;
    }

    field.visible_width = 5U;
    field.fractional_digits = 1U;

    if (DISPLAY_FORMAT_Unsigned(1U, &field, text, sizeof(text)) !=
        FM_STATUS_EINVAL)
    {
        return false;
    }

    field.fractional_digits = DISPLAY_FORMAT_FRACTIONAL_DIGITS_MAX + 1U;

    return DISPLAY_FORMAT_Double(1.0, &field, text, sizeof(text)) ==
           FM_STATUS_EINVAL;
}

/*
 * Verifies the board keyboard GPIO symbols generated by CubeMX map to the
 * semantic keys consumed by higher layers and bring-ups.
 */
static bool fm_regression_test_keyboard_mapping_(void)
{
    static const fm_regression_keyboard_case_t cases[] =
    {
        { KEY_DOWN_Pin, FM_BOARD_KEYBOARD_KEY_DOWN },
        { KEY_UP_Pin, FM_BOARD_KEYBOARD_KEY_UP },
        { KEY_ENTER_Pin, FM_BOARD_KEYBOARD_KEY_ENTER },
        { KEY_ESC_Pin, FM_BOARD_KEYBOARD_KEY_ESC }
    };
    fm_board_keyboard_key_t key;
    uint8_t index;

    for (index = 0U;
         index < (uint8_t) (sizeof(cases) / sizeof(cases[0]));
         index++)
    {
        key = FM_BOARD_KEYBOARD_KEY_COUNT;
        if (!FM_BOARD_KeyboardKeyFromGpioPin(cases[index].gpio_pin, &key) ||
            (key != cases[index].expected_key))
        {
            return false;
        }
    }

    key = FM_BOARD_KEYBOARD_KEY_DOWN;

    return !FM_BOARD_KeyboardKeyFromGpioPin(0U, &key) &&
           (key == FM_BOARD_KEYBOARD_KEY_DOWN) &&
           !FM_BOARD_KeyboardKeyFromGpioPin(KEY_DOWN_Pin, NULL);
}

static bool fm_regression_test_run_case_(fm_regression_test_case_t p_case)
{
    switch (p_case)
    {
    case FM_REGRESSION_TEST_CASE_INIT_DEFAULTS:
        return fm_regression_test_init_defaults_();

    case FM_REGRESSION_TEST_CASE_TOTALS:
        return fm_regression_test_totals_();

    case FM_REGRESSION_TEST_CASE_UNIT_VALIDITY:
        return fm_regression_test_unit_validity_();

    case FM_REGRESSION_TEST_CASE_UNIT_KIND:
        return fm_regression_test_unit_kind_();

    case FM_REGRESSION_TEST_CASE_LITERS_PER_UNIT:
        return fm_regression_test_liters_per_unit_();

    case FM_REGRESSION_TEST_CASE_PULSES_PER_ACTIVE_UNIT:
        return fm_regression_test_pulses_per_active_unit_();

    case FM_REGRESSION_TEST_CASE_ERROR_PATHS:
        return fm_regression_test_error_paths_();

    case FM_REGRESSION_TEST_CASE_RATE_WINDOWS:
        return fm_regression_test_rate_windows_();

    case FM_REGRESSION_TEST_CASE_RATE_ERROR_PATHS:
        return fm_regression_test_rate_error_paths_();

    case FM_REGRESSION_TEST_CASE_RUNTIME_EVENTS:
        return fm_regression_test_runtime_events_();

    case FM_REGRESSION_TEST_CASE_RUNTIME_INPUT_EVENTS:
        return fm_regression_test_runtime_input_events_();

    case FM_REGRESSION_TEST_CASE_RUNTIME_ERROR_PATHS:
        return fm_regression_test_runtime_error_paths_();

    case FM_REGRESSION_TEST_CASE_SERVICE_STATE:
        return fm_regression_test_service_state_();

    case FM_REGRESSION_TEST_CASE_SERVICE_ERROR_PATHS:
        return fm_regression_test_service_error_paths_();

    case FM_REGRESSION_TEST_CASE_VOLUME_VALUES:
        return fm_regression_test_volume_values_();

    case FM_REGRESSION_TEST_CASE_VOLUME_ERROR_PATHS:
        return fm_regression_test_volume_error_paths_();

    case FM_REGRESSION_TEST_CASE_DISPLAY_FORMAT_VALUES:
        return fm_regression_test_display_format_values_();

    case FM_REGRESSION_TEST_CASE_DISPLAY_FORMAT_ERROR_PATHS:
        return fm_regression_test_display_format_error_paths_();

    case FM_REGRESSION_TEST_CASE_KEYBOARD_MAPPING:
        return fm_regression_test_keyboard_mapping_();

    default:
        return false;
    }
}

static void fm_regression_test_emit_case_(fm_regression_test_case_t p_case,
                                               bool p_passed)
{
    switch (p_case)
    {
    case FM_REGRESSION_TEST_CASE_INIT_DEFAULTS:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:INIT_DEFAULTS:");
        break;

    case FM_REGRESSION_TEST_CASE_TOTALS:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:TOTALS:");
        break;

    case FM_REGRESSION_TEST_CASE_UNIT_VALIDITY:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:UNIT_VALIDITY:");
        break;

    case FM_REGRESSION_TEST_CASE_UNIT_KIND:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:UNIT_KIND:");
        break;

    case FM_REGRESSION_TEST_CASE_LITERS_PER_UNIT:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:LITERS_PER_UNIT:");
        break;

    case FM_REGRESSION_TEST_CASE_PULSES_PER_ACTIVE_UNIT:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:PULSES_PER_ACTIVE_UNIT:");
        break;

    case FM_REGRESSION_TEST_CASE_ERROR_PATHS:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:ERROR_PATHS:");
        break;

    case FM_REGRESSION_TEST_CASE_RATE_WINDOWS:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:RATE_WINDOWS:");
        break;

    case FM_REGRESSION_TEST_CASE_RATE_ERROR_PATHS:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:RATE_ERROR_PATHS:");
        break;

    case FM_REGRESSION_TEST_CASE_RUNTIME_EVENTS:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:RUNTIME_EVENTS:");
        break;

    case FM_REGRESSION_TEST_CASE_RUNTIME_INPUT_EVENTS:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:RUNTIME_INPUT_EVENTS:");
        break;

    case FM_REGRESSION_TEST_CASE_RUNTIME_ERROR_PATHS:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:RUNTIME_ERROR_PATHS:");
        break;

    case FM_REGRESSION_TEST_CASE_SERVICE_STATE:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:SERVICE_STATE:");
        break;

    case FM_REGRESSION_TEST_CASE_SERVICE_ERROR_PATHS:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:SERVICE_ERROR_PATHS:");
        break;

    case FM_REGRESSION_TEST_CASE_VOLUME_VALUES:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:VOLUME_VALUES:");
        break;

    case FM_REGRESSION_TEST_CASE_VOLUME_ERROR_PATHS:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:VOLUME_ERROR_PATHS:");
        break;

    case FM_REGRESSION_TEST_CASE_DISPLAY_FORMAT_VALUES:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:DISPLAY_FORMAT_VALUES:");
        break;

    case FM_REGRESSION_TEST_CASE_DISPLAY_FORMAT_ERROR_PATHS:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:DISPLAY_FORMAT_ERROR_PATHS:");
        break;

    case FM_REGRESSION_TEST_CASE_KEYBOARD_MAPPING:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:KEYBOARD_MAPPING:");
        break;

    default:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:UNKNOWN:");
        break;
    }

    if (p_passed)
    {
        (void) FM_DEBUG_UartStr("PASS\n");
    }
    else
    {
        (void) FM_DEBUG_UartStr("FAIL\n");
    }
}
