/**
 * @file    fm_pulse_counter_bringup.c
 * @brief   Human-observed LPTIM4 pulse-counter bring-up across Run and idle.
 *
 * Human validation contract:
 * - enable debug UART messages before reset
 * - open the ST-LINK VCP at 115200 8N1
 * - wait for the READY banner
 * - control and independently count falling edges on FMC_PRIMARY_PULSE
 * - compare those edges with the raw counter observations
 * - for a silent run, boot without debug jumpers and install them later
 *   without resetting; resumed UART counts and Run LED activity prove that
 *   the same execution remained live
 *
 * The one-second task sleep lets the configured ThreadX low-power idle path
 * enter Stop2 between observations. This app therefore exercises accumulation
 * across normal Run/Stop2 transitions, but does not by itself establish the
 * complete Stop2, rollover, or silent-current acceptance required by 7B2.
 *
 * The first five falling edges after counter startup are intentionally not
 * counted by the asynchronous LPTIM path and are accepted by this instrument.
 */

#include "fm_pulse_counter_bringup.h"

#include <stdint.h>

#include "fm_board.h"
#include "fm_debug.h"
#include "fm_port_pulse_counter.h"
#include "fm_port_time.h"

#define FM_PULSE_COUNTER_BRINGUP_OBSERVATION_MS   (1000U)

static void fm_pulse_counter_bringup_write_count_(uint16_t count);

static void fm_pulse_counter_bringup_write_count_(uint16_t count)
{
    FM_DEBUG_UartStr("PULSE_COUNTER_BRINGUP:COUNT=");
    FM_DEBUG_UartUint32((uint32_t) count);
}

void FM_PulseCounterBringup_Run(void)
{
    FM_BOARD_Init();
    FM_DEBUG_Init();

    FM_DEBUG_UartStr("PULSE_COUNTER_BRINGUP:START\n");

    if (!FM_PORT_PulseCounterStart())
    {
        FM_DEBUG_PanicMsg("PULSE_COUNTER_BRINGUP:LPTIM4_START_FAIL");
    }

    FM_DEBUG_UartStr(
            "PULSE_COUNTER_BRINGUP:READY EDGE=FALLING "
            "STARTUP_LOSS_EDGES=5 MAX_HZ=1000\n");

    for (;;)
    {
        FM_DEBUG_RefreshJumpers();
        fm_pulse_counter_bringup_write_count_(
                FM_PORT_PulseCounterReadStable());
        FM_PORT_TIME_SleepMs(FM_PULSE_COUNTER_BRINGUP_OBSERVATION_MS);
    }
}
