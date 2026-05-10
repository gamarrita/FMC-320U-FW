/**
 * @file    fm_pcf8553_bringup.c
 * @brief   Minimal bring-up app for the redesigned PCF8553 backend.
 *
 * This app intentionally targets the new backend contract only.
 * It is a small smoke path, not the final backend validation flow.
 */

#include "fm_pcf8553_bringup.h"

#include <stddef.h>
#include <stdint.h>

#include "fm_board.h"
#include "fm_debug.h"
#include "fm_port_time.h"
#include "devices/lcd/pcf8553/fm_pcf8553.h"

/* =========================== Private Defines ============================== */
#define FM_PCF8553_BRINGUP_IDLE_DELAY_MS   100U

void FM_Pcf8553Bringup_Run(void)
{
    static const uint8_t g_fm_pcf8553_bringup_clear_frame_[FM_PCF8553_RAM_SIZE] = { 0 };
    fm_pcf8553_status_t status;

    FM_BOARD_Init();
    FM_DEBUG_Init();

    (void) FM_DEBUG_UartStr("PCF8553_BRINGUP:START\n");

    (void) FM_DEBUG_UartStr("PCF8553_BRINGUP:BACKEND_INIT_START\n");
    status = FM_PCF8553_Init();

    if (status != FM_PCF8553_OK)
    {
        FM_DEBUG_PanicMsg("PCF8553_BRINGUP:BACKEND_INIT_FAIL\n");
    }

    (void) FM_DEBUG_UartStr("PCF8553_BRINGUP:BACKEND_INIT_OK\n");

    status = FM_PCF8553_WriteRam(0U,
                                 g_fm_pcf8553_bringup_clear_frame_,
                                 FM_PCF8553_RAM_SIZE);

    if (status != FM_PCF8553_OK)
    {
        FM_DEBUG_PanicMsg("PCF8553_BRINGUP:BACKEND_WRITE_FAIL\n");
    }

    (void) FM_DEBUG_UartStr("PCF8553_BRINGUP:BACKEND_WRITE_OK\n");
    FM_DEBUG_LedRun(FM_DEBUG_LED_ON);
    (void) FM_DEBUG_UartStr("PCF8553_BRINGUP:IDLE\n");

    for (;;)
    {
        FM_DEBUG_Flush();
        FM_PORT_TIME_SleepMs(FM_PCF8553_BRINGUP_IDLE_DELAY_MS);
    }
}
