/**
 * @file    fm_port_threadx_idle.c
 * @brief   ThreadX idle-state port hooks.
 */

#include "fm_port_threadx_idle.h"

#include "fm_debug.h"
#include "main.h"

void FM_PORT_THREADX_IDLE_TimerSetup(ULONG count)
{
    (void) count;
}

void FM_PORT_THREADX_IDLE_Enter(void)
{
    FM_DEBUG_LedRun(FM_DEBUG_LED_OFF);
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

void FM_PORT_THREADX_IDLE_Exit(void)
{
    FM_DEBUG_LedRun(FM_DEBUG_LED_ON);
}

ULONG FM_PORT_THREADX_IDLE_TimerAdjust(void)
{
    return 0U;
}
