/**
 * @file    fm_port_dwt.h
 * @brief   Platform DWT cycle counter helpers.
 */

#ifndef FM_PORT_DWT_H
#define FM_PORT_DWT_H

#include <stdbool.h>
#include <stdint.h>
#include "main.h"

#if defined(DWT) && defined(CoreDebug) &&     defined(DWT_CTRL_CYCCNTENA_Msk) && defined(CoreDebug_DEMCR_TRCENA_Msk)
#define FM_PORT_DWT_HAS_CYCCNT 1
#else
#define FM_PORT_DWT_HAS_CYCCNT 0
#endif

/**
 * @brief   Enable and reset the DWT cycle counter when supported.
 *
 * @return  true when CYCCNT is available and enabled, false otherwise.
 */
bool FM_PORT_DWT_Init(void);

/**
 * @brief   Get the current CPU clock frequency used for DWT conversions.
 *
 * @return  CPU frequency in hertz.
 */
uint32_t FM_PORT_DWT_GetCpuHz(void);

/**
 * @brief   Convert the current CPU clock to cycles per microsecond.
 *
 * @return  CPU cycles per microsecond.
 */
uint32_t FM_PORT_DWT_CyclesPerUs(void);

/**
 * @brief   Convert microseconds to CPU cycles using the current clock.
 *
 * @param   us  Interval in microseconds.
 *
 * @return  Equivalent cycle count, or 0 when the clock is unavailable.
 */
uint32_t FM_PORT_DWT_UsToCycles(uint32_t us);

/**
 * @brief   Convert CPU cycles to microseconds using the current clock.
 *
 * @param   cycles  Interval in CPU cycles.
 *
 * @return  Equivalent time in microseconds, or 0 when the clock is unavailable.
 */
int32_t FM_PORT_DWT_CyclesToUs(int32_t cycles);

/**
 * @brief   Busy-wait for a short interval using the DWT cycle counter.
 *
 * @param   time_us  Delay interval in microseconds.
 */
void FM_PORT_DWT_DelayUs(uint32_t time_us);

/**
 * @brief   Read the current DWT cycle counter value.
 *
 * @return  Current cycle count, or 0 when CYCCNT is not available.
 */
static inline uint32_t FM_PORT_DWT_GetCycles(void)
{
#if FM_PORT_DWT_HAS_CYCCNT
    return DWT->CYCCNT;
#else
    return 0U; /* no cycle counter */
#endif
}

/**
 * @brief   Reset the DWT cycle counter when supported.
 */
static inline void FM_PORT_DWT_Reset(void)
{
#if FM_PORT_DWT_HAS_CYCCNT
    DWT->CYCCNT = 0;
#endif
}

#endif /* FM_PORT_DWT_H */
