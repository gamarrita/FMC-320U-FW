/**
 * @file    fm_port_threadx_idle.c
 * @brief   ThreadX idle-state port hooks.
 */

#include "fm_port_threadx_idle.h"

#include <stdbool.h>

#include "fm_debug.h"
#include "lptim.h"
#include "main.h"

#define FM_PORT_THREADX_IDLE_LPTIM_CLOCK_HZ 32768UL
#define FM_PORT_THREADX_IDLE_LPTIM_PRESCALER 16UL
#define FM_PORT_THREADX_IDLE_LPTIM_TICKS_PER_SECOND \
    (FM_PORT_THREADX_IDLE_LPTIM_CLOCK_HZ / FM_PORT_THREADX_IDLE_LPTIM_PRESCALER)
#define FM_PORT_THREADX_IDLE_LPTIM_MAX_COUNT 65535UL

static ULONG fm_threadx_idle_lptim_target_ticks;
static ULONG fm_threadx_idle_lptim_elapsed_ticks;
static ULONG fm_threadx_idle_lptim_remainder;
static bool fm_threadx_idle_lptim_armed;
static bool fm_threadx_idle_lptim_started;
static bool fm_threadx_idle_low_power_ready;

static void FM_PORT_THREADX_IDLE_LowPowerReady(void);

static void FM_PORT_THREADX_IDLE_LowPowerReady(void)
{
    if (!fm_threadx_idle_low_power_ready)
    {
        __HAL_RCC_LPTIM1_CLKAM_ENABLE();
        fm_threadx_idle_low_power_ready = true;
    }
}

static ULONG FM_PORT_THREADX_IDLE_TxTicksToLptimTicks(ULONG tx_ticks)
{
    uint64_t lptim_ticks;

    lptim_ticks = (((uint64_t) tx_ticks * FM_PORT_THREADX_IDLE_LPTIM_TICKS_PER_SECOND)
            + TX_TIMER_TICKS_PER_SECOND - 1U) / TX_TIMER_TICKS_PER_SECOND;

    if (lptim_ticks == 0U)
    {
        lptim_ticks = 1U;
    }

    if (lptim_ticks > FM_PORT_THREADX_IDLE_LPTIM_MAX_COUNT)
    {
        lptim_ticks = FM_PORT_THREADX_IDLE_LPTIM_MAX_COUNT;
    }

    return (ULONG) lptim_ticks;
}

void FM_PORT_THREADX_IDLE_TimerSetup(ULONG count)
{
    LPTIM_OC_ConfigTypeDef config =
    {
        .OCPolarity = LPTIM_OCPOLARITY_HIGH,
        .Pulse = FM_PORT_THREADX_IDLE_LPTIM_MAX_COUNT,
    };

    FM_PORT_THREADX_IDLE_LowPowerReady();

    fm_threadx_idle_lptim_target_ticks = FM_PORT_THREADX_IDLE_TxTicksToLptimTicks(count);
    fm_threadx_idle_lptim_elapsed_ticks = 0U;
    fm_threadx_idle_lptim_armed = false;
    fm_threadx_idle_lptim_started = false;

    config.Pulse = fm_threadx_idle_lptim_target_ticks;

    if (HAL_LPTIM_OC_ConfigChannel(&hlptim1, &config, LPTIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }

    fm_threadx_idle_lptim_armed = true;
}

void FM_PORT_THREADX_IDLE_Enter(void)
{
    FM_PORT_THREADX_IDLE_LowPowerReady();
    fm_threadx_idle_lptim_elapsed_ticks = 0U;

    if (fm_threadx_idle_lptim_armed)
    {
        if (HAL_LPTIM_PWM_Start_IT(&hlptim1, LPTIM_CHANNEL_1) != HAL_OK)
        {
            Error_Handler();
        }

        fm_threadx_idle_lptim_started = true;
    }

    FM_DEBUG_LedRun(FM_DEBUG_LED_OFF);

    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
    HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

    FM_DEBUG_LedRun(FM_DEBUG_LED_ON);
}

void FM_PORT_THREADX_IDLE_Exit(void)
{
    if (fm_threadx_idle_lptim_started)
    {
        fm_threadx_idle_lptim_elapsed_ticks = LPTIM1->CNT;

        if (HAL_LPTIM_PWM_Stop_IT(&hlptim1, LPTIM_CHANNEL_1) != HAL_OK)
        {
            Error_Handler();
        }

        fm_threadx_idle_lptim_started = false;
        fm_threadx_idle_lptim_armed = false;
    }
}

ULONG FM_PORT_THREADX_IDLE_TimerAdjust(void)
{
    uint64_t scaled_ticks;
    ULONG tx_ticks;

    scaled_ticks = ((uint64_t) fm_threadx_idle_lptim_elapsed_ticks
            * TX_TIMER_TICKS_PER_SECOND) + fm_threadx_idle_lptim_remainder;
    tx_ticks = (ULONG) (scaled_ticks / FM_PORT_THREADX_IDLE_LPTIM_TICKS_PER_SECOND);
    fm_threadx_idle_lptim_remainder =
            (ULONG) (scaled_ticks % FM_PORT_THREADX_IDLE_LPTIM_TICKS_PER_SECOND);
    fm_threadx_idle_lptim_elapsed_ticks = 0U;

    return tx_ticks;
}
