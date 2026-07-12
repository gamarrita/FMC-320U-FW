/*
 * Autor: Daniel H Sagarra
 *
 * Notas:
 * LCD low level driver:
 * Fila HIGH de 8 caracteres
 * Fila LOW de 7 caracteres
 * Símbolos RATE, BACH, TTL, ACM, H, M, S, D....
 * Puntos para números decimales de cada fila (7 puntos en la fila HIGH, y
 * 6 en la fila LOW).
 * También formatea los datos pasados como parámetros en funciones de fm_lcd.c
 * con la función lcd_format_number_in_line().
 *
 * Para que los parámetros pasados a las funciones sean genéricos y
 * representativos, se utilizan enumeraciones cuyos elementos y tipo de datos
 * son clave para su entendimiento y posterior uso. (sand)
 *
 *
 */

#ifndef FM_LCD_LL_H_
#define FM_LCD_LL_H_

// includes
#include "fm_pcf8553.h"
#include "main.h"

// Typedefs.

// ROW_1 es la fila de arriba, ROW_2 es la de abajo. Se usan como parámetros de funciones entre otras cosas.
typedef enum
{
    FM_LCD_LL_ROW_1 = 0, FM_LCD_LL_ROW_2 = 1
} fm_lcd_ll_row_t;

//
typedef enum
{
    FM_LCD_LL_BLINK_OFF = 0,  // Símbolo encendido
    FM_LCD_LL_BLINK_ON_ON,  // Símbolo en modo parpadeo estado encendido
    FM_LCD_LL_BLINK_ON_OFF  // Simbolo en modo parpadeo estado apagado.
} fm_lcd_ll_blink_t;

/*
 * Enumeración de los bits que se pueden escribir de los registros de la
 * pantalla LCD, sirve para elegir que segmento se selecciona de cada registro.
 */
typedef enum
{
    BIT_0, BIT_1, BIT_2, BIT_3, BIT_4, BIT_5, BIT_6, BIT_7
} bit_t;

/*
 * Enumeración de los registros que corresponden a los segmentos del LCD.
 * Cada registro tiene 8 bits, haciendo que 20 registros x 8 bits = 160
 * segmentos.
 */
typedef enum
{
    REG_0,
    REG_1,
    REG_2,
    REG_3,
    REG_4,
    REG_5,
    REG_6,
    REG_7,
    REG_8,
    REG_9,
    REG_10,
    REG_11,
    REG_12,
    REG_13,
    REG_14,
    REG_15,
    REG_16,
    REG_17,
    REG_18,
    REG_19
} reg_t;

typedef enum
{
    DIGIT_0, DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4, DIGIT_5, DIGIT_6, DIGIT_7
} screen_digits_t;

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
    FM_LCD_LL_SYM_BACKSLASH,
    FM_LCD_LL_SYM_ACM_2,
    FM_LCD_LL_SYM_S,
    FM_LCD_LL_SYM_M,
    FM_LCD_LL_SYM_H,
    FM_LCD_LL_SYM_D,
    DOT_ROW_1_DECI,
    DOT_ROW_1_CENTI,
    DOT_ROW_1_MILI,
    DOT_ROW_1_MICRO,
    DOT_ROW_1_NANO,
    DOT_ROW_1_PICO,
    DOT_ROW_1_FEMTO,
    DOT_ROW_2_DECI,
    DOT_ROW_2_CENTI,
    DOT_ROW_2_MILI,
    DOT_ROW_2_MICRO,
    DOT_ROW_2_NANO,
    DOT_ROW_2_PICO,
    FM_LCD_LL_SYM_END
} fm_lcd_ll_sym_t;

typedef enum
{
    LT, M3, KG, GL, BR, CELSIUS, NOTHING
} fmc_unit_volume_t;

typedef enum
{
    H, D, S, M, UNIT_TIME_END
} fmc_unit_time_t;

/*
 * Cantidad de filas y columnas de la pantalla, notar que se tiene dos filas
 * donde:
 * La fila 0 tiene 8 caracteres, equivalente a 8 columnas
 * La fila 1 tiene 7 caracteres, equivalente a 7 columnas
 *
 * Aun asi estas macros son para definir un buffer de 2x8, luego los límites
 * para cada fila del buffer serán 8 y 7 elementos para las filas 0 y 1
 * respectivamente.
 *
 */
#define FM_LCD_LL_ROWS    		2
#define FM_LCD_LL_ROW_1_COLS	8 // Incluye terminador nulo
#define FM_LCD_LL_ROW_2_COLS	7 // Incluye terminador nulo
#define FM_LCD_LL_COLS			8

// Public function prototypes.
void
FM_LCD_LL_BlinkClear();
void
FM_LCD_LL_BlinkChar(uint8_t state);
void
FM_LCD_LL_BlinkNumber(fm_lcd_ll_row_t row, uint8_t digit, fm_lcd_ll_blink_t state);
uint8_t
FM_LCD_LL_BlinkRefresh(uint8_t mode);
void
FM_LCD_LL_BlinkSymbol(fm_lcd_ll_sym_t, fm_lcd_ll_blink_t);
void
FM_LCD_LL_Clear();
uint32_t
FM_LCD_LL_GetRowSize(fm_lcd_ll_row_t);
void
FM_LCD_LL_Init(uint8_t);
void
FM_LCD_LL_PutChar(char c, uint8_t col, fm_lcd_ll_row_t row);
void
FM_LCD_LL_PutChar_1(char ascii_char);
void
FM_LCD_LL_PutChar_2(char ascii_char);
void
FM_LCD_LL_Refresh();
void
FM_LCD_LL_SymbolWrite(fm_lcd_ll_sym_t symbol, uint8_t state);

#endif /* FM_LCD_H_ */

/*** end of file ***/
