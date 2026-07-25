/**
 * @file    pulse_delta.c
 * @brief   Stateful modulo-16-bit pulse-delta formation implementation.
 */
#include "pulse_delta.h"

#include <stddef.h>

void PULSE_DELTA_Init(pulse_delta_observer_t *p_observer)
{
    if (p_observer == NULL)
    {
        return;
    }

    p_observer->previous_count = 0U;
}

void PULSE_DELTA_Reset(pulse_delta_observer_t *p_observer)
{
    PULSE_DELTA_Init(p_observer);
}

fm_status_t PULSE_DELTA_Observe(pulse_delta_observer_t *p_observer,
                                uint16_t current_count,
                                uint64_t *p_pulse_delta)
{
    uint16_t delta;

    if ((p_observer == NULL) || (p_pulse_delta == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    delta = (uint16_t) (current_count - p_observer->previous_count);
    p_observer->previous_count = current_count;
    *p_pulse_delta = (uint64_t) delta;

    return FM_STATUS_OK;
}
