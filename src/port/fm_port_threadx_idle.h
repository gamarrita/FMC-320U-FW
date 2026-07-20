/**
 * @file    fm_port_threadx_idle.h
 * @brief   ThreadX idle-state port hooks.
 *
 * This module provides the application hooks used by ThreadX low-power idle
 * support. The implementation uses LPTIM1 as the STOP2 wake timer and elapsed
 * time source while the ThreadX SysTick path is stopped.
 */
#ifndef FM_PORT_THREADX_IDLE_H
#define FM_PORT_THREADX_IDLE_H

#include "tx_api.h"

/**
 * @brief Prepare the current idle sleep interval.
 *
 * Programs LPTIM1 channel 1 output-compare timing for the next known ThreadX
 * timer expiration.
 *
 * @param count ThreadX timer count until the next known expiration.
 */
void FM_PORT_THREADX_IDLE_TimerSetup(ULONG count);

/**
 * @brief Notify the port that the ThreadX scheduler is entering idle.
 *
 * Starts the programmed LPTIM1 wake timer when one is armed, turns the
 * debug-gated run LED off, masks the SysTick interrupt, and enters STOP2.
 *
 * @warning Called from the ThreadX scheduler low-power path.
 * @warning Do not block.
 */
void FM_PORT_THREADX_IDLE_Enter(void);

/**
 * @brief Notify the port that the ThreadX scheduler is leaving idle.
 *
 * Captures the elapsed LPTIM1 count and stops the LPTIM1 wake timer when it was
 * started before STOP2.
 *
 * @warning Called from the ThreadX scheduler low-power path.
 * @warning Do not block.
 */
void FM_PORT_THREADX_IDLE_Exit(void);

/**
 * @brief Return elapsed ThreadX ticks while the scheduler was idle.
 *
 * @return Elapsed ThreadX ticks converted from the captured LPTIM1 count.
 */
ULONG FM_PORT_THREADX_IDLE_TimerAdjust(void);

#endif /* FM_PORT_THREADX_IDLE_H */
