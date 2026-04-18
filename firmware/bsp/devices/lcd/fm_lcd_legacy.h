/**
 * @file    fm_lcd_legacy.h
 * @brief   Legacy high-level helpers for the FMC segmented LCD.
 *
 * This header preserves the legacy `fm_lcd.c` contract while the new LCD stack
 * is being redesigned around a new `fm_lcd.h` API.
 */

#ifndef FM_LCD_LEGACY_H_
#define FM_LCD_LEGACY_H_

#include <stdbool.h>
#include <stdint.h>

#include "fm_lcd_ll.h"

void FM_LCD_Clear(void);
void FM_LCD_DecimalPointWrite(uint8_t p_col, fm_lcd_ll_row_t p_row, bool p_on);
void FM_LCD_DisplayMessage(const char *p_msg, fm_lcd_ll_row_t p_row);
void FM_LCD_Fill(uint8_t p_fill);
void FM_LCD_Init(uint8_t p_fill);
void FM_LCD_PutString(const char *p_str, uint32_t p_str_len, fm_lcd_ll_row_t p_row);
void FM_LCD_PutUnsignedInt32(uint32_t p_value, fm_lcd_ll_row_t p_row);
void FM_LCD_Refresh(void);
void FM_LCD_SymbolWrite(fm_lcd_ll_symbol_t p_symbol, bool p_on);

#endif /* FM_LCD_LEGACY_H_ */

/*** end of file ***/
