/**
 * @file    fm_lcd_ll.h
 * @brief   LCD glass mapping layer for the FMC segmented display.
 */

#ifndef FM_LCD_LL_H_
#define FM_LCD_LL_H_

#include <stdbool.h>
#include <stdint.h>

#define FM_LCD_LL_ROW_1_COLS    8U
#define FM_LCD_LL_ROW_2_COLS    7U

typedef enum
{
    FM_LCD_LL_ROW_1 = 0,
    FM_LCD_LL_ROW_2
} fm_lcd_ll_row_t;

typedef enum
{
    FM_LCD_LL_SYM_POINT = 0,
    FM_LCD_LL_SYM_BATTERY,
    FM_LCD_LL_SYM_POWER,
    FM_LCD_LL_SYM_RATE,
    FM_LCD_LL_SYM_E,
    FM_LCD_LL_SYM_BATCH,
    FM_LCD_LL_SYM_ACM_1,
    FM_LCD_LL_SYM_TTL,
    FM_LCD_LL_SYM_SLASH,
    FM_LCD_LL_SYM_ACM_2,
    FM_LCD_LL_SYM_H,
    FM_LCD_LL_SYM_D,
    FM_LCD_LL_SYM_S,
    FM_LCD_LL_SYM_M,
    FM_LCD_LL_SYM_COUNT
} fm_lcd_ll_symbol_t;

void FM_LCD_LL_Clear(void);
void FM_LCD_LL_DecimalPointWrite(uint8_t p_col, fm_lcd_ll_row_t p_row, bool p_on);
void FM_LCD_LL_Fill(uint8_t p_fill);
uint8_t FM_LCD_LL_GetRowSize(fm_lcd_ll_row_t p_row);
void FM_LCD_LL_Init(uint8_t p_fill);
void FM_LCD_LL_PutChar(char p_char, uint8_t p_col, fm_lcd_ll_row_t p_row);
void FM_LCD_LL_Refresh(void);
void FM_LCD_LL_SymbolWrite(fm_lcd_ll_symbol_t p_symbol, bool p_on);

#endif /* FM_LCD_LL_H_ */

/*** end of file ***/
