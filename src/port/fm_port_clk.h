/**
 * @file    fm_port_clk.h
 * @brief   Platform clock helpers for MCU timing queries.
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
 * @return  CPU cycles per microsecond, clamped to at least 1.
 */
uint32_t FM_PORT_CLK_CyclesPerUs(void);

#endif /* FM_PORT_CLK_H */
