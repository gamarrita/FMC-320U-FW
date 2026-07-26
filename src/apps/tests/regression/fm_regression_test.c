/**
 * @file    fm_regression_test.c
 * @brief   Repeatable verification app for pure model, acquisition, unit,
 *          rate, volume, and display-format slices.
 */
#include "fm_regression_test.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "display_format.h"
#include "fm_board.h"
#include "fm_board_keyboard.h"
#include "fm_debug.h"
#include "frequency_observation.h"
#include "fm_main_acquisition.h"
#include "fm_main_event.h"
#include "fm_main_input_adapter.h"
#include "fm_main_input_recognizer.h"
#include "devices/lcd/fm_lcd_map.h"
#include "fm_port_time.h"
#include "fmc_input.h"
#include "fmc_model.h"
#include "fmc_presentation.h"
#include "fmc_rate.h"
#include "fmc_runtime.h"
#include "fmc_service.h"
#include "fmc_units.h"
#include "fmc_volume.h"
#include "main.h"
#include "pulse_delta.h"

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
    FM_REGRESSION_TEST_CASE_PULSE_DELTA_VECTORS,
    FM_REGRESSION_TEST_CASE_FREQUENCY_OBSERVATION_VECTORS,
    FM_REGRESSION_TEST_CASE_OBSERVER_INDEPENDENCE,
    FM_REGRESSION_TEST_CASE_RUNTIME_FREQUENCY_RESULT,
    FM_REGRESSION_TEST_CASE_MAIN_ACQUISITION,
    FM_REGRESSION_TEST_CASE_RUNTIME_EVENTS,
    FM_REGRESSION_TEST_CASE_RUNTIME_INPUT_EVENTS,
    FM_REGRESSION_TEST_CASE_RUNTIME_ERROR_PATHS,
    FM_REGRESSION_TEST_CASE_SERVICE_STATE,
    FM_REGRESSION_TEST_CASE_SERVICE_ERROR_PATHS,
    FM_REGRESSION_TEST_CASE_VOLUME_VALUES,
    FM_REGRESSION_TEST_CASE_VOLUME_ERROR_PATHS,
    FM_REGRESSION_TEST_CASE_DISPLAY_FORMAT_VALUES,
    FM_REGRESSION_TEST_CASE_DISPLAY_FORMAT_ERROR_PATHS,
    FM_REGRESSION_TEST_CASE_LCD_MAP_ALL_SEGMENTS,
    FM_REGRESSION_TEST_CASE_LCD_MAP_LITERS_LEGEND,
    FM_REGRESSION_TEST_CASE_PRESENTATION_SEQUENCE,
    FM_REGRESSION_TEST_CASE_PRESENTATION_VALUES,
    FM_REGRESSION_TEST_CASE_PRESENTATION_FAILURE,
    FM_REGRESSION_TEST_CASE_KEYBOARD_MAPPING,
    FM_REGRESSION_TEST_CASE_MAIN_INPUT_ADAPTER,
    FM_REGRESSION_TEST_CASE_MAIN_INPUT_RECOGNIZER,
    FM_REGRESSION_TEST_CASE_MAIN_EVENT_QUEUE,
    FM_REGRESSION_TEST_CASE_COUNT
} fm_regression_test_case_t;

typedef struct
{
    fmc_model_volume_unit_t unit;
    double expected_liters_per_unit;
} fm_regression_liters_case_t;

typedef struct
{
    uint16_t previous_count;
    uint16_t current_count;
    uint64_t expected_delta;
} fm_regression_pulse_delta_case_t;

typedef struct
{
    uint16_t gpio_pin;
    fm_board_keyboard_key_t expected_key;
} fm_regression_keyboard_case_t;

typedef struct
{
    fm_board_keyboard_key_t board_key;
    fmc_input_key_t expected_input_key;
} fm_regression_main_input_adapter_case_t;

typedef struct
{
    fmc_presentation_frame_t frame;
    fm_status_t next_status;
    uint32_t call_count;
} fm_regression_presentation_sink_t;

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
static bool fm_regression_test_pulse_delta_vectors_(void);
static bool fm_regression_test_frequency_observation_vectors_(void);
static bool fm_regression_test_observer_independence_(void);
static bool fm_regression_test_runtime_frequency_result_(void);
static bool fm_regression_test_main_acquisition_(void);
static bool fm_regression_test_runtime_events_(void);
static bool fm_regression_test_runtime_input_events_(void);
static bool fm_regression_test_runtime_error_paths_(void);
static bool fm_regression_test_service_state_(void);
static bool fm_regression_test_service_error_paths_(void);
static bool fm_regression_test_volume_values_(void);
static bool fm_regression_test_volume_error_paths_(void);
static bool fm_regression_test_display_format_values_(void);
static bool fm_regression_test_display_format_error_paths_(void);
static bool fm_regression_test_lcd_map_all_segments_(void);
static bool fm_regression_test_lcd_map_liters_legend_(void);
static bool fm_regression_test_presentation_sequence_(void);
static bool fm_regression_test_presentation_values_(void);
static bool fm_regression_test_presentation_failure_(void);
static bool fm_regression_test_keyboard_mapping_(void);
static bool fm_regression_test_main_input_adapter_(void);
static bool fm_regression_test_main_input_recognizer_(void);
static bool fm_regression_test_main_event_queue_(void);
static bool fm_regression_test_run_case_(fm_regression_test_case_t p_case);
static void fm_regression_test_emit_case_(fm_regression_test_case_t p_case,
                                               bool p_passed);
static bool fm_regression_test_text_eq_(const char *p_actual,
                                              const char *p_expected);
static fm_status_t fm_regression_test_presentation_sink_(
    const fmc_presentation_frame_t *p_frame,
    void *p_context);
static void fm_regression_test_make_presentation_snapshot_(
    fmc_presentation_snapshot_t *p_snapshot,
    double p_ttl,
    double p_rate,
    frequency_observation_quality_t p_quality,
    bool p_value_present,
    fmc_model_time_base_t p_time_base);
static bool fm_regression_test_dispatch_frequency_success_(
    fmc_runtime_t *p_runtime,
    const frequency_observation_result_t *p_result,
    frequency_observation_quality_t p_expected_quality,
    bool p_expected_value_present,
    double p_expected_value);

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

static fm_status_t fm_regression_test_presentation_sink_(
    const fmc_presentation_frame_t *p_frame,
    void *p_context)
{
    fm_regression_presentation_sink_t *sink =
        (fm_regression_presentation_sink_t *) p_context;

    if ((p_frame == NULL) || (sink == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    sink->call_count++;
    sink->frame = *p_frame;

    return sink->next_status;
}

static void fm_regression_test_make_presentation_snapshot_(
    fmc_presentation_snapshot_t *p_snapshot,
    double p_ttl,
    double p_rate,
    frequency_observation_quality_t p_quality,
    bool p_value_present,
    fmc_model_time_base_t p_time_base)
{
    if (p_snapshot == NULL)
    {
        return;
    }

    p_snapshot->ttl = p_ttl;
    p_snapshot->rate.value = p_rate;
    p_snapshot->rate.quality = p_quality;
    p_snapshot->rate.value_present = p_value_present;
    p_snapshot->volume_unit = FMC_MODEL_VOLUME_UNIT_L;
    p_snapshot->rate_time_base = p_time_base;
    p_snapshot->ttl_fractional_digits =
        FMC_PRESENTATION_VALUE_FRACTIONAL_DIGITS;
    p_snapshot->rate_fractional_digits =
        FMC_PRESENTATION_VALUE_FRACTIONAL_DIGITS;
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
 * Verifies the accepted modulo-16-bit delta examples without hardware input.
 *
 * Each case initializes the accepted zero baseline, optionally advances to the
 * requested previous observation, and then checks the resulting raw delta.
 */
static bool fm_regression_test_pulse_delta_vectors_(void)
{
    static const fm_regression_pulse_delta_case_t cases[] =
    {
        {     0U,     0U,     0U },
        {     0U,    12U,    12U },
        {  1200U,  1250U,    50U },
        {  1250U,  1250U,     0U },
        { 65530U,     9U,    15U },
        {  1000U,  4500U,  3500U },
        {   100U,    99U, 65535U }
    };
    pulse_delta_observer_t observer;
    uint64_t delta;
    uint64_t first_delta;
    uint64_t second_delta;
    uint8_t index;

    for (index = 0U;
         index < (uint8_t) (sizeof(cases) / sizeof(cases[0]));
         index++)
    {
        PULSE_DELTA_Init(&observer);

        if ((cases[index].previous_count != 0U) &&
            (PULSE_DELTA_Observe(&observer,
                                 cases[index].previous_count,
                                 &delta) != FM_STATUS_OK))
        {
            return false;
        }

        if ((PULSE_DELTA_Observe(&observer,
                                 cases[index].current_count,
                                 &delta) != FM_STATUS_OK) ||
            (delta != cases[index].expected_delta))
        {
            return false;
        }
    }

    PULSE_DELTA_Init(&observer);
    if ((PULSE_DELTA_Observe(&observer, 65530U, &first_delta) !=
         FM_STATUS_OK) ||
        (PULSE_DELTA_Observe(&observer, 9U, &second_delta) != FM_STATUS_OK) ||
        ((first_delta + second_delta) != 65545U))
    {
        return false;
    }

    PULSE_DELTA_Reset(&observer);
    if ((PULSE_DELTA_Observe(&observer, 7U, &delta) != FM_STATUS_OK) ||
        (delta != 7U))
    {
        return false;
    }

    PULSE_DELTA_Init(NULL);
    PULSE_DELTA_Reset(NULL);

    return (PULSE_DELTA_Observe(NULL, 0U, &delta) == FM_STATUS_EINVAL) &&
           (PULSE_DELTA_Observe(&observer, 0U, NULL) == FM_STATUS_EINVAL);
}

/*
 * Verifies every durable 7D frequency-window decision with injected samples.
 *
 * The vectors deliberately avoid a hardware clock: physical accuracy belongs
 * to the 7E bring-up, while this test proves exact state transitions and
 * arithmetic in the RTOS-neutral observer.
 */
static bool fm_regression_test_frequency_observation_vectors_(void)
{
    frequency_observation_t observer;
    frequency_observation_sample_t sample;
    frequency_observation_result_t result;
    bool available;

    FREQUENCY_OBSERVATION_Init(&observer);

    sample.pulse_count = 100U;
    sample.timestamp_us = 1000U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE) ||
        (result.pulse_delta != 0U) ||
        (result.elapsed_us != 0U))
    {
        return false;
    }

    sample.pulse_count = 200U;
    sample.timestamp_us = 1001123U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 100U) ||
        (result.elapsed_us != 1000123U))
    {
        return false;
    }

    sample.timestamp_us += 1000000U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 0U) ||
        (result.elapsed_us != 1000000U))
    {
        return false;
    }

    FREQUENCY_OBSERVATION_Reset(&observer);
    sample.pulse_count = 65530U;
    sample.timestamp_us = 0U;
    if (FREQUENCY_OBSERVATION_Observe(&observer,
                                      &sample,
                                      &available,
                                      &result) != FM_STATUS_OK)
    {
        return false;
    }

    sample.pulse_count = 9U;
    sample.timestamp_us = FREQUENCY_OBSERVATION_MIN_ELAPSED_US;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 15U) ||
        (result.elapsed_us != FREQUENCY_OBSERVATION_MIN_ELAPSED_US))
    {
        return false;
    }

    FREQUENCY_OBSERVATION_Reset(&observer);
    sample.pulse_count = 1U;
    sample.timestamp_us = 100U;
    if (FREQUENCY_OBSERVATION_Observe(&observer,
                                      &sample,
                                      &available,
                                      &result) != FM_STATUS_OK)
    {
        return false;
    }

    sample.pulse_count = 0U;
    sample.timestamp_us =
        100U + FREQUENCY_OBSERVATION_MAX_ELAPSED_US;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 65535U) ||
        (result.elapsed_us != FREQUENCY_OBSERVATION_MAX_ELAPSED_US))
    {
        return false;
    }

    FREQUENCY_OBSERVATION_Reset(&observer);
    sample.pulse_count = 100U;
    sample.timestamp_us = 0U;
    if (FREQUENCY_OBSERVATION_Observe(&observer,
                                      &sample,
                                      &available,
                                      &result) != FM_STATUS_OK)
    {
        return false;
    }

    result.quality = FREQUENCY_OBSERVATION_QUALITY_INVALID;
    result.pulse_delta = 123U;
    result.elapsed_us = 456U;
    sample.pulse_count = 150U;
    sample.timestamp_us = 500000U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_INVALID) ||
        (result.pulse_delta != 123U) ||
        (result.elapsed_us != 456U))
    {
        return false;
    }

    sample.pulse_count = 200U;
    sample.timestamp_us = 1000000U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 100U) ||
        (result.elapsed_us != 1000000U))
    {
        return false;
    }

    sample.pulse_count = 320U;
    sample.timestamp_us = 2200001U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_STALE))
    {
        return false;
    }

    sample.pulse_count = 420U;
    sample.timestamp_us = 3200001U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 100U) ||
        (result.elapsed_us != 1000000U))
    {
        return false;
    }

    FREQUENCY_OBSERVATION_Reset(&observer);
    sample.pulse_count = 10U;
    sample.timestamp_us = 1000U;
    if (FREQUENCY_OBSERVATION_Observe(&observer,
                                      &sample,
                                      &available,
                                      &result) != FM_STATUS_OK)
    {
        return false;
    }

    sample.pulse_count = 20U;
    sample.timestamp_us = 1101001U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE))
    {
        return false;
    }

    sample.pulse_count = 30U;
    sample.timestamp_us = 2101001U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 10U))
    {
        return false;
    }

    sample.pulse_count = 40U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_INVALID))
    {
        return false;
    }

    sample.pulse_count = 50U;
    sample.timestamp_us = 500U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE))
    {
        return false;
    }

    sample.pulse_count = 60U;
    sample.timestamp_us = 400U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_INVALID))
    {
        return false;
    }

    sample.pulse_count = 70U;
    sample.timestamp_us = 100U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE))
    {
        return false;
    }

    sample.pulse_count = 80U;
    sample.timestamp_us =
        100U + FREQUENCY_OBSERVATION_MIN_ELAPSED_US;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 10U))
    {
        return false;
    }

    if ((FREQUENCY_OBSERVATION_Invalidate(&observer, &result) !=
         FM_STATUS_OK) ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_INVALID) ||
        (result.pulse_delta != 0U) ||
        (result.elapsed_us != 0U))
    {
        return false;
    }

    sample.pulse_count = 90U;
    sample.timestamp_us = 0U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE))
    {
        return false;
    }

    FREQUENCY_OBSERVATION_Reset(&observer);
    sample.pulse_count = 100U;
    sample.timestamp_us = 0U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE))
    {
        return false;
    }

    FREQUENCY_OBSERVATION_Init(NULL);
    FREQUENCY_OBSERVATION_Reset(NULL);

    return
        (FREQUENCY_OBSERVATION_Observe(NULL,
                                       &sample,
                                       &available,
                                       &result) == FM_STATUS_EINVAL) &&
        (FREQUENCY_OBSERVATION_Observe(&observer,
                                       NULL,
                                       &available,
                                       &result) == FM_STATUS_EINVAL) &&
        (FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       NULL,
                                       &result) == FM_STATUS_EINVAL) &&
        (FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       NULL) == FM_STATUS_EINVAL) &&
        (FREQUENCY_OBSERVATION_Invalidate(NULL, &result) ==
         FM_STATUS_EINVAL) &&
        (FREQUENCY_OBSERVATION_Invalidate(&observer, NULL) ==
         FM_STATUS_EINVAL);
}

/*
 * Verifies that frequency observers and pulse totalization retain independent
 * baselines even when their trusted sample sequences are interleaved.
 */
static bool fm_regression_test_observer_independence_(void)
{
    frequency_observation_t observer_a;
    frequency_observation_t observer_b;
    frequency_observation_sample_t sample;
    frequency_observation_result_t result;
    pulse_delta_observer_t pulse_observer;
    uint64_t pulse_delta;
    bool available;

    FREQUENCY_OBSERVATION_Init(&observer_a);
    FREQUENCY_OBSERVATION_Init(&observer_b);
    PULSE_DELTA_Init(&pulse_observer);

    if ((PULSE_DELTA_Observe(&pulse_observer, 100U, &pulse_delta) !=
         FM_STATUS_OK) ||
        (pulse_delta != 100U))
    {
        return false;
    }

    sample.pulse_count = 400U;
    sample.timestamp_us = 0U;
    if (FREQUENCY_OBSERVATION_Observe(&observer_a,
                                      &sample,
                                      &available,
                                      &result) != FM_STATUS_OK)
    {
        return false;
    }

    sample.pulse_count = 50U;
    sample.timestamp_us = 100U;
    if (FREQUENCY_OBSERVATION_Observe(&observer_b,
                                      &sample,
                                      &available,
                                      &result) != FM_STATUS_OK)
    {
        return false;
    }

    if ((PULSE_DELTA_Observe(&pulse_observer, 150U, &pulse_delta) !=
         FM_STATUS_OK) ||
        (pulse_delta != 50U))
    {
        return false;
    }

    sample.pulse_count = 450U;
    sample.timestamp_us = 500000U;
    result.pulse_delta = 999U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer_a,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        available ||
        (result.pulse_delta != 999U))
    {
        return false;
    }

    sample.pulse_count = 70U;
    sample.timestamp_us =
        100U + FREQUENCY_OBSERVATION_MIN_ELAPSED_US;
    if ((FREQUENCY_OBSERVATION_Observe(&observer_b,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 20U))
    {
        return false;
    }

    if ((PULSE_DELTA_Observe(&pulse_observer, 10U, &pulse_delta) !=
         FM_STATUS_OK) ||
        (pulse_delta != 65396U))
    {
        return false;
    }

    sample.pulse_count = 500U;
    sample.timestamp_us = 1000000U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer_a,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 100U) ||
        (result.elapsed_us != 1000000U))
    {
        return false;
    }

    FREQUENCY_OBSERVATION_Reset(&observer_b);

    if ((PULSE_DELTA_Observe(&pulse_observer, 20U, &pulse_delta) !=
         FM_STATUS_OK) ||
        (pulse_delta != 10U))
    {
        return false;
    }

    sample.pulse_count = 600U;
    sample.timestamp_us = 2000000U;

    return
        (FREQUENCY_OBSERVATION_Observe(&observer_a,
                                       &sample,
                                       &available,
                                       &result) == FM_STATUS_OK) &&
        available &&
        (result.quality == FREQUENCY_OBSERVATION_QUALITY_VALID) &&
        (result.pulse_delta == 100U) &&
        (result.elapsed_us == 1000000U);
}

/*
 * Verifies the deterministic 7F integration chain from admitted pulse/time
 * observations through runtime-owned RATE state. Physical sampling remains a
 * later product-main composition responsibility.
 */
static bool fm_regression_test_runtime_frequency_result_(void)
{
    frequency_observation_t observer;
    frequency_observation_sample_t sample;
    frequency_observation_result_t result;
    fmc_runtime_event_t event;
    fmc_runtime_rate_state_t rate_state;
    fmc_service_snapshot_t snapshot;
    fmc_runtime_t runtime;
    bool available;

    FREQUENCY_OBSERVATION_Init(&observer);
    FMC_RUNTIME_Init(&runtime);
    runtime.service.model.acm.pulses = 123U;
    runtime.service.model.ttl.pulses = 456U;

    if ((FMC_RUNTIME_GetRateState(&runtime, &rate_state) != FM_STATUS_OK) ||
        (rate_state.quality !=
         FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE) ||
        rate_state.value_present ||
        !fm_regression_test_double_eq_(rate_state.value, 0.0) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime))
    {
        return false;
    }

    /* The first trusted sample is reported and applied as unavailable. */
    sample.pulse_count = 10U;
    sample.timestamp_us = 1000U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE) ||
        !fm_regression_test_dispatch_frequency_success_(
            &runtime,
            &result,
            FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE,
            false,
            0.0))
    {
        return false;
    }

    /*
     * An early sample produces no result, so integration has no event to
     * dispatch and runtime remains unchanged.
     */
    sample.pulse_count = 15U;
    sample.timestamp_us = 501000U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        available ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        (FMC_RUNTIME_GetRateState(&runtime, &rate_state) != FM_STATUS_OK) ||
        (rate_state.quality !=
         FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE) ||
        rate_state.value_present)
    {
        return false;
    }

    /* Nine pulses over the inclusive 900 ms limit produce exactly 10 L/s. */
    sample.pulse_count = 19U;
    sample.timestamp_us =
        1000U + FREQUENCY_OBSERVATION_MIN_ELAPSED_US;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 9U) ||
        (result.elapsed_us != FREQUENCY_OBSERVATION_MIN_ELAPSED_US) ||
        !fm_regression_test_dispatch_frequency_success_(
            &runtime,
            &result,
            FREQUENCY_OBSERVATION_QUALITY_VALID,
            true,
            10.0))
    {
        return false;
    }

    /* Eleven pulses over the inclusive 1.1 s limit also produce 10 L/s. */
    sample.pulse_count = 30U;
    sample.timestamp_us =
        1000U +
        FREQUENCY_OBSERVATION_MIN_ELAPSED_US +
        FREQUENCY_OBSERVATION_MAX_ELAPSED_US;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 11U) ||
        (result.elapsed_us != FREQUENCY_OBSERVATION_MAX_ELAPSED_US) ||
        !fm_regression_test_dispatch_frequency_success_(
            &runtime,
            &result,
            FREQUENCY_OBSERVATION_QUALITY_VALID,
            true,
            10.0))
    {
        return false;
    }

    /* A complete zero-pulse window is a present and valid numeric zero. */
    sample.timestamp_us += 1000000U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 0U) ||
        !fm_regression_test_dispatch_frequency_success_(
            &runtime,
            &result,
            FREQUENCY_OBSERVATION_QUALITY_VALID,
            true,
            0.0))
    {
        return false;
    }

    /* A late sample becomes stale, resynchronizes, and retains numeric zero. */
    sample.pulse_count = 31U;
    sample.timestamp_us +=
        FREQUENCY_OBSERVATION_MAX_ELAPSED_US + 100001U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_STALE) ||
        !fm_regression_test_dispatch_frequency_success_(
            &runtime,
            &result,
            FREQUENCY_OBSERVATION_QUALITY_STALE,
            true,
            0.0))
    {
        return false;
    }

    /* A normal sample after stale proves recovery from the new baseline. */
    sample.pulse_count = 41U;
    sample.timestamp_us += 1000000U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 10U) ||
        !fm_regression_test_dispatch_frequency_success_(
            &runtime,
            &result,
            FREQUENCY_OBSERVATION_QUALITY_VALID,
            true,
            10.0))
    {
        return false;
    }

    /* Equal monotonic time is invalid and retains the last numeric RATE. */
    sample.pulse_count = 50U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_INVALID) ||
        !fm_regression_test_dispatch_frequency_success_(
            &runtime,
            &result,
            FREQUENCY_OBSERVATION_QUALITY_INVALID,
            true,
            10.0))
    {
        return false;
    }

    /* Invalid clears the observer, so the next sample is a new baseline. */
    sample.pulse_count = 60U;
    sample.timestamp_us = 6000000U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE) ||
        !fm_regression_test_dispatch_frequency_success_(
            &runtime,
            &result,
            FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE,
            false,
            0.0))
    {
        return false;
    }

    sample.pulse_count = 70U;
    sample.timestamp_us += 1000000U;
    if ((FREQUENCY_OBSERVATION_Observe(&observer,
                                       &sample,
                                       &available,
                                       &result) != FM_STATUS_OK) ||
        !available ||
        (result.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        (result.pulse_delta != 10U) ||
        !fm_regression_test_dispatch_frequency_success_(
            &runtime,
            &result,
            FREQUENCY_OBSERVATION_QUALITY_VALID,
            true,
            10.0))
    {
        return false;
    }

    /*
     * Direct malformed runtime events test atomicity below the observer
     * boundary: no RATE state or pending flag may change on failure.
     */
    event.kind = FMC_RUNTIME_EVENT_FREQUENCY_RESULT;
    event.data.frequency_result.quality =
        FREQUENCY_OBSERVATION_QUALITY_VALID;
    event.data.frequency_result.pulse_delta = 1U;
    event.data.frequency_result.elapsed_us = 0U;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_ERANGE) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        (FMC_RUNTIME_GetRateState(&runtime, &rate_state) != FM_STATUS_OK) ||
        (rate_state.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        !rate_state.value_present ||
        !fm_regression_test_double_eq_(rate_state.value, 10.0))
    {
        return false;
    }

    runtime.service.model.measurement.calibration_pulses_per_unit =
        FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_MIN - 1.0;
    event.data.frequency_result.elapsed_us = 1000000U;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_ERANGE) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        (FMC_RUNTIME_GetRateState(&runtime, &rate_state) != FM_STATUS_OK) ||
        (rate_state.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        !rate_state.value_present ||
        !fm_regression_test_double_eq_(rate_state.value, 10.0))
    {
        return false;
    }
    runtime.service.model.measurement.calibration_pulses_per_unit =
        FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_DEFAULT;

    event.data.frequency_result.quality =
        (frequency_observation_quality_t) 99;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_EINVAL) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        (FMC_RUNTIME_GetRateState(&runtime, &rate_state) != FM_STATUS_OK) ||
        (rate_state.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        !rate_state.value_present ||
        !fm_regression_test_double_eq_(rate_state.value, 10.0))
    {
        return false;
    }

    /*
     * A final snapshot proves the entire frequency sequence conserved both
     * independent totalizers; no tested path can silently update ACM or TTL.
     */
    return
        (FMC_RUNTIME_GetRateState(NULL, &rate_state) == FM_STATUS_EINVAL) &&
        (FMC_RUNTIME_GetRateState(&runtime, NULL) == FM_STATUS_EINVAL) &&
        (FMC_RUNTIME_GetSnapshot(&runtime, &snapshot) == FM_STATUS_OK) &&
        (snapshot.model.acm.pulses == 123U) &&
        (snapshot.model.ttl.pulses == 456U);
}

/*
 * Verifies the exactly-once product-main handoff from one trusted counter
 * observation to both canonical totals.
 */
static bool fm_regression_test_main_acquisition_(void)
{
    fm_main_acquisition_t acquisition;
    frequency_observation_sample_t frequency_sample;
    fmc_runtime_rate_state_t rate_state;
    fmc_runtime_t runtime;
    fmc_service_snapshot_t snapshot;

    FM_MAIN_ACQUISITION_Init(&acquisition);
    FMC_RUNTIME_Init(&runtime);

    if ((FM_MAIN_ACQUISITION_ProcessObservation(&acquisition,
                                                12U,
                                                &runtime) != FM_STATUS_OK) ||
        !FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        (FMC_RUNTIME_GetSnapshot(&runtime, &snapshot) != FM_STATUS_OK) ||
        (snapshot.model.acm.pulses != 12U) ||
        (snapshot.model.ttl.pulses != 12U) ||
        (FMC_RUNTIME_ClearPresentationUpdatePending(&runtime) !=
         FM_STATUS_OK))
    {
        return false;
    }

    if ((FM_MAIN_ACQUISITION_ProcessObservation(&acquisition,
                                                12U,
                                                &runtime) != FM_STATUS_OK) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        (FMC_RUNTIME_GetSnapshot(&runtime, &snapshot) != FM_STATUS_OK) ||
        (snapshot.model.acm.pulses != 12U) ||
        (snapshot.model.ttl.pulses != 12U))
    {
        return false;
    }

    if ((FM_MAIN_ACQUISITION_ProcessObservation(&acquisition,
                                                20U,
                                                &runtime) != FM_STATUS_OK) ||
        (FMC_RUNTIME_GetSnapshot(&runtime, &snapshot) != FM_STATUS_OK) ||
        (snapshot.model.acm.pulses != 20U) ||
        (snapshot.model.ttl.pulses != 20U))
    {
        return false;
    }

    FM_MAIN_ACQUISITION_Init(&acquisition);
    FMC_RUNTIME_Init(&runtime);
    if ((FM_MAIN_ACQUISITION_ProcessObservation(&acquisition,
                                                65530U,
                                                &runtime) != FM_STATUS_OK) ||
        (FM_MAIN_ACQUISITION_ProcessObservation(&acquisition,
                                                9U,
                                                &runtime) != FM_STATUS_OK) ||
        (FMC_RUNTIME_GetSnapshot(&runtime, &snapshot) != FM_STATUS_OK) ||
        (snapshot.model.acm.pulses != 65545U) ||
        (snapshot.model.ttl.pulses != 65545U))
    {
        return false;
    }

    if ((FM_MAIN_ACQUISITION_ProcessObservation(NULL,
                                                0U,
                                                &runtime) !=
         FM_STATUS_EINVAL) ||
        (FM_MAIN_ACQUISITION_ProcessObservation(&acquisition,
                                                0U,
                                                NULL) !=
         FM_STATUS_EINVAL))
    {
        return false;
    }

    /*
     * The two acquisition operations own independent baselines. Totalization
     * can advance before frequency is usable, an early frequency sample emits
     * no event, and frequency dispatch never changes either total.
     */
    FM_MAIN_ACQUISITION_Init(&acquisition);
    FMC_RUNTIME_Init(&runtime);
    if ((FM_MAIN_ACQUISITION_ProcessObservation(&acquisition,
                                                12U,
                                                &runtime) != FM_STATUS_OK) ||
        (FMC_RUNTIME_ClearPresentationUpdatePending(&runtime) !=
         FM_STATUS_OK))
    {
        return false;
    }

    frequency_sample.pulse_count = 100U;
    frequency_sample.timestamp_us = 1000U;
    if ((FM_MAIN_ACQUISITION_ProcessFrequencyObservation(
             &acquisition,
             &frequency_sample,
             &runtime) != FM_STATUS_OK) ||
        !FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        (FMC_RUNTIME_GetRateState(&runtime, &rate_state) != FM_STATUS_OK) ||
        (rate_state.quality !=
         FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE) ||
        rate_state.value_present ||
        (FMC_RUNTIME_ClearPresentationUpdatePending(&runtime) !=
         FM_STATUS_OK))
    {
        return false;
    }

    frequency_sample.pulse_count = 150U;
    frequency_sample.timestamp_us = 501000U;
    if ((FM_MAIN_ACQUISITION_ProcessFrequencyObservation(
             &acquisition,
             &frequency_sample,
             &runtime) != FM_STATUS_OK) ||
        FMC_RUNTIME_PresentationUpdateIsPending(&runtime))
    {
        return false;
    }

    if ((FM_MAIN_ACQUISITION_ProcessObservation(&acquisition,
                                                20U,
                                                &runtime) != FM_STATUS_OK) ||
        (FMC_RUNTIME_ClearPresentationUpdatePending(&runtime) !=
         FM_STATUS_OK))
    {
        return false;
    }

    frequency_sample.pulse_count = 200U;
    frequency_sample.timestamp_us = 1001000U;
    if ((FM_MAIN_ACQUISITION_ProcessFrequencyObservation(
             &acquisition,
             &frequency_sample,
             &runtime) != FM_STATUS_OK) ||
        !FMC_RUNTIME_PresentationUpdateIsPending(&runtime) ||
        (FMC_RUNTIME_GetRateState(&runtime, &rate_state) != FM_STATUS_OK) ||
        (rate_state.quality != FREQUENCY_OBSERVATION_QUALITY_VALID) ||
        !rate_state.value_present ||
        !fm_regression_test_double_eq_(rate_state.value, 100.0) ||
        (FMC_RUNTIME_GetSnapshot(&runtime, &snapshot) != FM_STATUS_OK) ||
        (snapshot.model.acm.pulses != 20U) ||
        (snapshot.model.ttl.pulses != 20U) ||
        (FM_MAIN_ACQUISITION_ProcessFrequencyObservation(
             NULL,
             &frequency_sample,
             &runtime) != FM_STATUS_EINVAL) ||
        (FM_MAIN_ACQUISITION_ProcessFrequencyObservation(
             &acquisition,
             NULL,
             &runtime) != FM_STATUS_EINVAL) ||
        (FM_MAIN_ACQUISITION_ProcessFrequencyObservation(
             &acquisition,
             &frequency_sample,
             NULL) != FM_STATUS_EINVAL))
    {
        return false;
    }

    FM_MAIN_ACQUISITION_Init(NULL);
    FM_MAIN_ACQUISITION_Init(&acquisition);
    FMC_RUNTIME_Init(&runtime);
    runtime.service.model.acm.pulses = UINT64_MAX;

    return (FM_MAIN_ACQUISITION_ProcessObservation(&acquisition,
                                                   1U,
                                                   &runtime) ==
            FM_STATUS_ERANGE) &&
           (runtime.service.model.acm.pulses == UINT64_MAX) &&
           (runtime.service.model.ttl.pulses == 0U) &&
           !FMC_RUNTIME_PresentationUpdateIsPending(&runtime);
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
 * Verifies that the all-segments mapping operation covers the complete
 * controller RAM image and validates its public buffer contract.
 */
static bool fm_regression_test_lcd_map_all_segments_(void)
{
    uint8_t ram[FM_LCD_MAP_RAM_SIZE + 1U];
    uint8_t index;

    for (index = 0U; index < (uint8_t) sizeof(ram); index++)
    {
        ram[index] = 0U;
    }

    if ((FM_LCD_MAP_SetAll(NULL, FM_LCD_MAP_RAM_SIZE) !=
         FM_LCD_MAP_EINVAL) ||
        (FM_LCD_MAP_SetAll(ram, FM_LCD_MAP_RAM_SIZE - 1U) !=
         FM_LCD_MAP_ERANGE) ||
        (FM_LCD_MAP_SetAll(ram, (uint8_t) sizeof(ram)) !=
         FM_LCD_MAP_OK))
    {
        return false;
    }

    for (index = 0U; index < FM_LCD_MAP_RAM_SIZE; index++)
    {
        if (ram[index] != 0xFFU)
        {
            return false;
        }
    }

    return ram[FM_LCD_MAP_RAM_SIZE] == 0U;
}

/*
 * Verifies that the visible liters legend uses both alphanumeric positions:
 * `L` in the left position and a non-blank `t` in the right position.
 */
static bool fm_regression_test_lcd_map_liters_legend_(void)
{
    uint8_t liters[FM_LCD_MAP_RAM_SIZE] = {0};
    uint8_t left_l[FM_LCD_MAP_RAM_SIZE] = {0};
    uint8_t right_t[FM_LCD_MAP_RAM_SIZE] = {0};
    bool left_has_segments = false;
    bool right_has_segments = false;
    uint8_t index;

    if ((FM_LCD_MAP_WriteAlpha(liters,
                               FM_LCD_MAP_RAM_SIZE,
                               "Lt",
                               FM_LCD_ALIGN_LEFT,
                               true) != FM_LCD_MAP_OK) ||
        (FM_LCD_MAP_WriteAlpha(left_l,
                               FM_LCD_MAP_RAM_SIZE,
                               "L ",
                               FM_LCD_ALIGN_LEFT,
                               true) != FM_LCD_MAP_OK) ||
        (FM_LCD_MAP_WriteAlpha(right_t,
                               FM_LCD_MAP_RAM_SIZE,
                               " t",
                               FM_LCD_ALIGN_LEFT,
                               true) != FM_LCD_MAP_OK))
    {
        return false;
    }

    for (index = 0U; index < FM_LCD_MAP_RAM_SIZE; index++)
    {
        if (liters[index] != (uint8_t) (left_l[index] | right_t[index]))
        {
            return false;
        }

        left_has_segments = left_has_segments || (left_l[index] != 0U);
        right_has_segments = right_has_segments || (right_t[index] != 0U);
    }

    return left_has_segments && right_has_segments;
}

/*
 * Verifies the bounded startup order, SHORT ESC transition, and that first
 * entry to TTL/RATE uses the latest supplied live snapshot atomically.
 */
static bool fm_regression_test_presentation_sequence_(void)
{
    fmc_presentation_t presentation;
    fmc_presentation_snapshot_t snapshot;
    fm_regression_presentation_sink_t sink = {0};
    fmc_input_event_t input;
    uint32_t calls_before_stable_advance;

    sink.next_status = FM_STATUS_OK;
    fm_regression_test_make_presentation_snapshot_(
        &snapshot,
        0.0,
        0.0,
        FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE,
        false,
        FMC_MODEL_TIME_BASE_SECOND);

    if ((FMC_PRESENTATION_Init(&presentation,
                               &snapshot,
                               fm_regression_test_presentation_sink_,
                               &sink) != FM_STATUS_OK) ||
        (FMC_PRESENTATION_GetState(&presentation) !=
         FMC_PRESENTATION_STATE_NOT_STARTED) ||
        (FMC_PRESENTATION_Start(&presentation) != FM_STATUS_OK) ||
        (FMC_PRESENTATION_GetState(&presentation) !=
         FMC_PRESENTATION_STATE_ALL_SEGMENTS) ||
        !sink.frame.all_segments)
    {
        return false;
    }

    input.key = FMC_INPUT_KEY_ESC;
    input.action = FMC_INPUT_ACTION_SHORT;
    if ((FMC_PRESENTATION_HandleInput(&presentation,
                                      &input,
                                      &snapshot) !=
         FM_STATUS_OK) ||
        (FMC_PRESENTATION_GetState(&presentation) !=
         FMC_PRESENTATION_STATE_FIRMWARE_VERSION) ||
        sink.frame.all_segments ||
        !fm_regression_test_text_eq_(sink.frame.top_text, "") ||
        !fm_regression_test_text_eq_(sink.frame.bottom_text, "00.01.00") ||
        !fm_regression_test_text_eq_(sink.frame.alpha_text, "B0") ||
        sink.frame.indicator_ttl ||
        sink.frame.indicator_rate ||
        sink.frame.indicator_slash ||
        sink.frame.indicator_second ||
        sink.frame.indicator_minute)
    {
        return false;
    }

    fm_regression_test_make_presentation_snapshot_(
        &snapshot,
        42.5,
        100.0,
        FREQUENCY_OBSERVATION_QUALITY_VALID,
        true,
        FMC_MODEL_TIME_BASE_SECOND);
    if ((FMC_PRESENTATION_Advance(&presentation,
                                  &snapshot) != FM_STATUS_OK) ||
        (FMC_PRESENTATION_GetState(&presentation) !=
         FMC_PRESENTATION_STATE_TTL_RATE) ||
        !fm_regression_test_text_eq_(sink.frame.top_text, "     42.5") ||
        !fm_regression_test_text_eq_(sink.frame.bottom_text, "   100.0") ||
        !fm_regression_test_text_eq_(sink.frame.alpha_text, "Lt") ||
        !sink.frame.indicator_ttl ||
        !sink.frame.indicator_rate ||
        !sink.frame.indicator_slash ||
        !sink.frame.indicator_second ||
        sink.frame.indicator_minute)
    {
        return false;
    }

    calls_before_stable_advance = sink.call_count;

    return (FMC_PRESENTATION_Advance(&presentation,
                                     &snapshot) == FM_STATUS_OK) &&
           (sink.call_count == calls_before_stable_advance);
}

/*
 * Verifies numeric live values, both bounded time indicators, visual overflow,
 * and the common RATE representation for every admitted nonvalid quality.
 */
static bool fm_regression_test_presentation_values_(void)
{
    fmc_presentation_t presentation;
    fmc_presentation_snapshot_t snapshot;
    fm_regression_presentation_sink_t sink = {0};

    sink.next_status = FM_STATUS_OK;
    fm_regression_test_make_presentation_snapshot_(
        &snapshot,
        1234.5,
        12.3,
        FREQUENCY_OBSERVATION_QUALITY_VALID,
        true,
        FMC_MODEL_TIME_BASE_SECOND);

    if ((FMC_PRESENTATION_Init(&presentation,
                               &snapshot,
                               fm_regression_test_presentation_sink_,
                               &sink) != FM_STATUS_OK) ||
        (FMC_PRESENTATION_Start(&presentation) != FM_STATUS_OK) ||
        (FMC_PRESENTATION_Advance(&presentation,
                                  &snapshot) != FM_STATUS_OK) ||
        (FMC_PRESENTATION_Advance(&presentation,
                                  &snapshot) != FM_STATUS_OK))
    {
        return false;
    }

    snapshot.ttl = 1.26;
    snapshot.rate.value = 0.0;
    if ((FMC_PRESENTATION_Refresh(&presentation, &snapshot) !=
         FM_STATUS_OK) ||
        !fm_regression_test_text_eq_(sink.frame.top_text, "      1.3") ||
        !fm_regression_test_text_eq_(sink.frame.bottom_text, "     0.0") ||
        sink.frame.ttl_overflow ||
        sink.frame.rate_overflow ||
        !sink.frame.indicator_second ||
        sink.frame.indicator_minute)
    {
        return false;
    }

    snapshot.ttl = 123456789.14;
    snapshot.rate.value = 9876543.26;
    snapshot.rate_time_base = FMC_MODEL_TIME_BASE_MINUTE;
    if ((FMC_PRESENTATION_Refresh(&presentation, &snapshot) !=
         FM_STATUS_OK) ||
        !fm_regression_test_text_eq_(sink.frame.top_text, "3456789.1") ||
        !fm_regression_test_text_eq_(sink.frame.bottom_text, "876543.3") ||
        !sink.frame.ttl_overflow ||
        !sink.frame.rate_overflow ||
        !sink.frame.indicator_ttl ||
        !sink.frame.indicator_rate ||
        !sink.frame.indicator_slash ||
        sink.frame.indicator_second ||
        !sink.frame.indicator_minute)
    {
        return false;
    }

    snapshot.ttl = 15.0;
    snapshot.rate.quality =
        FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE;
    snapshot.rate.value_present = false;
    if ((FMC_PRESENTATION_Refresh(&presentation, &snapshot) !=
         FM_STATUS_OK) ||
        !fm_regression_test_text_eq_(sink.frame.bottom_text, "-------") ||
        !sink.frame.indicator_ttl ||
        !sink.frame.indicator_rate ||
        !sink.frame.indicator_slash ||
        sink.frame.indicator_second ||
        !sink.frame.indicator_minute)
    {
        return false;
    }

    snapshot.rate.quality = FREQUENCY_OBSERVATION_QUALITY_STALE;
    snapshot.rate.value_present = true;
    if ((FMC_PRESENTATION_Refresh(&presentation, &snapshot) !=
         FM_STATUS_OK) ||
        !fm_regression_test_text_eq_(sink.frame.bottom_text, "-------"))
    {
        return false;
    }

    snapshot.rate.quality = FREQUENCY_OBSERVATION_QUALITY_INVALID;
    snapshot.rate.value_present = false;

    return (FMC_PRESENTATION_Refresh(&presentation, &snapshot) ==
            FM_STATUS_OK) &&
           fm_regression_test_text_eq_(sink.frame.bottom_text, "-------");
}

/*
 * Verifies that a failed sink call propagates and does not confirm a new
 * presentation state or snapshot.
 */
static bool fm_regression_test_presentation_failure_(void)
{
    fmc_presentation_t presentation;
    fmc_presentation_snapshot_t snapshot;
    fm_regression_presentation_sink_t sink = {0};
    fmc_runtime_event_t event;
    fmc_runtime_t runtime;

    sink.next_status = FM_STATUS_ESTATE;
    fm_regression_test_make_presentation_snapshot_(
        &snapshot,
        1234.5,
        12.3,
        FREQUENCY_OBSERVATION_QUALITY_VALID,
        true,
        FMC_MODEL_TIME_BASE_SECOND);

    if ((FMC_PRESENTATION_Init(&presentation,
                               &snapshot,
                               fm_regression_test_presentation_sink_,
                               &sink) != FM_STATUS_OK) ||
        (FMC_PRESENTATION_Start(&presentation) != FM_STATUS_ESTATE) ||
        (FMC_PRESENTATION_GetState(&presentation) !=
         FMC_PRESENTATION_STATE_NOT_STARTED))
    {
        return false;
    }

    sink.next_status = FM_STATUS_OK;
    if ((FMC_PRESENTATION_Start(&presentation) != FM_STATUS_OK) ||
        (FMC_PRESENTATION_GetState(&presentation) !=
         FMC_PRESENTATION_STATE_ALL_SEGMENTS))
    {
        return false;
    }

    sink.next_status = FM_STATUS_ESTATE;
    if ((FMC_PRESENTATION_Advance(&presentation,
                                  &snapshot) != FM_STATUS_ESTATE) ||
        (FMC_PRESENTATION_GetState(&presentation) !=
         FMC_PRESENTATION_STATE_ALL_SEGMENTS))
    {
        return false;
    }

    sink.next_status = FM_STATUS_OK;
    if ((FMC_PRESENTATION_Advance(&presentation,
                                  &snapshot) != FM_STATUS_OK) ||
        (FMC_PRESENTATION_GetState(&presentation) !=
         FMC_PRESENTATION_STATE_FIRMWARE_VERSION))
    {
        return false;
    }

    snapshot.rate.quality =
        FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE;
    snapshot.rate.value_present = true;
    if (FMC_PRESENTATION_Advance(&presentation,
                                 &snapshot) != FM_STATUS_EINVAL)
    {
        return false;
    }

    snapshot.rate.value_present = false;
    snapshot.rate.quality = (frequency_observation_quality_t) 99;
    if (FMC_PRESENTATION_Advance(&presentation,
                                 &snapshot) != FM_STATUS_EINVAL)
    {
        return false;
    }

    /*
     * The runtime pending flag is an integration acknowledgement: failed
     * presentation leaves it set; only the caller clears it after success.
     */
    fm_regression_test_make_presentation_snapshot_(
        &snapshot,
        1.0,
        2.0,
        FREQUENCY_OBSERVATION_QUALITY_VALID,
        true,
        FMC_MODEL_TIME_BASE_SECOND);
    if (FMC_PRESENTATION_Advance(&presentation,
                                 &snapshot) != FM_STATUS_OK)
    {
        return false;
    }

    FMC_RUNTIME_Init(&runtime);
    event.kind = FMC_RUNTIME_EVENT_PULSE_DELTA;
    event.data.pulse_delta = 1U;
    if ((FMC_RUNTIME_Dispatch(&runtime, &event) != FM_STATUS_OK) ||
        !FMC_RUNTIME_PresentationUpdateIsPending(&runtime))
    {
        return false;
    }

    sink.next_status = FM_STATUS_ESTATE;
    if ((FMC_PRESENTATION_Refresh(&presentation, &snapshot) !=
         FM_STATUS_ESTATE) ||
        !FMC_RUNTIME_PresentationUpdateIsPending(&runtime))
    {
        return false;
    }

    sink.next_status = FM_STATUS_OK;

    return (FMC_PRESENTATION_Refresh(&presentation, &snapshot) ==
            FM_STATUS_OK) &&
           FMC_RUNTIME_PresentationUpdateIsPending(&runtime) &&
           (FMC_RUNTIME_ClearPresentationUpdatePending(&runtime) ==
            FM_STATUS_OK) &&
           !FMC_RUNTIME_PresentationUpdateIsPending(&runtime);
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

/*
 * Verifies the product app adapter from board keyboard identity to provisional
 * runtime SHORT input events.
 */
static bool fm_regression_test_main_input_adapter_(void)
{
    static const fm_regression_main_input_adapter_case_t cases[] =
    {
        { FM_BOARD_KEYBOARD_KEY_DOWN, FMC_INPUT_KEY_DOWN },
        { FM_BOARD_KEYBOARD_KEY_UP, FMC_INPUT_KEY_UP },
        { FM_BOARD_KEYBOARD_KEY_ENTER, FMC_INPUT_KEY_ENTER },
        { FM_BOARD_KEYBOARD_KEY_ESC, FMC_INPUT_KEY_ESC }
    };
    fmc_runtime_event_t event;
    uint8_t index;

    for (index = 0U;
         index < (uint8_t) (sizeof(cases) / sizeof(cases[0]));
         index++)
    {
        event.kind = FMC_RUNTIME_EVENT_NONE;
        event.data.input.key = FMC_INPUT_KEY_COUNT;
        event.data.input.action = FMC_INPUT_ACTION_COUNT;

        if (!FM_MAIN_INPUT_ADAPTER_ShortEventFromBoardKey(
                cases[index].board_key,
                &event) ||
            (event.kind != FMC_RUNTIME_EVENT_INPUT) ||
            (event.data.input.key != cases[index].expected_input_key) ||
            (event.data.input.action != FMC_INPUT_ACTION_SHORT))
        {
            return false;
        }
    }

    event.kind = FMC_RUNTIME_EVENT_NONE;
    event.data.input.key = FMC_INPUT_KEY_DOWN;
    event.data.input.action = FMC_INPUT_ACTION_SHORT;

    if (FM_MAIN_INPUT_ADAPTER_ShortEventFromBoardKey(
            FM_BOARD_KEYBOARD_KEY_COUNT,
            &event) ||
        (event.kind != FMC_RUNTIME_EVENT_NONE) ||
        (event.data.input.key != FMC_INPUT_KEY_DOWN) ||
        (event.data.input.action != FMC_INPUT_ACTION_SHORT))
    {
        return false;
    }

    if (FM_MAIN_INPUT_ADAPTER_ShortEventFromBoardKey(
            (fm_board_keyboard_key_t) -1,
            &event) ||
        (event.kind != FMC_RUNTIME_EVENT_NONE) ||
        (event.data.input.key != FMC_INPUT_KEY_DOWN) ||
        (event.data.input.action != FMC_INPUT_ACTION_SHORT))
    {
        return false;
    }

    event.kind = FMC_RUNTIME_EVENT_NONE;
    event.data.input.key = FMC_INPUT_KEY_COUNT;
    event.data.input.action = FMC_INPUT_ACTION_COUNT;

    if (!FM_MAIN_INPUT_ADAPTER_EventFromBoardKey(
            FM_BOARD_KEYBOARD_KEY_ESC,
            FMC_INPUT_ACTION_LONG,
            &event) ||
        (event.kind != FMC_RUNTIME_EVENT_INPUT) ||
        (event.data.input.key != FMC_INPUT_KEY_ESC) ||
        (event.data.input.action != FMC_INPUT_ACTION_LONG))
    {
        return false;
    }

    event.kind = FMC_RUNTIME_EVENT_NONE;
    event.data.input.key = FMC_INPUT_KEY_ESC;
    event.data.input.action = FMC_INPUT_ACTION_LONG;

    if (FM_MAIN_INPUT_ADAPTER_EventFromBoardKey(
            FM_BOARD_KEYBOARD_KEY_ESC,
            FMC_INPUT_ACTION_COUNT,
            &event) ||
        (event.kind != FMC_RUNTIME_EVENT_NONE) ||
        (event.data.input.key != FMC_INPUT_KEY_ESC) ||
        (event.data.input.action != FMC_INPUT_ACTION_LONG))
    {
        return false;
    }

    return !FM_MAIN_INPUT_ADAPTER_ShortEventFromBoardKey(
        FM_BOARD_KEYBOARD_KEY_DOWN,
        NULL);
}

/*
 * Verifies the product/main mechanical-key recognizer contract.
 *
 * RISING starts the hold, timeout emits one LONG, and FALLING emits SHORT only
 * when the hold did not already emit LONG.
 */
static bool fm_regression_test_main_input_recognizer_(void)
{
    fm_main_input_recognizer_t recognizer;
    fm_main_input_recognizer_output_t output = {0};

    FM_MAIN_INPUT_RECOGNIZER_Init(&recognizer);

    if ((FM_MAIN_INPUT_RECOGNIZER_HandleKeyboard(
             NULL,
             FM_BOARD_KEYBOARD_KEY_DOWN,
             FM_BOARD_KEYBOARD_EDGE_RISING,
             &output) != FM_STATUS_EINVAL) ||
        (FM_MAIN_INPUT_RECOGNIZER_HandleKeyboard(
             &recognizer,
             FM_BOARD_KEYBOARD_KEY_COUNT,
             FM_BOARD_KEYBOARD_EDGE_RISING,
             &output) != FM_STATUS_EINVAL) ||
        (FM_MAIN_INPUT_RECOGNIZER_HandleKeyboard(
             &recognizer,
             FM_BOARD_KEYBOARD_KEY_DOWN,
             (fm_board_keyboard_edge_t) 99,
             &output) != FM_STATUS_EINVAL) ||
        (FM_MAIN_INPUT_RECOGNIZER_HandleHoldTimeout(
             NULL,
             &output) != FM_STATUS_EINVAL))
    {
        return false;
    }

    if ((FM_MAIN_INPUT_RECOGNIZER_HandleKeyboard(
             &recognizer,
             FM_BOARD_KEYBOARD_KEY_DOWN,
             FM_BOARD_KEYBOARD_EDGE_FALLING,
             &output) != FM_STATUS_ESTATE) ||
        (output.timer_action != FM_MAIN_INPUT_RECOGNIZER_TIMER_NONE) ||
        output.runtime_event_valid)
    {
        return false;
    }

    if ((FM_MAIN_INPUT_RECOGNIZER_HandleKeyboard(
             &recognizer,
             FM_BOARD_KEYBOARD_KEY_DOWN,
             FM_BOARD_KEYBOARD_EDGE_RISING,
             &output) != FM_STATUS_OK) ||
        (output.timer_action != FM_MAIN_INPUT_RECOGNIZER_TIMER_START) ||
        output.runtime_event_valid)
    {
        return false;
    }

    if ((FM_MAIN_INPUT_RECOGNIZER_HandleKeyboard(
             &recognizer,
             FM_BOARD_KEYBOARD_KEY_DOWN,
             FM_BOARD_KEYBOARD_EDGE_FALLING,
             &output) != FM_STATUS_OK) ||
        (output.timer_action != FM_MAIN_INPUT_RECOGNIZER_TIMER_CANCEL) ||
        !output.runtime_event_valid ||
        (output.runtime_event.kind != FMC_RUNTIME_EVENT_INPUT) ||
        (output.runtime_event.data.input.key != FMC_INPUT_KEY_DOWN) ||
        (output.runtime_event.data.input.action != FMC_INPUT_ACTION_SHORT))
    {
        return false;
    }

    if ((FM_MAIN_INPUT_RECOGNIZER_HandleKeyboard(
             &recognizer,
             FM_BOARD_KEYBOARD_KEY_UP,
             FM_BOARD_KEYBOARD_EDGE_RISING,
             &output) != FM_STATUS_OK) ||
        (output.timer_action != FM_MAIN_INPUT_RECOGNIZER_TIMER_START) ||
        output.runtime_event_valid)
    {
        return false;
    }

    if ((FM_MAIN_INPUT_RECOGNIZER_HandleHoldTimeout(
             &recognizer,
             &output) != FM_STATUS_OK) ||
        (output.timer_action != FM_MAIN_INPUT_RECOGNIZER_TIMER_NONE) ||
        !output.runtime_event_valid ||
        (output.runtime_event.kind != FMC_RUNTIME_EVENT_INPUT) ||
        (output.runtime_event.data.input.key != FMC_INPUT_KEY_UP) ||
        (output.runtime_event.data.input.action != FMC_INPUT_ACTION_LONG))
    {
        return false;
    }

    if ((FM_MAIN_INPUT_RECOGNIZER_HandleKeyboard(
             &recognizer,
             FM_BOARD_KEYBOARD_KEY_UP,
             FM_BOARD_KEYBOARD_EDGE_FALLING,
             &output) != FM_STATUS_OK) ||
        (output.timer_action != FM_MAIN_INPUT_RECOGNIZER_TIMER_CANCEL) ||
        output.runtime_event_valid)
    {
        return false;
    }

    if ((FM_MAIN_INPUT_RECOGNIZER_HandleHoldTimeout(
             &recognizer,
             &output) != FM_STATUS_ESTATE) ||
        output.runtime_event_valid)
    {
        return false;
    }

    if ((FM_MAIN_INPUT_RECOGNIZER_HandleKeyboard(
             &recognizer,
             FM_BOARD_KEYBOARD_KEY_DOWN,
             FM_BOARD_KEYBOARD_EDGE_RISING,
             &output) != FM_STATUS_OK) ||
        (FM_MAIN_INPUT_RECOGNIZER_HandleKeyboard(
             &recognizer,
             FM_BOARD_KEYBOARD_KEY_ENTER,
             FM_BOARD_KEYBOARD_EDGE_RISING,
             &output) != FM_STATUS_ESTATE) ||
        (output.timer_action != FM_MAIN_INPUT_RECOGNIZER_TIMER_START) ||
        output.runtime_event_valid)
    {
        return false;
    }

    return (FM_MAIN_INPUT_RECOGNIZER_HandleHoldTimeout(
                &recognizer,
                &output) == FM_STATUS_OK) &&
           output.runtime_event_valid &&
           (output.runtime_event.kind == FMC_RUNTIME_EVENT_INPUT) &&
           (output.runtime_event.data.input.key == FMC_INPUT_KEY_ENTER) &&
           (output.runtime_event.data.input.action == FMC_INPUT_ACTION_LONG);
}

/*
 * Verifies the app-level owner-loop event payload used by product/main.
 *
 * Periodic refresh stays in the app composition layer, coexists with keyboard
 * delivery in the same queue, and reports queue-full publication explicitly.
 */
static bool fm_regression_test_main_event_queue_(void)
{
    TX_QUEUE queue;
    ULONG storage[2U * FM_MAIN_EVENT_QUEUE_MESSAGE_WORDS];
    fm_main_event_t event = {0U};
    fm_main_event_t received = {0U};
    bool passed = false;

    if ((FM_MAIN_EVENT_Publish(NULL, &event) != FM_STATUS_EINVAL) ||
        (FM_MAIN_EVENT_Publish(&queue, NULL) != FM_STATUS_EINVAL))
    {
        return false;
    }

    if (tx_queue_create(&queue,
                        (CHAR *) "REG_MAIN_EVENT",
                        FM_MAIN_EVENT_QUEUE_MESSAGE_WORDS,
                        storage,
                        sizeof(storage)) != TX_SUCCESS)
    {
        return false;
    }

    FM_MAIN_EVENT_MakePeriodicRefresh(&event);
    if ((FM_MAIN_EVENT_Publish(&queue, &event) != FM_STATUS_OK) ||
        (tx_queue_receive(&queue, &received, TX_NO_WAIT) != TX_SUCCESS) ||
        ((fm_main_event_kind_t) received.kind !=
         FM_MAIN_EVENT_PERIODIC_REFRESH))
    {
        goto done;
    }

    FM_MAIN_EVENT_MakeKeyboard(&event,
                               FM_BOARD_KEYBOARD_KEY_DOWN,
                               FM_BOARD_KEYBOARD_EDGE_FALLING);
    if (FM_MAIN_EVENT_Publish(&queue, &event) != FM_STATUS_OK)
    {
        goto done;
    }

    FM_MAIN_EVENT_MakePeriodicRefresh(&event);
    if (FM_MAIN_EVENT_Publish(&queue, &event) != FM_STATUS_OK)
    {
        goto done;
    }

    if ((tx_queue_receive(&queue, &received, TX_NO_WAIT) != TX_SUCCESS) ||
        ((fm_main_event_kind_t) received.kind != FM_MAIN_EVENT_KEYBOARD) ||
        ((fm_board_keyboard_key_t) received.key !=
         FM_BOARD_KEYBOARD_KEY_DOWN) ||
        ((fm_board_keyboard_edge_t) received.edge !=
         FM_BOARD_KEYBOARD_EDGE_FALLING))
    {
        goto done;
    }

    if ((tx_queue_receive(&queue, &received, TX_NO_WAIT) != TX_SUCCESS) ||
        ((fm_main_event_kind_t) received.kind !=
         FM_MAIN_EVENT_PERIODIC_REFRESH))
    {
        goto done;
    }

    FM_MAIN_EVENT_MakeKeyHoldTimeout(&event);
    if ((FM_MAIN_EVENT_Publish(&queue, &event) != FM_STATUS_OK) ||
        (tx_queue_receive(&queue, &received, TX_NO_WAIT) != TX_SUCCESS) ||
        ((fm_main_event_kind_t) received.kind !=
         FM_MAIN_EVENT_KEY_HOLD_TIMEOUT))
    {
        goto done;
    }

    FM_MAIN_EVENT_MakePresentationTimeout(&event);
    if ((FM_MAIN_EVENT_Publish(&queue, &event) != FM_STATUS_OK) ||
        (tx_queue_receive(&queue, &received, TX_NO_WAIT) != TX_SUCCESS) ||
        ((fm_main_event_kind_t) received.kind !=
         FM_MAIN_EVENT_PRESENTATION_TIMEOUT))
    {
        goto done;
    }

    FM_MAIN_EVENT_MakeKeyboard(&event,
                               FM_BOARD_KEYBOARD_KEY_UP,
                               FM_BOARD_KEYBOARD_EDGE_FALLING);
    if ((FM_MAIN_EVENT_Publish(&queue, &event) != FM_STATUS_OK) ||
        (FM_MAIN_EVENT_Publish(&queue, &event) != FM_STATUS_OK) ||
        (FM_MAIN_EVENT_Publish(&queue, &event) != FM_STATUS_ERANGE))
    {
        goto done;
    }

    passed = true;

done:
    (void) tx_queue_delete(&queue);
    return passed;
}

/*
 * Apply one already produced observation result and verify the common runtime
 * success contract used by every stage of the 7F integration sequence.
 */
static bool fm_regression_test_dispatch_frequency_success_(
    fmc_runtime_t *p_runtime,
    const frequency_observation_result_t *p_result,
    frequency_observation_quality_t p_expected_quality,
    bool p_expected_value_present,
    double p_expected_value)
{
    fmc_runtime_rate_state_t rate_state;
    fmc_runtime_event_t event;

    event.kind = FMC_RUNTIME_EVENT_FREQUENCY_RESULT;
    event.data.frequency_result = *p_result;

    if ((FMC_RUNTIME_Dispatch(p_runtime, &event) != FM_STATUS_OK) ||
        !FMC_RUNTIME_PresentationUpdateIsPending(p_runtime) ||
        (FMC_RUNTIME_GetRateState(p_runtime, &rate_state) != FM_STATUS_OK) ||
        (rate_state.quality != p_expected_quality) ||
        (rate_state.value_present != p_expected_value_present) ||
        !fm_regression_test_double_eq_(rate_state.value, p_expected_value) ||
        (FMC_RUNTIME_ClearPresentationUpdatePending(p_runtime) !=
         FM_STATUS_OK) ||
        FMC_RUNTIME_PresentationUpdateIsPending(p_runtime))
    {
        return false;
    }

    return true;
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

    case FM_REGRESSION_TEST_CASE_PULSE_DELTA_VECTORS:
        return fm_regression_test_pulse_delta_vectors_();

    case FM_REGRESSION_TEST_CASE_FREQUENCY_OBSERVATION_VECTORS:
        return fm_regression_test_frequency_observation_vectors_();

    case FM_REGRESSION_TEST_CASE_OBSERVER_INDEPENDENCE:
        return fm_regression_test_observer_independence_();

    case FM_REGRESSION_TEST_CASE_RUNTIME_FREQUENCY_RESULT:
        return fm_regression_test_runtime_frequency_result_();

    case FM_REGRESSION_TEST_CASE_MAIN_ACQUISITION:
        return fm_regression_test_main_acquisition_();

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

    case FM_REGRESSION_TEST_CASE_LCD_MAP_ALL_SEGMENTS:
        return fm_regression_test_lcd_map_all_segments_();

    case FM_REGRESSION_TEST_CASE_LCD_MAP_LITERS_LEGEND:
        return fm_regression_test_lcd_map_liters_legend_();

    case FM_REGRESSION_TEST_CASE_PRESENTATION_SEQUENCE:
        return fm_regression_test_presentation_sequence_();

    case FM_REGRESSION_TEST_CASE_PRESENTATION_VALUES:
        return fm_regression_test_presentation_values_();

    case FM_REGRESSION_TEST_CASE_PRESENTATION_FAILURE:
        return fm_regression_test_presentation_failure_();

    case FM_REGRESSION_TEST_CASE_KEYBOARD_MAPPING:
        return fm_regression_test_keyboard_mapping_();

    case FM_REGRESSION_TEST_CASE_MAIN_INPUT_ADAPTER:
        return fm_regression_test_main_input_adapter_();

    case FM_REGRESSION_TEST_CASE_MAIN_INPUT_RECOGNIZER:
        return fm_regression_test_main_input_recognizer_();

    case FM_REGRESSION_TEST_CASE_MAIN_EVENT_QUEUE:
        return fm_regression_test_main_event_queue_();

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

    case FM_REGRESSION_TEST_CASE_PULSE_DELTA_VECTORS:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:PULSE_DELTA_VECTORS:");
        break;

    case FM_REGRESSION_TEST_CASE_FREQUENCY_OBSERVATION_VECTORS:
        (void) FM_DEBUG_UartStr(
            "REGRESSION_TEST:FREQUENCY_OBSERVATION_VECTORS:");
        break;

    case FM_REGRESSION_TEST_CASE_OBSERVER_INDEPENDENCE:
        (void) FM_DEBUG_UartStr(
            "REGRESSION_TEST:OBSERVER_INDEPENDENCE:");
        break;

    case FM_REGRESSION_TEST_CASE_RUNTIME_FREQUENCY_RESULT:
        (void) FM_DEBUG_UartStr(
            "REGRESSION_TEST:RUNTIME_FREQUENCY_RESULT:");
        break;

    case FM_REGRESSION_TEST_CASE_MAIN_ACQUISITION:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:MAIN_ACQUISITION:");
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

    case FM_REGRESSION_TEST_CASE_LCD_MAP_ALL_SEGMENTS:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:LCD_MAP_ALL_SEGMENTS:");
        break;

    case FM_REGRESSION_TEST_CASE_LCD_MAP_LITERS_LEGEND:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:LCD_MAP_LITERS_LEGEND:");
        break;

    case FM_REGRESSION_TEST_CASE_PRESENTATION_SEQUENCE:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:PRESENTATION_SEQUENCE:");
        break;

    case FM_REGRESSION_TEST_CASE_PRESENTATION_VALUES:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:PRESENTATION_VALUES:");
        break;

    case FM_REGRESSION_TEST_CASE_PRESENTATION_FAILURE:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:PRESENTATION_FAILURE:");
        break;

    case FM_REGRESSION_TEST_CASE_KEYBOARD_MAPPING:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:KEYBOARD_MAPPING:");
        break;

    case FM_REGRESSION_TEST_CASE_MAIN_INPUT_ADAPTER:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:MAIN_INPUT_ADAPTER:");
        break;

    case FM_REGRESSION_TEST_CASE_MAIN_INPUT_RECOGNIZER:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:MAIN_INPUT_RECOGNIZER:");
        break;

    case FM_REGRESSION_TEST_CASE_MAIN_EVENT_QUEUE:
        (void) FM_DEBUG_UartStr("REGRESSION_TEST:MAIN_EVENT_QUEUE:");
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
