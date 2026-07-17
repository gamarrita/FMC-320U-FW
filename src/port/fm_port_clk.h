/**
 * @file    fm_port_clk.h
 * @brief   Platform clock helpers for MCU timing queries.
 *
 * This module exposes lightweight timing queries derived from the active HAL
 * clock state. It does not configure clocks; use `fm_port_rcc` for that.
 */

#ifndef FM_PORT_CLK_H
#define FM_PORT_CLK_H

#include <stdint.h>

/**
 * @brief   Get the current HCLK frequency reported by HAL.
 *
 * @return  HCLK frequency in hertz.
 */
uint32_t FM_PORT_CLK_GetHclkHz(void);

/**
 * @brief   Get the cached number of CPU cycles per microsecond.
 *
 * The value is derived from HCLK on first call and then cached. If the clock
 * tree changes later, this module does not currently refresh the cache.
 *
 * @return  CPU cycles per microsecond, clamped to at least 1.
 */
uint32_t FM_PORT_CLK_CyclesPerUs(void);

#endif /* FM_PORT_CLK_H */
