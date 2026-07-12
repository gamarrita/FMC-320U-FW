/**
 * @file fm_lcd.h
 * @brief High-level helpers for the FMC segment LCD.
 */

#ifndef FM_LCD_H_
#define FM_LCD_H_

#include "fm_lcd_ll.h"

/** Indicates whether the UI is showing user data or configuration screens. */
typedef enum {
    USER,
    CONFIGURATION,
} user_or_configuration_t;

void FM_LCD_Init(uint8_t fill);
void FM_LCD_fill(uint8_t fill);
void FM_LCD_PutString(const char *str, uint32_t str_len, fm_lcd_ll_row_t row);
void FM_LCD_PutUnsignedInt32(uint32_t value, fm_lcd_ll_row_t row);
void FM_LCD_PutChar(char *ch);

#endif // FM_LCD_H_

