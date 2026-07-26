/**
 * @file    frequency_observation.c
 * @brief   Stateful frequency-window admission implementation.
 */

#include "frequency_observation.h"

#include <stddef.h>

/* Private function declarations */
static void frequency_observation_clear_state_(
    frequency_observation_t *p_observer);
static void frequency_observation_set_baseline_(
    frequency_observation_t *p_observer,
    const frequency_observation_sample_t *p_sample);
static void frequency_observation_set_nonvalid_result_(
    frequency_observation_result_t *p_result,
    frequency_observation_quality_t quality);

/* Public function definitions */
void FREQUENCY_OBSERVATION_Init(frequency_observation_t *p_observer)
{
    if (p_observer == NULL)
    {
        return;
    }

    frequency_observation_clear_state_(p_observer);
}

void FREQUENCY_OBSERVATION_Reset(frequency_observation_t *p_observer)
{
    FREQUENCY_OBSERVATION_Init(p_observer);
}

fm_status_t FREQUENCY_OBSERVATION_Observe(
    frequency_observation_t *p_observer,
    const frequency_observation_sample_t *p_sample,
    bool *p_result_available,
    frequency_observation_result_t *p_result)
{
    uint64_t elapsed_us;
    uint16_t pulse_delta;

    if ((p_observer == NULL) || (p_sample == NULL) ||
        (p_result_available == NULL) || (p_result == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    *p_result_available = true;

    if (!p_observer->baseline_available)
    {
        frequency_observation_set_baseline_(p_observer, p_sample);
        frequency_observation_set_nonvalid_result_(
            p_result,
            FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE);
        return FM_STATUS_OK;
    }

    if (p_sample->timestamp_us <= p_observer->previous_timestamp_us)
    {
        frequency_observation_clear_state_(p_observer);
        frequency_observation_set_nonvalid_result_(
            p_result,
            FREQUENCY_OBSERVATION_QUALITY_INVALID);
        return FM_STATUS_OK;
    }

    elapsed_us =
        p_sample->timestamp_us - p_observer->previous_timestamp_us;

    if (elapsed_us < FREQUENCY_OBSERVATION_MIN_ELAPSED_US)
    {
        *p_result_available = false;
        return FM_STATUS_OK;
    }

    if (elapsed_us > FREQUENCY_OBSERVATION_MAX_ELAPSED_US)
    {
        frequency_observation_quality_t quality =
            p_observer->valid_window_seen ?
            FREQUENCY_OBSERVATION_QUALITY_STALE :
            FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE;

        frequency_observation_set_baseline_(p_observer, p_sample);
        frequency_observation_set_nonvalid_result_(p_result, quality);
        return FM_STATUS_OK;
    }

    pulse_delta =
        (uint16_t) (p_sample->pulse_count - p_observer->previous_count);
    frequency_observation_set_baseline_(p_observer, p_sample);
    p_observer->valid_window_seen = true;

    p_result->quality = FREQUENCY_OBSERVATION_QUALITY_VALID;
    p_result->pulse_delta = (uint64_t) pulse_delta;
    p_result->elapsed_us = elapsed_us;

    return FM_STATUS_OK;
}

fm_status_t FREQUENCY_OBSERVATION_Invalidate(
    frequency_observation_t *p_observer,
    frequency_observation_result_t *p_result)
{
    if ((p_observer == NULL) || (p_result == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    frequency_observation_clear_state_(p_observer);
    frequency_observation_set_nonvalid_result_(
        p_result,
        FREQUENCY_OBSERVATION_QUALITY_INVALID);

    return FM_STATUS_OK;
}

/* Private function definitions */
static void frequency_observation_clear_state_(
    frequency_observation_t *p_observer)
{
    p_observer->previous_count = 0U;
    p_observer->previous_timestamp_us = 0U;
    p_observer->baseline_available = false;
    p_observer->valid_window_seen = false;
}

static void frequency_observation_set_baseline_(
    frequency_observation_t *p_observer,
    const frequency_observation_sample_t *p_sample)
{
    p_observer->previous_count = p_sample->pulse_count;
    p_observer->previous_timestamp_us = p_sample->timestamp_us;
    p_observer->baseline_available = true;
}

static void frequency_observation_set_nonvalid_result_(
    frequency_observation_result_t *p_result,
    frequency_observation_quality_t quality)
{
    p_result->quality = quality;
    p_result->pulse_delta = 0U;
    p_result->elapsed_us = 0U;
}
