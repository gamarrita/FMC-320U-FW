/**
 * @file    fm_lcd_ll_bringup.c
 * @brief   Staged bring-up flow for the low-level LCD driver.
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
#define FM_LCD_LL_BRINGUP_EXPECTED_ROW_1_COLS      8U
#define FM_LCD_LL_BRINGUP_EXPECTED_ROW_2_COLS      7U


/* =========================== Private Types ================================ */


/* =========================== Private Data ================================= */


/* =========================== Private Prototypes =========================== */
static void fm_lcd_ll_bringup_fail_(const char *p_msg, int32_t p_param);
static void fm_lcd_ll_bringup_show_acm_1_(void);
static void fm_lcd_ll_bringup_show_alpha_pair_(char p_digit_0_char,
                                               char p_digit_1_char,
                                               const char *p_msg);
static void fm_lcd_ll_bringup_show_alpha_patterns_(void);
static void fm_lcd_ll_bringup_show_all_confirmed_(void);
static void fm_lcd_ll_bringup_show_clear_(void);
static void fm_lcd_ll_bringup_show_confirmed_symbols_(void);
static void fm_lcd_ll_bringup_show_decimal_points_(fm_lcd_ll_row_t p_row);
static void fm_lcd_ll_bringup_show_row_all_eights_(fm_lcd_ll_row_t p_row);
static void fm_lcd_ll_bringup_stage_done_(const char *p_msg);
static void fm_lcd_ll_bringup_validate_geometry_(void);
static void fm_lcd_ll_bringup_write_all_confirmed_symbols_(bool p_on);
static void fm_lcd_ll_bringup_write_all_decimal_points_(fm_lcd_ll_row_t p_row, bool p_on);
static void fm_lcd_ll_bringup_write_all_eights_(fm_lcd_ll_row_t p_row);


/* =========================== Private Bodies =============================== */
static void fm_lcd_ll_bringup_fail_(const char *p_msg, int32_t p_param)
{
    FM_DEBUG_ReportErrorWithParam(FM_DEBUG_ERR_BACKEND, p_param);

    if (p_msg != NULL)
    {
        (void) FM_DEBUG_UartStr(p_msg);
        FM_DEBUG_PanicMsg(p_msg);
    }
    else
    {
        FM_DEBUG_PanicMsg("LCD_LL_BRINGUP:FAIL\n");
    }
}

static void 
fm_lcd_ll_bringup_show_alpha_pair_(char p_digit_0_char,
                                               char p_digit_1_char,
                                               const char *p_msg)
{
    FM_LCD_LL_Clear();
    FM_LCD_LL_AlphaPutChar(p_digit_0_char, FM_LCD_LL_ALPHA_DIGIT_0);
    FM_LCD_LL_AlphaPutChar(p_digit_1_char, FM_LCD_LL_ALPHA_DIGIT_1);
    fm_lcd_ll_bringup_stage_done_(p_msg);
}

static void fm_lcd_ll_bringup_show_alpha_patterns_(void)
{
    fm_lcd_ll_bringup_show_alpha_pair_('A', 'A', "LCD_LL_BRINGUP:ALPHA_AA_OK\n");
    fm_lcd_ll_bringup_show_alpha_pair_('H', 'H', "LCD_LL_BRINGUP:ALPHA_HH_OK\n");
    fm_lcd_ll_bringup_show_alpha_pair_('N', 'N', "LCD_LL_BRINGUP:ALPHA_NN_OK\n");
    fm_lcd_ll_bringup_show_alpha_pair_('M', 'M', "LCD_LL_BRINGUP:ALPHA_MM_OK\n");
    fm_lcd_ll_bringup_show_alpha_pair_('X', 'X', "LCD_LL_BRINGUP:ALPHA_XX_OK\n");
    fm_lcd_ll_bringup_show_alpha_pair_('2', '2', "LCD_LL_BRINGUP:ALPHA_22_OK\n");
    fm_lcd_ll_bringup_show_alpha_pair_('8', '8', "LCD_LL_BRINGUP:ALPHA_88_OK\n");
}

static void fm_lcd_ll_bringup_show_all_confirmed_(void)
{
    FM_LCD_LL_Clear();
    fm_lcd_ll_bringup_write_all_eights_(FM_LCD_LL_ROW_1);
    fm_lcd_ll_bringup_write_all_decimal_points_(FM_LCD_LL_ROW_1, true);
    fm_lcd_ll_bringup_write_all_eights_(FM_LCD_LL_ROW_2);
    fm_lcd_ll_bringup_write_all_decimal_points_(FM_LCD_LL_ROW_2, true);
    fm_lcd_ll_bringup_write_all_confirmed_symbols_(true);

    fm_lcd_ll_bringup_stage_done_("LCD_LL_BRINGUP:ALL_CONFIRMED_ON_OK\n");
}

static void fm_lcd_ll_bringup_show_clear_(void)
{
    FM_LCD_LL_Clear();
    fm_lcd_ll_bringup_stage_done_("LCD_LL_BRINGUP:CLEAR_OK\n");
}

static void fm_lcd_ll_bringup_show_confirmed_symbols_(void)
{
    FM_LCD_LL_Clear();
    fm_lcd_ll_bringup_write_all_confirmed_symbols_(true);

    fm_lcd_ll_bringup_stage_done_("LCD_LL_BRINGUP:SYMBOLS_CONFIRMED_ON_OK\n");
}

static void fm_lcd_ll_bringup_show_acm_1_(void)
{
    FM_LCD_LL_Clear();
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_ACM_1, true);

    fm_lcd_ll_bringup_stage_done_("LCD_LL_BRINGUP:ACM_1_ON_OK\n");
}

static void fm_lcd_ll_bringup_show_decimal_points_(fm_lcd_ll_row_t p_row)
{
    FM_LCD_LL_Clear();
    fm_lcd_ll_bringup_write_all_eights_(p_row);
    fm_lcd_ll_bringup_write_all_decimal_points_(p_row, true);

    if (p_row == FM_LCD_LL_ROW_1)
    {
        fm_lcd_ll_bringup_stage_done_("LCD_LL_BRINGUP:ROW1_DP_ALL_ON_OK\n");
    }
    else
    {
        fm_lcd_ll_bringup_stage_done_("LCD_LL_BRINGUP:ROW2_DP_ALL_ON_OK\n");
    }
}

static void fm_lcd_ll_bringup_show_row_all_eights_(fm_lcd_ll_row_t p_row)
{
    FM_LCD_LL_Clear();
    fm_lcd_ll_bringup_write_all_eights_(p_row);

    if (p_row == FM_LCD_LL_ROW_1)
    {
        fm_lcd_ll_bringup_stage_done_("LCD_LL_BRINGUP:ROW1_ALL_8_OK\n");
    }
    else
    {
        fm_lcd_ll_bringup_stage_done_("LCD_LL_BRINGUP:ROW2_ALL_8_OK\n");
    }
}

static void fm_lcd_ll_bringup_stage_done_(const char *p_msg)
{
    FM_DEBUG_LedSignal(FM_DEBUG_LED_ON);
    FM_LCD_LL_Refresh();
    (void) FM_DEBUG_UartStr(p_msg);
    FM_PORT_TIME_SleepMs(FM_LCD_LL_BRINGUP_STAGE_HOLD_MS);
    FM_DEBUG_LedSignal(FM_DEBUG_LED_OFF);
}

static void fm_lcd_ll_bringup_validate_geometry_(void)
{
    if (FM_LCD_LL_GetRowSize(FM_LCD_LL_ROW_1) != FM_LCD_LL_BRINGUP_EXPECTED_ROW_1_COLS)
    {
        fm_lcd_ll_bringup_fail_("LCD_LL_BRINGUP:FAIL:ROW1_SIZE\n",
                                (int32_t) FM_LCD_LL_GetRowSize(FM_LCD_LL_ROW_1));
    }

    if (FM_LCD_LL_GetRowSize(FM_LCD_LL_ROW_2) != FM_LCD_LL_BRINGUP_EXPECTED_ROW_2_COLS)
    {
        fm_lcd_ll_bringup_fail_("LCD_LL_BRINGUP:FAIL:ROW2_SIZE\n",
                                (int32_t) FM_LCD_LL_GetRowSize(FM_LCD_LL_ROW_2));
    }
}

static void fm_lcd_ll_bringup_write_all_confirmed_symbols_(bool p_on)
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

static void fm_lcd_ll_bringup_write_all_decimal_points_(fm_lcd_ll_row_t p_row, bool p_on)
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
        FM_LCD_LL_DecimalPointWrite(col, p_row, p_on);
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
    fm_lcd_ll_bringup_validate_geometry_();

    fm_lcd_ll_bringup_show_clear_();
    fm_lcd_ll_bringup_show_row_all_eights_(FM_LCD_LL_ROW_1);
    fm_lcd_ll_bringup_show_decimal_points_(FM_LCD_LL_ROW_1);
    fm_lcd_ll_bringup_show_row_all_eights_(FM_LCD_LL_ROW_2);
    fm_lcd_ll_bringup_show_decimal_points_(FM_LCD_LL_ROW_2);
    fm_lcd_ll_bringup_show_confirmed_symbols_();
    fm_lcd_ll_bringup_show_all_confirmed_();
    fm_lcd_ll_bringup_show_alpha_patterns_();
    fm_lcd_ll_bringup_show_acm_1_();

    (void) FM_DEBUG_UartStr("LCD_LL_BRINGUP:ACM1_VALIDATED\n");
    FM_DEBUG_LedRun(FM_DEBUG_LED_ON);
    (void) FM_DEBUG_UartStr("LCD_LL_BRINGUP:IDLE\n");

    for (;;)
    {
        FM_DEBUG_Flush();
        FM_PORT_TIME_SleepMs(FM_LCD_LL_BRINGUP_IDLE_DELAY_MS);
    }
}
