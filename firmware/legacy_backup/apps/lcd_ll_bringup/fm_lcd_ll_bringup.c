/**
 * @file    fm_lcd_ll_bringup.c
 * @brief   Minimal bring-up flow for the legacy LCD low-level driver.
 */

/* ===========================     Includes    ============================== */

#include "fm_lcd_ll_bringup.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "fm_board.h"
#include "fm_debug.h"
#include "fm_port_time.h"
#include "devices/lcd/fm_lcd_ll.h"


/* =========================== Private Defines ============================== */
#define FM_LCD_LL_BRINGUP_IDLE_DELAY_MS           100U
#define FM_LCD_LL_BRINGUP_STAGE_HOLD_MS          800U

/* =========================== Private Prototypes =========================== */
static void fm_lcd_ll_bringup_stage_done_(const char *p_msg);
static void fm_lcd_ll_bringup_write_all_indicators_(bool p_on);
static void fm_lcd_ll_bringup_write_all_decimal_points_(fm_lcd_ll_row_t p_row);
static void fm_lcd_ll_bringup_write_all_eights_(fm_lcd_ll_row_t p_row);


/* =========================== Private Bodies =============================== */
static void fm_lcd_ll_bringup_stage_done_(const char *p_msg)
{
    FM_DEBUG_LedSignal(FM_DEBUG_LED_ON);
    FM_LCD_LL_Refresh();
    (void) FM_DEBUG_UartStr(p_msg);
    FM_PORT_TIME_SleepMs(FM_LCD_LL_BRINGUP_STAGE_HOLD_MS);
    FM_DEBUG_LedSignal(FM_DEBUG_LED_OFF);
}

static void fm_lcd_ll_bringup_write_all_indicators_(bool p_on)
{
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_POINT, p_on);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_BATTERY, p_on);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_POWER, p_on);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_RATE, p_on);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_E, p_on);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_BATCH, p_on);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_TTL, p_on);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_SLASH, p_on);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_ACM_2, p_on);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_ACM_1, p_on);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_H, p_on);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_D, p_on);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_S, p_on);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_M, p_on);
}

static void fm_lcd_ll_bringup_write_all_decimal_points_(fm_lcd_ll_row_t p_row)
{
    uint8_t col;
    uint8_t row_size;

    row_size = FM_LCD_LL_GetRowSize(p_row);

    if (row_size == 0U)
    {
        return;
    }

    for (col = 0U; col < (row_size - 1U); col++)
    {
        FM_LCD_LL_DecimalPointWrite(col, p_row, true);
    }
}

static void fm_lcd_ll_bringup_write_all_eights_(fm_lcd_ll_row_t p_row)
{
    uint8_t col;
    uint8_t row_size;

    row_size = FM_LCD_LL_GetRowSize(p_row);

    for (col = 0U; col < row_size; col++)
    {
        FM_LCD_LL_PutChar('8', col, p_row);
    }
}


/* =========================== Public Bodies ================================ */
void FM_LcdLlBringup_Run(void)
{
    FM_BOARD_Init();
    FM_DEBUG_Init();

    (void) FM_DEBUG_UartStr("LCD_LL_BRINGUP:START\n");
    FM_LCD_LL_Init(0U);
    (void) FM_DEBUG_UartStr("LCD_LL_BRINGUP:INIT_OK\n");

    FM_LCD_LL_Clear();
    fm_lcd_ll_bringup_stage_done_("LCD_LL_BRINGUP:CLEAR_OK\n");

    FM_LCD_LL_Clear();
    fm_lcd_ll_bringup_write_all_eights_(FM_LCD_LL_ROW_1);
    fm_lcd_ll_bringup_stage_done_("LCD_LL_BRINGUP:ROW1_ALL_8_OK\n");

    FM_LCD_LL_Clear();
    fm_lcd_ll_bringup_write_all_eights_(FM_LCD_LL_ROW_2);
    fm_lcd_ll_bringup_stage_done_("LCD_LL_BRINGUP:ROW2_ALL_8_OK\n");

    FM_LCD_LL_Clear();
    fm_lcd_ll_bringup_write_all_eights_(FM_LCD_LL_ROW_1);
    fm_lcd_ll_bringup_write_all_decimal_points_(FM_LCD_LL_ROW_1);
    fm_lcd_ll_bringup_write_all_eights_(FM_LCD_LL_ROW_2);
    fm_lcd_ll_bringup_write_all_decimal_points_(FM_LCD_LL_ROW_2);
    fm_lcd_ll_bringup_stage_done_("LCD_LL_BRINGUP:NUMERIC_ALL_ON_OK\n");

    FM_LCD_LL_Clear();
    fm_lcd_ll_bringup_write_all_indicators_(true);
    fm_lcd_ll_bringup_stage_done_("LCD_LL_BRINGUP:INDICATORS_ON_OK\n");

    FM_LCD_LL_Clear();
    FM_LCD_LL_AlphaPutChar('8', FM_LCD_LL_ALPHA_DIGIT_0);
    FM_LCD_LL_AlphaPutChar('8', FM_LCD_LL_ALPHA_DIGIT_1);
    fm_lcd_ll_bringup_stage_done_("LCD_LL_BRINGUP:ALPHA_88_OK\n");

    FM_DEBUG_LedRun(FM_DEBUG_LED_ON);
    (void) FM_DEBUG_UartStr("LCD_LL_BRINGUP:IDLE\n");

    for (;;)
    {
        FM_DEBUG_Flush();
        FM_PORT_TIME_SleepMs(FM_LCD_LL_BRINGUP_IDLE_DELAY_MS);
    }
}
