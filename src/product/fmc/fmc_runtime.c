/**
 * @file    fmc_runtime.c
 * @brief   RTOS-neutral FMC runtime event boundary implementation.
 */
#include "fmc_runtime.h"

#include <stdbool.h>
#include <stddef.h>

#include "fmc_rate.h"

/* Private macros */
#define FMC_RUNTIME_MICROSECONDS_PER_SECOND   (1000000.0)

/* Private function declarations */
static fm_status_t fmc_runtime_apply_frequency_result_(
    fmc_runtime_t *p_runtime,
    const frequency_observation_result_t *p_result);
static fm_status_t fmc_runtime_reset_total_(fmc_runtime_t *p_runtime,
                                            fmc_model_total_t p_total);

void FMC_RUNTIME_Init(fmc_runtime_t *p_runtime)
{
    if (p_runtime == NULL)
    {
        return;
    }

    FMC_SERVICE_Init(&p_runtime->service);
    p_runtime->rate.value = 0.0;
    p_runtime->rate.quality = FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE;
    p_runtime->rate.value_present = false;
    p_runtime->presentation_update_pending = false;
}

fm_status_t FMC_RUNTIME_Dispatch(fmc_runtime_t *p_runtime,
                                 const fmc_runtime_event_t *p_event)
{
    fm_status_t status;

    if ((p_runtime == NULL) || (p_event == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    switch (p_event->kind)
    {
    case FMC_RUNTIME_EVENT_NONE:
        return FM_STATUS_OK;

    case FMC_RUNTIME_EVENT_PULSE_DELTA:
        if (p_event->data.pulse_delta == 0U)
        {
            return FM_STATUS_OK;
        }

        status = FMC_SERVICE_AddPulseDelta(&p_runtime->service,
                                           p_event->data.pulse_delta);
        if (status == FM_STATUS_OK)
        {
            p_runtime->presentation_update_pending = true;
        }
        return status;

    case FMC_RUNTIME_EVENT_FREQUENCY_RESULT:
        status = fmc_runtime_apply_frequency_result_(
            p_runtime,
            &p_event->data.frequency_result);
        if (status == FM_STATUS_OK)
        {
            p_runtime->presentation_update_pending = true;
        }
        return status;

    case FMC_RUNTIME_EVENT_RESET_ACM:
        return fmc_runtime_reset_total_(p_runtime, FMC_MODEL_TOTAL_ACM);

    case FMC_RUNTIME_EVENT_RESET_TTL:
        return fmc_runtime_reset_total_(p_runtime, FMC_MODEL_TOTAL_TTL);

    case FMC_RUNTIME_EVENT_PRESENTATION_INVALIDATE:
        p_runtime->presentation_update_pending = true;
        return FM_STATUS_OK;

    default:
        return FM_STATUS_EINVAL;
    }
}

fm_status_t FMC_RUNTIME_GetRateState(
    const fmc_runtime_t *p_runtime,
    fmc_runtime_rate_state_t *p_rate_state)
{
    if ((p_runtime == NULL) || (p_rate_state == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    *p_rate_state = p_runtime->rate;

    return FM_STATUS_OK;
}

fm_status_t FMC_RUNTIME_GetSnapshot(const fmc_runtime_t *p_runtime,
                                    fmc_service_snapshot_t *p_snapshot)
{
    if (p_runtime == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    return FMC_SERVICE_GetSnapshot(&p_runtime->service, p_snapshot);
}

bool FMC_RUNTIME_PresentationUpdateIsPending(
    const fmc_runtime_t *p_runtime)
{
    if (p_runtime == NULL)
    {
        return false;
    }

    return p_runtime->presentation_update_pending;
}

fm_status_t FMC_RUNTIME_ClearPresentationUpdatePending(
    fmc_runtime_t *p_runtime)
{
    if (p_runtime == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    p_runtime->presentation_update_pending = false;

    return FM_STATUS_OK;
}

static fm_status_t fmc_runtime_apply_frequency_result_(
    fmc_runtime_t *p_runtime,
    const frequency_observation_result_t *p_result)
{
    double elapsed_seconds;
    double rate;
    fm_status_t status;

    if ((p_runtime == NULL) || (p_result == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    switch (p_result->quality)
    {
    case FREQUENCY_OBSERVATION_QUALITY_VALID:
        elapsed_seconds =
            (double) p_result->elapsed_us /
            FMC_RUNTIME_MICROSECONDS_PER_SECOND;
        status = FMC_RATE_Calc(&p_runtime->service.model.measurement,
                               p_result->pulse_delta,
                               elapsed_seconds,
                               &rate);
        if (status != FM_STATUS_OK)
        {
            return status;
        }

        p_runtime->rate.value = rate;
        p_runtime->rate.quality = FREQUENCY_OBSERVATION_QUALITY_VALID;
        p_runtime->rate.value_present = true;
        return FM_STATUS_OK;

    case FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE:
        p_runtime->rate.value = 0.0;
        p_runtime->rate.quality =
            FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE;
        p_runtime->rate.value_present = false;
        return FM_STATUS_OK;

    case FREQUENCY_OBSERVATION_QUALITY_STALE:
    case FREQUENCY_OBSERVATION_QUALITY_INVALID:
        p_runtime->rate.quality = p_result->quality;
        return FM_STATUS_OK;

    default:
        return FM_STATUS_EINVAL;
    }
}

static fm_status_t fmc_runtime_reset_total_(fmc_runtime_t *p_runtime,
                                            fmc_model_total_t p_total)
{
    fm_status_t status;

    status = FMC_SERVICE_ResetTotal(&p_runtime->service, p_total);
    if (status == FM_STATUS_OK)
    {
        p_runtime->presentation_update_pending = true;
    }

    return status;
}
