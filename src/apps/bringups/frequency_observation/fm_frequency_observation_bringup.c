/**
 * @file    fm_frequency_observation_bringup.c
 * @brief   Human-observed LPTIM4/LPTIM3 frequency characterization.
 */

#include "fm_frequency_observation_bringup.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "fm_board.h"
#include "fm_debug.h"
#include "fm_port_frequency_time.h"
#include "fm_port_pulse_counter.h"
#include "fm_port_time.h"
#include "frequency_observation.h"

#define FM_FREQUENCY_OBSERVATION_BRINGUP_EARLY_MS      (500U)
#define FM_FREQUENCY_OBSERVATION_BRINGUP_NOMINAL_MS    (1000U)
#define FM_FREQUENCY_OBSERVATION_BRINGUP_LATE_MS       (1200U)
#define FM_FREQUENCY_OBSERVATION_BRINGUP_SYNC_POLL_MS  (10U)
#define FM_FREQUENCY_OBSERVATION_BRINGUP_SYNC_LIMIT_MS (1000U)

/* Private variables */
static frequency_observation_t observer;
static uint32_t sample_index;

/* Private function declarations */
static void fm_frequency_observation_bringup_emit_instructions_(void);
static void fm_frequency_observation_bringup_wait_for_counter_sync_(void);
static void fm_frequency_observation_bringup_observe_(const char *p_label);
static const char *fm_frequency_observation_bringup_quality_(
    frequency_observation_quality_t quality);
static void fm_frequency_observation_bringup_require_ok_(
    fm_status_t status,
    const char *p_msg);

/* Public function definitions */
void FM_FrequencyObservationBringup_Run(void)
{
    FM_BOARD_Init();
    FM_DEBUG_Init();

    FM_DEBUG_UartStr("FO:START\n");

    fm_frequency_observation_bringup_require_ok_(
        FM_PORT_FrequencyTimeStart(),
        "FO:L3_START_FAIL");

    if (!FM_PORT_PulseCounterStart())
    {
        FM_DEBUG_PanicMsg("FO:L4_START_FAIL");
    }

    fm_frequency_observation_bringup_wait_for_counter_sync_();

    FREQUENCY_OBSERVATION_Init(&observer);
    sample_index = 0U;

    fm_frequency_observation_bringup_emit_instructions_();

    fm_frequency_observation_bringup_observe_("B");

    FM_PORT_TIME_SleepMs(FM_FREQUENCY_OBSERVATION_BRINGUP_EARLY_MS);
    fm_frequency_observation_bringup_observe_("E500");

    FM_PORT_TIME_SleepMs(FM_FREQUENCY_OBSERVATION_BRINGUP_EARLY_MS);
    fm_frequency_observation_bringup_observe_("V500X2");

    FM_PORT_TIME_SleepMs(FM_FREQUENCY_OBSERVATION_BRINGUP_NOMINAL_MS);
    fm_frequency_observation_bringup_observe_("V1A");

    FM_PORT_TIME_SleepMs(FM_FREQUENCY_OBSERVATION_BRINGUP_NOMINAL_MS);
    fm_frequency_observation_bringup_observe_("V1B");

    FM_PORT_TIME_SleepMs(FM_FREQUENCY_OBSERVATION_BRINGUP_LATE_MS);
    fm_frequency_observation_bringup_observe_("L1200");

    FM_PORT_TIME_SleepMs(FM_FREQUENCY_OBSERVATION_BRINGUP_NOMINAL_MS);
    fm_frequency_observation_bringup_observe_("REC");

    FM_DEBUG_UartStr("FO:RUN HZ=0,1,100,1000 HOLD_1K=70S\n");

    for (;;)
    {
        FM_PORT_TIME_SleepMs(FM_FREQUENCY_OBSERVATION_BRINGUP_NOMINAL_MS);
        fm_frequency_observation_bringup_observe_("RUN");
    }
}

/* Private function definitions */
static void fm_frequency_observation_bringup_emit_instructions_(void)
{
    FM_DEBUG_UartStr("FO:READY HZ=100 EDGE=FALL L3_HZ=2048 WRAP_S=32\n");
    FM_DEBUG_UartStr("FO:CHECK_1K DOC=fm_frequency_observation_bringup.h\n");
}

static void fm_frequency_observation_bringup_wait_for_counter_sync_(void)
{
    uint32_t waited_ms = 0U;

    /*
     * The asynchronous LPTIM4 path intentionally discards its first five
     * active edges after startup. A nonzero raw count proves that those edges
     * and the first effective counted edge have already occurred. Establishing
     * the frequency baseline earlier would include part of that one-time loss
     * inside the first physical window and invalidate its exact-pulse evidence.
     * The approved bring-up always starts at 100 Hz, so one second is a bounded
     * diagnostic timeout rather than a product activity policy.
     */
    while ((FM_PORT_PulseCounterReadStable() == 0U) &&
           (waited_ms < FM_FREQUENCY_OBSERVATION_BRINGUP_SYNC_LIMIT_MS))
    {
        FM_PORT_TIME_SleepMs(
            FM_FREQUENCY_OBSERVATION_BRINGUP_SYNC_POLL_MS);
        waited_ms += FM_FREQUENCY_OBSERVATION_BRINGUP_SYNC_POLL_MS;
    }

    if (FM_PORT_PulseCounterReadStable() == 0U)
    {
        FM_DEBUG_PanicMsg("FO:L4_SYNC_FAIL");
    }
}

static void fm_frequency_observation_bringup_observe_(const char *p_label)
{
    frequency_observation_sample_t sample;
    frequency_observation_result_t result;
    char message[96];
    bool result_available;
    uint32_t timestamp_seconds;
    uint32_t timestamp_microseconds;
    int length;

    FM_DEBUG_RefreshJumpers();

    sample.pulse_count = FM_PORT_PulseCounterReadStable();
    fm_frequency_observation_bringup_require_ok_(
        FM_PORT_FrequencyTimeRead(&sample.timestamp_us),
        "FO:L3_READ_FAIL");
    fm_frequency_observation_bringup_require_ok_(
        FREQUENCY_OBSERVATION_Observe(
            &observer,
            &sample,
            &result_available,
            &result),
        "FO:OBS_FAIL");

    sample_index++;

    if (!FM_DEBUG_MsgIsEnabled())
    {
        return;
    }

    /*
     * The canonical newlib-nano build omits C99 long-long formatted I/O.
     * Keep every snprintf conversion C89/32-bit and split the uint64_t
     * timestamp into seconds plus its six-digit microsecond remainder.
     */
    timestamp_seconds =
        (uint32_t) (sample.timestamp_us / 1000000ULL);
    timestamp_microseconds =
        (uint32_t) (sample.timestamp_us % 1000000ULL);

    if (!result_available)
    {
        length = snprintf(message,
                          sizeof(message),
                          "FO:S=%lu L=%s C=%u T=%lu.%06lu Q=-\n",
                          (unsigned long) sample_index,
                          p_label,
                          (unsigned int) sample.pulse_count,
                          (unsigned long) timestamp_seconds,
                          (unsigned long) timestamp_microseconds);
    }
    else if (result.quality == FREQUENCY_OBSERVATION_QUALITY_VALID)
    {
        length = snprintf(
            message,
            sizeof(message),
            "FO:S=%lu L=%s C=%u T=%lu.%06lu Q=V N=%lu E=%lu\n",
            (unsigned long) sample_index,
            p_label,
            (unsigned int) sample.pulse_count,
            (unsigned long) timestamp_seconds,
            (unsigned long) timestamp_microseconds,
            (unsigned long) result.pulse_delta,
            (unsigned long) result.elapsed_us);
    }
    else
    {
        length = snprintf(
            message,
            sizeof(message),
            "FO:S=%lu L=%s C=%u T=%lu.%06lu Q=%s\n",
            (unsigned long) sample_index,
            p_label,
            (unsigned int) sample.pulse_count,
            (unsigned long) timestamp_seconds,
            (unsigned long) timestamp_microseconds,
            fm_frequency_observation_bringup_quality_(result.quality));
    }

    if ((length > 0) && ((uint32_t) length < sizeof(message)))
    {
        FM_DEBUG_UartMsg(message, (uint32_t) length);
    }
    else
    {
        FM_DEBUG_UartStr("FO:FMT_ERR\n");
    }
}

static const char *fm_frequency_observation_bringup_quality_(
    frequency_observation_quality_t quality)
{
    switch (quality)
    {
    case FREQUENCY_OBSERVATION_QUALITY_VALID:
        return "V";

    case FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE:
        return "U";

    case FREQUENCY_OBSERVATION_QUALITY_STALE:
        return "S";

    case FREQUENCY_OBSERVATION_QUALITY_INVALID:
        return "I";

    default:
        return "?";
    }
}

static void fm_frequency_observation_bringup_require_ok_(
    fm_status_t status,
    const char *p_msg)
{
    if (status != FM_STATUS_OK)
    {
        FM_DEBUG_PanicMsg(p_msg);
    }
}
