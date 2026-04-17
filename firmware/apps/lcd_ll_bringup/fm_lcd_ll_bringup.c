#include "fm_lcd_ll_bringup.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "fm_board.h"
#include "fm_debug.h"
#include "fm_port_time.h"
#include "devices/lcd/fm_lcd_ll.h"

#define FM_LCD_LL_BRINGUP_IDLE_DELAY_MS    100U

static void fm_lcd_ll_bringup_log_(const char *p_msg);
static void fm_lcd_ll_bringup_write_pattern_(void);
static void fm_lcd_ll_bringup_write_row_(const char *p_text, fm_lcd_ll_row_t p_row);

static void fm_lcd_ll_bringup_log_(const char *p_msg)
{
    if (p_msg == NULL)
    {
        return;
    }

    (void)FM_DEBUG_UartMsg(p_msg, (uint32_t)strlen(p_msg));
}

static void fm_lcd_ll_bringup_write_pattern_(void)
{
    FM_LCD_LL_Clear();
    fm_lcd_ll_bringup_log_("LCD_LL_BRINGUP:CLEAR_OK\n");

    fm_lcd_ll_bringup_write_row_("12.34.56", FM_LCD_LL_ROW_1);
    fm_lcd_ll_bringup_write_row_("1.23.45", FM_LCD_LL_ROW_2);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_BATTERY, true);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_RATE, true);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_SLASH, true);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_H, true);
    FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_M, true);
    FM_LCD_LL_Refresh();
    fm_lcd_ll_bringup_log_("LCD_LL_BRINGUP:PATTERN_OK\n");
}

static void fm_lcd_ll_bringup_write_row_(const char *p_text, fm_lcd_ll_row_t p_row)
{
    uint8_t col;
    uint8_t row_size;
    char current_char;

    if (p_text == NULL)
    {
        return;
    }

    row_size = FM_LCD_LL_GetRowSize(p_row);
    col = 0U;

    while ((col < row_size) && (*p_text != '\0'))
    {
        current_char = *p_text;

        if (current_char == '.')
        {
            if (col > 0U)
            {
                FM_LCD_LL_DecimalPointWrite((uint8_t)(col - 1U), p_row, true);
            }

            p_text++;
            continue;
        }

        FM_LCD_LL_PutChar(current_char, col, p_row);
        col++;
        p_text++;
    }
}

void FM_LcdLlBringup_Run(void)
{
    FM_BOARD_Init();
    FM_DEBUG_Init();

    fm_lcd_ll_bringup_log_("LCD_LL_BRINGUP:START\n");
    FM_LCD_LL_Init(0U);
    fm_lcd_ll_bringup_log_("LCD_LL_BRINGUP:INIT_OK\n");

    fm_lcd_ll_bringup_write_pattern_();

    FM_DEBUG_LedRun(FM_DEBUG_LED_ON);
    fm_lcd_ll_bringup_log_("LCD_LL_BRINGUP:IDLE\n");

    for (;;)
    {
        FM_DEBUG_Flush();
        FM_PORT_TIME_SleepMs(FM_LCD_LL_BRINGUP_IDLE_DELAY_MS);
    }
}
