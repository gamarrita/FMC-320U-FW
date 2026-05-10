/**
 * @file    fm_port_dwt.c
 * @brief   Platform DWT cycle counter helpers.
 */

#include <fm_port_dwt.h>

bool FM_PORT_DWT_Init(void)
{
#if FM_PORT_DWT_HAS_CYCCNT
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    return true;
#else
    return false; /* CYCCNT not present on this device */
#endif
}

uint32_t FM_PORT_DWT_GetCpuHz(void)
{
    return SystemCoreClock;
}

uint32_t FM_PORT_DWT_CyclesPerUs(void)
{
    return FM_PORT_DWT_GetCpuHz() / 1000000U; /* returns 0 when clock is unknown */
}

uint32_t FM_PORT_DWT_UsToCycles(uint32_t us)
{
    uint32_t cycles_per_us = FM_PORT_DWT_CyclesPerUs();
    if (cycles_per_us == 0U)
    {
        return 0U;
    }

    /* 64-bit math avoids overflow for larger intervals */
    uint64_t cycles = (uint64_t) cycles_per_us * (uint64_t) us;
    return (uint32_t)cycles;
}

int32_t FM_PORT_DWT_CyclesToUs(int32_t cycles)
{
    uint32_t cycles_per_us = FM_PORT_DWT_CyclesPerUs();
    if (cycles_per_us == 0U)
    {
        return 0;
    }

    return cycles / (int32_t)cycles_per_us;
}

void FM_PORT_DWT_DelayUs(uint32_t time_us)
{
#if FM_PORT_DWT_HAS_CYCCNT
    uint32_t delay_cycles = FM_PORT_DWT_UsToCycles(time_us);
    uint32_t start_cycles;

    if (delay_cycles == 0U)
    {
        return;
    }

    start_cycles = FM_PORT_DWT_GetCycles();

    while ((FM_PORT_DWT_GetCycles() - start_cycles) < delay_cycles)
    {
    }
#else
    (void)time_us;
#endif
}
