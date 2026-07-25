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
}

fm_status_t FM_MAIN_ACQUISITION_ProcessObservation(
    fm_main_acquisition_t *p_acquisition,
    uint16_t current_count,
    fmc_runtime_t *p_runtime)
{
    fmc_runtime_event_t event;
    fm_status_t status;

    if ((p_acquisition == NULL) || (p_runtime == NULL))
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

    return FMC_RUNTIME_Dispatch(p_runtime, &event);
}
