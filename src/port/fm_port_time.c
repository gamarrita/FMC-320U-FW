/**
 * @file    fm_port_time.c
 * @brief   Platform millisecond sleep helpers.
 */

#include <fm_port_time.h>

#include <limits.h>
#include <stdbool.h>

#include "fm_debug.h"
#include "tx_api.h"

static bool fm_port_time_ms_to_ticks_(uint32_t time_ms, ULONG *p_ticks);

void FM_PORT_TIME_SleepMs(uint32_t time_ms)
{
    ULONG ticks;
    UINT status;

    if (time_ms == 0U)
    {
        return;
    }

    if (!fm_port_time_ms_to_ticks_(time_ms, &ticks))
    {
        FM_DEBUG_PanicMsg("PORT_TIME:SLEEP_TICK_OVERFLOW");
    }

    status = tx_thread_sleep(ticks);
    if (status != TX_SUCCESS)
    {
        FM_DEBUG_PanicMsg("PORT_TIME:SLEEP_CONTEXT_ERROR");
    }
}

static bool fm_port_time_ms_to_ticks_(uint32_t time_ms, ULONG *p_ticks)
{
    const ULONG ticks_per_second = (ULONG) TX_TIMER_TICKS_PER_SECOND;
    ULONG whole_seconds;
    ULONG remaining_ms;
    ULONG ticks;
    ULONG partial_product;
    ULONG partial_ticks;

    if ((p_ticks == NULL) || (ticks_per_second == 0U))
    {
        return false;
    }

    whole_seconds = ((ULONG) time_ms) / 1000U;
    remaining_ms = ((ULONG) time_ms) % 1000U;

    if (whole_seconds > (ULONG_MAX / ticks_per_second))
    {
        return false;
    }

    ticks = whole_seconds * ticks_per_second;

    if (remaining_ms != 0U)
    {
        if (remaining_ms > ((ULONG_MAX - 999U) / ticks_per_second))
        {
            return false;
        }

        partial_product = remaining_ms * ticks_per_second;
        partial_ticks = (partial_product + 999U) / 1000U;

        if (ticks > (ULONG_MAX - partial_ticks))
        {
            return false;
        }

        ticks += partial_ticks;
    }

    if (ticks == 0U)
    {
        ticks = 1U;
    }

    *p_ticks = ticks;

    return true;
}
