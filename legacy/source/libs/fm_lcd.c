/**
 * @file fm_lcd.c
 * @brief High-level rendering helpers for the FMC segment LCD.
 *
 * Provides wrappers around the low-level driver to print strings, digits and
 * individual characters on the two main rows of the display.
 */

// Includes.
#include "fm_lcd.h"
#include "stdio.h"

// Typedef.

// Const data.

// Defines.
#define LCD_BUFFER_SIZE 20

// Project variables, non-static, at least used in other file.

// External variables.

// Global variables, statics.

char lcd_buffer[LCD_BUFFER_SIZE];

// Private function prototypes.

// Private function bodies.

// Public function bodies.

/**
 * @brief Initializes the LCD driver and fills the frame buffer.
 * @param fill Pattern written to each LCD RAM byte (typically 0x00 or 0xFF).
 */
void FM_LCD_Init(uint8_t fill)
{
    FM_LCD_LL_Init(fill);
}

/**
 * @brief Writes two adjacent 7-segment characters to the LCD.
 * @param ptr Pointer to the pair of characters to render.
 */
void FM_LCD_PutChar(char *ptr)
{
    FM_LCD_LL_PutChar_1(*ptr);
    FM_LCD_LL_PutChar_2(*(ptr + 1));
}

/**
 * @brief Prints a string on either LCD row (8 chars on row 1, 7 on row 2).
 * @param my_str Text to render.
 * @param len    Number of characters available in the string.
 * @param row    Target row (FM_LCD_LL_ROW_1 or FM_LCD_LL_ROW_2).
 */
void FM_LCD_PutString(const char *my_str, uint32_t len, fm_lcd_ll_row_t row)
{
    uint8_t str_index = 0;
    uint8_t lcd_index = 0;

    int index_end;

    if (row == FM_LCD_LL_ROW_1)
    {
        // Clamp text length to the maximum capacity of the selected row.
        index_end = FM_LCD_LL_ROW_1_COLS;
    }
    else
    {
        // Clamp text length to the maximum capacity of the selected row.
        index_end = FM_LCD_LL_ROW_2_COLS;
    }

    if (len < index_end)
    {
        // String is shorter than the row capacity.
        index_end = len;
    }

        // Walk the input string and map separators (e.g. decimal points) to the LCD buffer.
    while ((lcd_index < index_end) && my_str[str_index])
    {
        FM_LCD_LL_PutChar(my_str[str_index], lcd_index, row);
        str_index++;
        lcd_index++;

                // If the next character is a dot, merge it with the previous digit.
        if ((my_str[str_index] == '.'))
        {
                        // Skip the decimal point when already at the last column.
            if (lcd_index < index_end)
            {
                FM_LCD_LL_PutChar(my_str[str_index], lcd_index - 1, row);
                // Consume the decimal point in the source string.
            }
            str_index++;
        }
    }
}

/**
 * @brief Formats and displays an unsigned integer on the selected row.
 * @param num Value to print.
 * @param row Destination row.
 */
void FM_LCD_PutUnsignedInt32(uint32_t num, fm_lcd_ll_row_t row)
{
    uint8_t index_lcd; // Column index within the selected row
    int index_num;  // Pointer within the formatted string

        // Start from the last available column on the selected row
    index_lcd = FM_LCD_LL_GetRowSize(row);

        // Convert the number to text; index_num becomes the number of digits.
    index_num = snprintf(lcd_buffer, sizeof(lcd_buffer), "%lu", num);

    if (index_num > index_lcd)
    {
        index_num = index_lcd; // Avoid exceeding the row capacity.
    }

    while (index_num)
    {
        // Columns are 0-indexed; step backwards while printing.
        index_lcd--;
        index_num--;
        FM_LCD_LL_PutChar(lcd_buffer[index_num], index_lcd, row);
    }
}

// Interrupts

/*** end of file ***/








