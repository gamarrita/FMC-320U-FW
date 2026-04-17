#include "fm_pcf8553_bringup.h"

#include <string.h>

#include "fm_board.h"
#include "fm_debug.h"
#include "fm_port_time.h"
#include "devices/lcd/pcf8553/fm_pcf8553.h"

#define FM_PCF8553_BRINGUP_IDLE_DELAY_MS   100U

static void fm_pcf8553_bringup_log_(const char *p_msg)
{
    if (p_msg == NULL)
    {
        return;
    }

    (void) FM_DEBUG_UartMsg(p_msg, (uint32_t) strlen(p_msg));
}

static void fm_pcf8553_bringup_run_all_on_(void)
{
    fm_pcf8553_bringup_log_("PCF8553_BRINGUP:INIT_START\n");
    FM_PCF8553_Init();
    fm_pcf8553_bringup_log_("PCF8553_BRINGUP:INIT_OK\n");

    FM_PCF8553_WriteAll(PCF8553_SEGMENTS_ON);
    fm_pcf8553_bringup_log_("PCF8553_BRINGUP:ALL_ON_OK\n");
}

void FM_Pcf8553Bringup_Run(void)
{
    FM_BOARD_Init();
    FM_DEBUG_Init();

    fm_pcf8553_bringup_log_("PCF8553_BRINGUP:START\n");
    fm_pcf8553_bringup_run_all_on_();
    FM_DEBUG_LedRun(FM_DEBUG_LED_ON);
    fm_pcf8553_bringup_log_("PCF8553_BRINGUP:IDLE\n");

    for (;;)
    {
        FM_DEBUG_Flush();
        FM_PORT_TIME_SleepMs(FM_PCF8553_BRINGUP_IDLE_DELAY_MS);
    }
}
