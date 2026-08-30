/**
 * @file    fm_main_acquisition.c
 * @brief   Product-main pulse-observation adapter implementation.
 */
#include "fm_main_acquisition.h"

#include <stddef.h>

void FM_MAIN_ACQUISITION_Init(fm_main_acquisition_t *p_acquisition)
{
    if (p_acquisition == NULL)
    {
        return;
    }

    PULSE_DELTA_Init(&p_acquisition->pulse_delta_observer);
    FREQUENCY_OBSERVATION_Init(&p_acquisition->frequency_observer);
}

fm_status_t FM_MAIN_ACQUISITION_ProcessObservation(
    fm_main_acquisition_t *p_acquisition,
    uint16_t current_count,
    fmc_runtime_t *p_runtime,
    uint64_t *p_pulse_delta)
{
    fmc_runtime_event_t event;
    fm_status_t status;

    if ((p_acquisition == NULL) || (p_runtime == NULL) ||
        (p_pulse_delta == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    status = PULSE_DELTA_Observe(&p_acquisition->pulse_delta_observer,
                                 current_count,
                                 &event.data.pulse_delta);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    event.kind = FMC_RUNTIME_EVENT_PULSE_DELTA;
    status = FMC_RUNTIME_Dispatch(p_runtime, &event);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    *p_pulse_delta = event.data.pulse_delta;

    return FM_STATUS_OK;
}

fm_status_t FM_MAIN_ACQUISITION_ProcessFrequencyObservation(
    fm_main_acquisition_t *p_acquisition,
    const frequency_observation_sample_t *p_sample,
    fmc_runtime_t *p_runtime)
{
    fmc_runtime_event_t event;
    bool result_available;
    fm_status_t status;

    if ((p_acquisition == NULL) || (p_sample == NULL) ||
        (p_runtime == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    status = FREQUENCY_OBSERVATION_Observe(
        &p_acquisition->frequency_observer,
        p_sample,
        &result_available,
        &event.data.frequency_result);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    if (!result_available)
    {
        return FM_STATUS_OK;
    }

    event.kind = FMC_RUNTIME_EVENT_FREQUENCY_RESULT;

    return FMC_RUNTIME_Dispatch(p_runtime, &event);
}
