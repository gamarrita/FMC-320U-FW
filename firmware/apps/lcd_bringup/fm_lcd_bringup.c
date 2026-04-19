/**
 * @file    fm_lcd_bringup.c
 * @brief   Minimal bring-up app for the redesigned LCD stack.
 */

#include "fm_lcd_bringup.h"

#include "fm_board.h"
#include "fm_debug.h"
#include "fm_port_time.h"
#include "devices/lcd/fm_lcd.h"

/* =========================== Private Macros ============================= */
#define FM_LCD_BRINGUP_IDLE_DELAY_MS    100U

/* =========================== Public Bodies ============================== */
void FM_LcdBringup_Run(void)
{
    fm_lcd_status_t status;

    FM_BOARD_Init();
    FM_DEBUG_Init();

    (void) FM_DEBUG_UartStr("LCD_BRINGUP:START\n");

    status = FM_LCD_Init();

    if (status != FM_LCD_OK)
    {
        FM_DEBUG_PanicMsg("LCD_BRINGUP:LCD_INIT_FAILL\n");
    }

    (void) FM_DEBUG_UartStr("LCD_BRINGUP:LCD_INIT_OK\n");

    status = FM_LCD_Clear();

    if (status != FM_LCD_OK)
    {
        FM_DEBUG_PanicMsg("LCD_BRINGUP:CLEAR_FAIL\n");
    }

    status = FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_TOP,
                              "12.34",
                              FM_LCD_ALIGN_LEFT,
                              true);

    if (status != FM_LCD_OK)
    {
        FM_DEBUG_PanicMsg("LCD_BRINGUP:ROW_TOP_FAIL\n");
    }

    (void) FM_DEBUG_UartStr("LCD_BRINGUP:ROW_TOP_OK\n");

    status = FM_LCD_SetIndicator(FM_LCD_LAYOUT_INDICATOR_BATTERY, true);

    if (status != FM_LCD_OK)
    {
        FM_DEBUG_PanicMsg("LCD_BRINGUP:INDICATOR_FAIL\n");
    }

    (void) FM_DEBUG_UartStr("LCD_BRINGUP:INDICATOR_OK\n");

    status = FM_LCD_Flush();

    if (status != FM_LCD_OK)
    {
        FM_DEBUG_PanicMsg("LCD_BRINGUP:FLUSH_FAIL\n");
    }

    (void) FM_DEBUG_UartStr("LCD_BRINGUP:FLUSH_OK\n");

    FM_DEBUG_LedRun(FM_DEBUG_LED_ON);
    (void) FM_DEBUG_UartStr("LCD_BRINGUP:IDLE\n");

    for (;;)
    {
        FM_DEBUG_Flush();
        FM_PORT_TIME_SleepMs(FM_LCD_BRINGUP_IDLE_DELAY_MS);
    }
}

/*** end of file ***/
