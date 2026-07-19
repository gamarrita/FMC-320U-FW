/**
 * @file    fm_port_threadx_idle.h
 * @brief   ThreadX idle-state port hooks.
 *
 * This module provides the application hooks used by ThreadX low-power idle
 * support. The current implementation uses MCU sleep mode only; it does not
 * enter STOP mode, program an alternate wake timer, or adjust ThreadX ticks.
 */
#ifndef FM_PORT_THREADX_IDLE_H
#define FM_PORT_THREADX_IDLE_H

#include "tx_api.h"

/**
 * @brief Prepare the current idle sleep interval.
 *
 * The current bootstrap implementation has no alternate low-power timer, so
 * the requested ThreadX timer count is accepted but not programmed.
 *
 * @param count ThreadX timer count until the next known expiration.
 */
void FM_PORT_THREADX_IDLE_TimerSetup(ULONG count);

/**
 * @brief Notify the port that the ThreadX scheduler is entering idle.
 *
 * Turns the debug-gated run LED off and enters MCU sleep mode with WFI. This is
 * intentionally lighter than STOP mode so the current ThreadX tick path keeps
 * working during the bootstrap baseline.
 *
 * @warning Called from the ThreadX scheduler low-power path.
 * @warning Do not block.
 */
void FM_PORT_THREADX_IDLE_Enter(void);

/**
 * @brief Notify the port that the ThreadX scheduler is leaving idle.
 *
 * Turns the debug-gated run LED on.
 *
 * @warning Called from the ThreadX scheduler low-power path.
 * @warning Do not block.
 */
void FM_PORT_THREADX_IDLE_Exit(void);

/**
 * @brief Return elapsed ThreadX ticks while the scheduler was idle.
 *
 * @return Always 0 until an alternate low-power wake timer exists.
 */
ULONG FM_PORT_THREADX_IDLE_TimerAdjust(void);

#endif /* FM_PORT_THREADX_IDLE_H */
