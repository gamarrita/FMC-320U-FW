/**
 * @file fm_factory.c
 * @brief Factory presets and firmware identification helpers.
 */

#include "fm_factory.h"

// --- Factory constants ---

static const char k_firmware_version[] = "01.01.011"; // Major.Minor.Patch
static const char k_release_tag[]      = "b1";

// Generic configuration for sensors without prior calibration.
static const fm_fmc_totalizer_t k_sensor_0 = {
    .pulse_acm        = 0,
    .pulse_ttl        = 0,
    .vol_pf_sel       = 0,
    .factor_k         = 1.000,
    .factor_cal       = 1000,
    .vol_unit         = VOL_UNIT_LT,
    .time_unit        = TIME_UNIT_SECOND,
    .rate.factor_r    = 1000,
    .rate.rate_pf_sel = 0,
    .rate.limit_high  = 1500000, // 1500 Hz
    .rate.limit_low   = 250,     // 0.1 Hz
    .rate.delta_t     = 1000,
    .rate.filter      = 1,
    .ticket_number    = 0,
};

// Reference values for an axial AI DN25 sensor.
static const fm_fmc_totalizer_t k_sensor_ai_25 = {
    .pulse_acm        = 0,
    .pulse_ttl        = 0,
    .vol_pf_sel       = 3,
    .factor_k         = 123.456f,
    .factor_cal       = 123456,
    .vol_unit         = VOL_UNIT_LT,
    .time_unit        = TIME_UNIT_SECOND,
    .rate.factor_r    = 1000,
    .rate.rate_pf_sel = 3,
    .rate.limit_high  = 1500000,
    .rate.limit_low   = 250,
    .rate.delta_t     = 1000,
    .rate.filter      = 1,
    .ticket_number    = 0,
};

// Reference values for an axial AI DN80 sensor.
static const fm_fmc_totalizer_t k_sensor_ai_80 = {
    .pulse_acm        = 0,
    .pulse_ttl        = 0,
    .vol_pf_sel       = 3,
    .factor_k         = 23408.0f,
    .factor_cal       = 1000,
    .vol_unit         = VOL_UNIT_M3,
    .time_unit        = TIME_UNIT_SECOND,
    .rate.factor_r    = 1000,
    .rate.rate_pf_sel = 3,
    .rate.limit_high  = 1500000,
    .rate.limit_low   = 250,
    .rate.delta_t     = 1000,
    .rate.filter      = 1,
    .ticket_number    = 0,
};

// --- API ---

/**
 * Returns the firmware version string (major.minor.patch).
 */
const char *FM_FACTORY_FirmwareVersionGet(void)
{
    return k_firmware_version;
}

/**
 * Returns the release tag used to identify official builds.
 */
const char *FM_FACTORY_ReleaseGet(void)
{
    return k_release_tag;
}

/**
 * Retrieves a factory configuration for the requested totalizer.
 * @param sel Sensor identifier or data source to load.
 * @return Copy of the requested base configuration.
 */
fm_fmc_totalizer_t FM_FACTORY_TotalizerGet(sensors_list_t sel)
{
    switch (sel) {
    case FM_FACTORY_RAM_BACKUP:
    case FM_FACTORY_LAST_SETUP:
        return FM_FMC_GetEnviroment();
    case FM_FACTORY_SENSOR_0:
        return k_sensor_0;
    case FM_FACTORY_AI_25:
        return k_sensor_ai_25;
    case FM_FACTORY_AI_80:
        return k_sensor_ai_80;
    default:
        return k_sensor_0;
    }
}

