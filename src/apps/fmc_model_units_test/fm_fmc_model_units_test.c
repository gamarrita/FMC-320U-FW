/**
 * @file    fm_fmc_model_units_test.c
 * @brief   Repeatable verification app for pure FMC model and unit slices.
 */
#include "fm_fmc_model_units_test.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "fm_board.h"
#include "fm_debug.h"
#include "fm_port_time.h"
#include "fmc_model.h"
#include "fmc_rate.h"
#include "fmc_units.h"

#define FM_FMC_MODEL_UNITS_TEST_IDLE_MS   1000U
#define FM_FMC_MODEL_UNITS_TEST_EPSILON   0.000001

typedef enum
{
    FM_FMC_MODEL_UNITS_TEST_CASE_INIT_DEFAULTS = 0,
    FM_FMC_MODEL_UNITS_TEST_CASE_TOTALS,
    FM_FMC_MODEL_UNITS_TEST_CASE_UNIT_VALIDITY,
    FM_FMC_MODEL_UNITS_TEST_CASE_UNIT_KIND,
    FM_FMC_MODEL_UNITS_TEST_CASE_LITERS_PER_UNIT,
    FM_FMC_MODEL_UNITS_TEST_CASE_PULSES_PER_ACTIVE_UNIT,
    FM_FMC_MODEL_UNITS_TEST_CASE_ERROR_PATHS,
    FM_FMC_MODEL_UNITS_TEST_CASE_RATE_WINDOWS,
    FM_FMC_MODEL_UNITS_TEST_CASE_RATE_ERROR_PATHS,
    FM_FMC_MODEL_UNITS_TEST_CASE_COUNT
} fm_fmc_model_units_test_case_t;

typedef struct
{
    fmc_model_volume_unit_t unit;
    double expected_liters_per_unit;
} fm_fmc_model_units_liters_case_t;

/* Private function declarations */
static bool fm_fmc_model_units_test_double_eq_(double p_actual,
                                               double p_expected);
static bool fm_fmc_model_units_test_init_defaults_(void);
static bool fm_fmc_model_units_test_totals_(void);
static bool fm_fmc_model_units_test_unit_validity_(void);
static bool fm_fmc_model_units_test_unit_kind_(void);
static bool fm_fmc_model_units_test_liters_per_unit_(void);
static bool fm_fmc_model_units_test_pulses_per_active_unit_(void);
static bool fm_fmc_model_units_test_error_paths_(void);
static bool fm_fmc_model_units_test_rate_windows_(void);
static bool fm_fmc_model_units_test_rate_error_paths_(void);
static bool fm_fmc_model_units_test_run_case_(fm_fmc_model_units_test_case_t p_case);
static void fm_fmc_model_units_test_emit_case_(fm_fmc_model_units_test_case_t p_case,
                                               bool p_passed);

/* Public function definitions */
void FM_FmcModelUnitsTest_Run(void)
{
    fm_fmc_model_units_test_case_t test_case;
    bool passed = true;

    FM_BOARD_Init();
    FM_DEBUG_Init();

    (void) FM_DEBUG_UartStr("FMC_MODEL_UNITS_TEST:START\n");

    for (test_case = FM_FMC_MODEL_UNITS_TEST_CASE_INIT_DEFAULTS;
         test_case < FM_FMC_MODEL_UNITS_TEST_CASE_COUNT;
         test_case = (fm_fmc_model_units_test_case_t) (test_case + 1U))
    {
        if (!fm_fmc_model_units_test_run_case_(test_case))
        {
            passed = false;
            fm_fmc_model_units_test_emit_case_(test_case, false);
            FM_DEBUG_Flush();
            FM_DEBUG_PanicMsg("FMC_MODEL_UNITS_TEST:FAIL\n");
        }

        fm_fmc_model_units_test_emit_case_(test_case, true);
        FM_DEBUG_Flush();
    }

    if (passed)
    {
        (void) FM_DEBUG_UartStr("FMC_MODEL_UNITS_TEST:PASS\n");
    }

    for (;;)
    {
        FM_DEBUG_LedRun(FM_DEBUG_LED_ON);
        FM_PORT_TIME_SleepMs(FM_FMC_MODEL_UNITS_TEST_IDLE_MS);
        FM_DEBUG_LedRun(FM_DEBUG_LED_OFF);
        FM_PORT_TIME_SleepMs(FM_FMC_MODEL_UNITS_TEST_IDLE_MS);
    }
}

/* Private function definitions */
static bool fm_fmc_model_units_test_double_eq_(double p_actual,
                                               double p_expected)
{
    double diff = p_actual - p_expected;

    if (diff < 0.0)
    {
        diff = -diff;
    }

    return diff <= FM_FMC_MODEL_UNITS_TEST_EPSILON;
}

static bool fm_fmc_model_units_test_init_defaults_(void)
{
    fmc_model_t model;

    FMC_MODEL_Init(&model);

    return fm_fmc_model_units_test_double_eq_(
               model.measurement.calibration_pulses_per_unit,
               FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_DEFAULT) &&
           (model.measurement.calibration_volume_unit ==
            FMC_MODEL_CALIBRATION_UNIT_DEFAULT) &&
           (model.measurement.active_volume_unit == FMC_MODEL_VOLUME_UNIT_L) &&
           (model.measurement.active_time_base == FMC_MODEL_TIME_BASE_SECOND) &&
           (model.acm.pulses == 0U) &&
           (model.ttl.pulses == 0U);
}

static bool fm_fmc_model_units_test_totals_(void)
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

static bool fm_fmc_model_units_test_unit_validity_(void)
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

static bool fm_fmc_model_units_test_unit_kind_(void)
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

static bool fm_fmc_model_units_test_liters_per_unit_(void)
{
    static const fm_fmc_model_units_liters_case_t cases[] =
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

        if (!fm_fmc_model_units_test_double_eq_(
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

static bool fm_fmc_model_units_test_pulses_per_active_unit_(void)
{
    static const fm_fmc_model_units_liters_case_t cases[] =
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

        if (!fm_fmc_model_units_test_double_eq_(
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
           fm_fmc_model_units_test_double_eq_(pulses_per_active_unit, 2.5);
}

static bool fm_fmc_model_units_test_error_paths_(void)
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

static bool fm_fmc_model_units_test_rate_windows_(void)
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
            !fm_fmc_model_units_test_double_eq_(rate,
                                                expected_rates[index]))
        {
            return false;
        }
    }

    measurement.active_time_base = FMC_MODEL_TIME_BASE_MINUTE;
    measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_M3;

    if ((FMC_RATE_Calc(&measurement, 2000U, 10.0, &rate) != FM_STATUS_OK) ||
        !fm_fmc_model_units_test_double_eq_(rate, 6.0))
    {
        return false;
    }

    measurement.calibration_pulses_per_unit = 6000.0;
    measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_L;

    if ((FMC_RATE_Calc(&measurement, 60U, 1.0, &rate) != FM_STATUS_OK) ||
        !fm_fmc_model_units_test_double_eq_(rate, 0.6))
    {
        return false;
    }

    return (FMC_RATE_Calc(&measurement, 0U, 10.0, &rate) == FM_STATUS_OK) &&
           fm_fmc_model_units_test_double_eq_(rate, 0.0);
}

static bool fm_fmc_model_units_test_rate_error_paths_(void)
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

static bool fm_fmc_model_units_test_run_case_(fm_fmc_model_units_test_case_t p_case)
{
    switch (p_case)
    {
    case FM_FMC_MODEL_UNITS_TEST_CASE_INIT_DEFAULTS:
        return fm_fmc_model_units_test_init_defaults_();

    case FM_FMC_MODEL_UNITS_TEST_CASE_TOTALS:
        return fm_fmc_model_units_test_totals_();

    case FM_FMC_MODEL_UNITS_TEST_CASE_UNIT_VALIDITY:
        return fm_fmc_model_units_test_unit_validity_();

    case FM_FMC_MODEL_UNITS_TEST_CASE_UNIT_KIND:
        return fm_fmc_model_units_test_unit_kind_();

    case FM_FMC_MODEL_UNITS_TEST_CASE_LITERS_PER_UNIT:
        return fm_fmc_model_units_test_liters_per_unit_();

    case FM_FMC_MODEL_UNITS_TEST_CASE_PULSES_PER_ACTIVE_UNIT:
        return fm_fmc_model_units_test_pulses_per_active_unit_();

    case FM_FMC_MODEL_UNITS_TEST_CASE_ERROR_PATHS:
        return fm_fmc_model_units_test_error_paths_();

    case FM_FMC_MODEL_UNITS_TEST_CASE_RATE_WINDOWS:
        return fm_fmc_model_units_test_rate_windows_();

    case FM_FMC_MODEL_UNITS_TEST_CASE_RATE_ERROR_PATHS:
        return fm_fmc_model_units_test_rate_error_paths_();

    default:
        return false;
    }
}

static void fm_fmc_model_units_test_emit_case_(fm_fmc_model_units_test_case_t p_case,
                                               bool p_passed)
{
    switch (p_case)
    {
    case FM_FMC_MODEL_UNITS_TEST_CASE_INIT_DEFAULTS:
        (void) FM_DEBUG_UartStr("FMC_MODEL_UNITS_TEST:INIT_DEFAULTS:");
        break;

    case FM_FMC_MODEL_UNITS_TEST_CASE_TOTALS:
        (void) FM_DEBUG_UartStr("FMC_MODEL_UNITS_TEST:TOTALS:");
        break;

    case FM_FMC_MODEL_UNITS_TEST_CASE_UNIT_VALIDITY:
        (void) FM_DEBUG_UartStr("FMC_MODEL_UNITS_TEST:UNIT_VALIDITY:");
        break;

    case FM_FMC_MODEL_UNITS_TEST_CASE_UNIT_KIND:
        (void) FM_DEBUG_UartStr("FMC_MODEL_UNITS_TEST:UNIT_KIND:");
        break;

    case FM_FMC_MODEL_UNITS_TEST_CASE_LITERS_PER_UNIT:
        (void) FM_DEBUG_UartStr("FMC_MODEL_UNITS_TEST:LITERS_PER_UNIT:");
        break;

    case FM_FMC_MODEL_UNITS_TEST_CASE_PULSES_PER_ACTIVE_UNIT:
        (void) FM_DEBUG_UartStr("FMC_MODEL_UNITS_TEST:PULSES_PER_ACTIVE_UNIT:");
        break;

    case FM_FMC_MODEL_UNITS_TEST_CASE_ERROR_PATHS:
        (void) FM_DEBUG_UartStr("FMC_MODEL_UNITS_TEST:ERROR_PATHS:");
        break;

    case FM_FMC_MODEL_UNITS_TEST_CASE_RATE_WINDOWS:
        (void) FM_DEBUG_UartStr("FMC_MODEL_UNITS_TEST:RATE_WINDOWS:");
        break;

    case FM_FMC_MODEL_UNITS_TEST_CASE_RATE_ERROR_PATHS:
        (void) FM_DEBUG_UartStr("FMC_MODEL_UNITS_TEST:RATE_ERROR_PATHS:");
        break;

    default:
        (void) FM_DEBUG_UartStr("FMC_MODEL_UNITS_TEST:UNKNOWN:");
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
