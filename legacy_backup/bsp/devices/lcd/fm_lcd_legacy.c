/**
 * @file    fm_lcd_legacy.c
 * @brief   Frozen high-level LCD helper implementation from the legacy path.
 */

#include "fm_lcd_legacy.h"

#include <stdio.h>
#include <string.h>

/* =========================== Private Macros ============================= */
#define FM_LCD_BUFFER_SIZE    20U

/* =========================== Private Data =============================== */
static char g_fm_lcd_buffer_[FM_LCD_BUFFER_SIZE];

/* =========================== Public Bodies ============================== */
void FM_LCD_Clear(void)
{
    FM_LCD_LL_Clear();
}

void FM_LCD_DecimalPointWrite(uint8_t p_col, fm_lcd_ll_row_t p_row, bool p_on)
{
    FM_LCD_LL_DecimalPointWrite(p_col, p_row, p_on);
}

void FM_LCD_DisplayMessage(const char *p_msg, fm_lcd_ll_row_t p_row)
{
    if (p_msg == NULL)
    {
        return;
    }

    FM_LCD_Clear();
    FM_LCD_PutString(p_msg, (uint32_t)strlen(p_msg), p_row);
    FM_LCD_Refresh();
}

void FM_LCD_Fill(uint8_t p_fill)
{
    FM_LCD_LL_Fill(p_fill);
}

void FM_LCD_Init(uint8_t p_fill)
{
    FM_LCD_LL_Init(p_fill);
}

void FM_LCD_PutString(const char *p_str, uint32_t p_str_len, fm_lcd_ll_row_t p_row)
{
    uint8_t row_size;
    uint8_t display_index;
    uint32_t source_index;

    if (p_str == NULL)
    {
        return;
    }

    row_size = FM_LCD_LL_GetRowSize(p_row);

    if (row_size == 0U)
    {
        return;
    }

    display_index = 0U;
    source_index = 0U;

    while ((display_index < row_size) &&
           (source_index < p_str_len) &&
           (p_str[source_index] != '\0'))
    {
        if (p_str[source_index] == '.')
        {
            if (display_index > 0U)
            {
                FM_LCD_LL_DecimalPointWrite((uint8_t)(display_index - 1U), p_row, true);
            }

            source_index++;
            continue;
        }

        FM_LCD_LL_PutChar(p_str[source_index], display_index, p_row);
        display_index++;
        source_index++;
    }
}

void FM_LCD_PutUnsignedInt32(uint32_t p_value, fm_lcd_ll_row_t p_row)
{
    int char_count;
    uint8_t row_size;

    row_size = FM_LCD_LL_GetRowSize(p_row);

    if (row_size == 0U)
    {
        return;
    }

    char_count = snprintf(g_fm_lcd_buffer_,
                          sizeof(g_fm_lcd_buffer_),
                          "%lu",
                          (unsigned long)p_value);

    if (char_count <= 0)
    {
        return;
    }

    if ((uint32_t)char_count > row_size)
    {
        char_count = (int)row_size;
    }

    while (char_count > 0)
    {
        row_size--;
        char_count--;
        FM_LCD_LL_PutChar(g_fm_lcd_buffer_[char_count], row_size, p_row);
    }
}

void FM_LCD_Refresh(void)
{
    FM_LCD_LL_Refresh();
}

void FM_LCD_SymbolWrite(fm_lcd_ll_symbol_t p_symbol, bool p_on)
{
    FM_LCD_LL_SymbolWrite(p_symbol, p_on);
}

/*** end of file ***/
