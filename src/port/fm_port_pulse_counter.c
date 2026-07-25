/**
 * @file    fm_port_pulse_counter.c
 * @brief   STM32 LPTIM4 adaptation for the primary pulse counter.
 */

#include "fm_port_pulse_counter.h"

#include "lptim.h"

/* Private variables */
static bool started;

bool FM_PORT_PulseCounterStart(void)
{
    if (started)
    {
        return true;
    }

    /*
     * CubeMX does not expose the STM32U5 RCC autonomous-access bit for LPTIM4.
     * Set it once and leave it enabled so the peripheral configuration remains
     * unchanged across later Run/Stop transitions.
     */
    __HAL_RCC_LPTIM4_CLKAM_ENABLE();

    if (HAL_LPTIM_Counter_Start(&hlptim4) != HAL_OK)
    {
        return false;
    }

    /*
     * The asynchronous external-clock path loses its first five active edges
     * after this start. The instrument accepts that one-time startup loss and
     * does not stop or rearm the counter during normal operation.
     */
    started = true;

    return true;
}

uint16_t FM_PORT_PulseCounterReadStable(void)
{
    uint16_t first;
    uint16_t second;

    do
    {
        first = (uint16_t) HAL_LPTIM_ReadCounter(&hlptim4);
        second = (uint16_t) HAL_LPTIM_ReadCounter(&hlptim4);
    }
    while (first != second);

    return second;
}
