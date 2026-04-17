/*
 * @brief Este es un driver intermedio entre fm_lcd.c y pcf8553.c. Entre las
 * responsabilidades de este modulo están:
 * Dar accesos a la escritura y borrado de cada caracter, punto y simbolo en el
 *
 * Partes LCD de 160 segmentos:
 * Fila superior de caracteres numericos, cantidad 8, son de 7 segmentos.
 * Fila superior de puntos decimales cantidad 7.
 * Fila inferior de caracteres numericos, cantidad 7, son de 7 segmentos.
 * Fila inferior de puntos decimales cantidad 6.
 * Símbolos: Batería, POWER, RATE, E, BATCH, ACM_1, TTL, /, ACM_2, H, S, D, M
 *
 * Version 2
 * Autor: Daniel H Sagarra
 * Fecha: 10/11/2024
 * Modificaciones: version inicial.
 *
 *
 */

// Includes.
#include "fm_lcd_ll.h"
#include "fm_debug.h"

// Typedef.

typedef struct
{
    uint8_t pos;
    uint8_t reg;
} octal_t;

// Defines.
#define FALSE   0
#define TRUE    1

/*
 * Ver hoja de datos del lcd, cada segmento se identifica con una letra, se
 * codifica a numero. Cada segmento esta conectado a un pin del pcf8553, luego
 * cada pin esta relacionado con bit de un registro de 8 bits en el mapa de
 * memoria del pcf8553. Se debe construir una matiz que relacione los segmentos
 * con el bit que se quiere encender, esto se hará para un solo digito, luego
 * por se obtendrán las posiciones de los siguientes dígitos.
 *
 */
#define SEG_A 0
#define SEG_B 1
#define SEG_C 2
#define SEG_D 3
#define SEG_E 4
#define SEG_F 5
#define SEG_G 6
#define SEG_H 7

#define LCD_MSG_LENGTH 12

#define ROW_1_SIZE 8 // Caracteres en linea 1
#define ROW_2_SIZE 7 // Caracteres en linea 2

// Project variables, non-static, at least used in other file.

// Extern variables.

// Global variables, statics.

uint8_t blink_short_refresh = 0;

/*
 * Lo que se quiera escribir en las líneas 1 y 2 primero se vuelca a este
 * buffer. Leer el buffer es la única manera practica que se tiene para
 * saber que esta escrito en la pantalla, no se debe corromper esta condición.
 */
fm_lcd_ll_blink_t blink_number[FM_LCD_LL_ROWS][FM_LCD_LL_COLS]; // Mapa de dígitos a parpadear.
fm_lcd_ll_blink_t blink_char_1 = 0;
fm_lcd_ll_blink_t blink_char_2 = 0;
fm_lcd_ll_blink_t blink_symbol[FM_LCD_LL_SYM_END];

uint8_t g_col; // se usara para calcular que registro y posición del bit a modificar.
uint8_t g_row; // se usara para calcular que registro y posición del bit a modificar.

/*
 *  Para un caracter octal, en la linea superior del LCD, inicializo el
 *  siguiente arreglo. Ver tabla memoria del PCF8553, el encendido/apagado
 *  de cada segmento esta controlado por un bit en esta tabla, son 20 registros
 *  de 8 bits cada uno, 160 bits par controlar la misma cantidad de segmentos.
 *  Los valores de inicialización de la siguiente tabla corresponde al caracter
 *  ubicado mas a la derecha en la tabla superior, ver datasheet del LCD.
 *  Cada par de valores pos y reg se corresponden a la dirección de un registro
 *  y el bit correspondiente que controlan los segmentos de A a G.
 *  Solo se necesitan los datos de este caracter, las posiciones de los demás
 *  se obtienen por aritmética dentro de la función WriteLine.
 */
octal_t octal_1[] =
{
{ .pos = 6, .reg = 7 },
{ .pos = 6, .reg = 2 },
{ .pos = 7, .reg = 2 },
{ .pos = 6, .reg = 17 },
{ .pos = 6, .reg = 12 },
{ .pos = 7, .reg = 17 },
{ .pos = 7, .reg = 12 },
{ .pos = 7, .reg = 7 }, };

octal_t octal_2[] =
{
{ .pos = 7, .reg = 5 },
{ .pos = 7, .reg = 0 },
{ .pos = 6, .reg = 0 },
{ .pos = 7, .reg = 15 },
{ .pos = 7, .reg = 10 },
{ .pos = 6, .reg = 15 },
{ .pos = 6, .reg = 10 },
{ .pos = 6, .reg = 5 } };

/*
 * @brief   Controla el encendido y apagado del parpadeo.
 * @note
 * @param   state:  0 parpadeo desactivado,
 *                  1 activado y encendido,
 *                  2 activado y apagado.
 * @retval  ninguno
 */
void FM_LCD_LL_BlinkChar(uint8_t state)
{
    blink_char_1 = state;
    blink_char_2 = state;
}

// Private function prototypes.
void
WriteLine(uint8_t seg, uint8_t data);

// Public function bodies.

/*
 * @brief		Detiene todos los parpadeos.
 * @note
 * @param		ninguno.
 * @retval	ninguno
 */
void FM_LCD_LL_BlinkClear()
{

    for (int n = 0; n < FM_LCD_LL_SYM_END; n++)
    {
        blink_symbol[n] = FM_LCD_LL_BLINK_OFF;  //
    }

    for (int n = 0; n < FM_LCD_LL_ROW_1_COLS; n++)
    {
        blink_number[FM_LCD_LL_ROW_1][n] = FM_LCD_LL_BLINK_OFF;  //
    }

    for (int n = 0; n < FM_LCD_LL_ROW_2_COLS; n++)
    {
        blink_number[FM_LCD_LL_ROW_2][n] = FM_LCD_LL_BLINK_OFF;  //
    }

    blink_char_1 = 0;
    blink_char_2 = 0;
}

/*
 * @brief   Habilita o deshabilita el parpadeo de los numeros en la fila 1 y 2 de la pantalla.
 * @note
 * @param   row = 1, fila 1 selccionada
 *          row = 2, fila 2 seleccionada
 *          digit = x, digito seleccionado
 *          state =  estado del parpadeo
 *
 * @retval  ninguno
 */
void FM_LCD_LL_BlinkNumber(fm_lcd_ll_row_t row, uint8_t digit, fm_lcd_ll_blink_t state)
{
    switch (row)
    {
    case (FM_LCD_LL_ROW_1):
        if (digit < FM_LCD_LL_ROW_1_COLS)
        {
            blink_number[row][digit] = state;
        }
        break;
    case (FM_LCD_LL_ROW_2):
        if (digit < FM_LCD_LL_ROW_2_COLS)
        {
            blink_number[row][digit] = state;
        }
        break;
    default:
        break;
    }
}

/*
 * @brief   Cambia el estado de parpadedo. Para los segmentos que estén habilitados para parpadear
 *          esta función lee y ajusta su condición. Esta funcion solo afecta a los segmentos que
 *          tengan habilitado el parpadeo.
 * @param   mode    0, los segmentos habilitados se apagaran, al refrescar el LCD.
 *                  1, los segmentos habilitados se encenderan, al resfrescar el LCD.
 *                  2, solo lectura.
 * @retval
 */
uint8_t FM_LCD_LL_BlinkRefresh(uint8_t mode)
{
    uint8_t ret;

    ret = blink_short_refresh;

    switch (mode)
    {
    case 0:  // read&clear
        blink_short_refresh = 0;
        break;
    case 1:  // read&set
        blink_short_refresh = 1;
        break;
    case 2:  // read
        break;
    default:
        blink_short_refresh = 0;
        break;
    }
    return ret;
}

/*
 * @brief
 * @note
 * @param
 * @retval
 */
void FM_LCD_LL_BlinkSymbol(fm_lcd_ll_sym_t symbol, fm_lcd_ll_blink_t blink)
{
    blink_symbol[symbol] = blink;
}

/*
 * @brief	apaga todos los segmentos de la pantalla.
 * @note	ninguno
 * @param  	ninguno
 * @retval 	ninguno
 */
void FM_LCD_LL_Clear()
{
    FM_PCF8553_ClearBuffer();
    FM_PCF8553_WriteAll(PCF8553_SEGMENTS_OFF);
}

/*
 * @brief	Devuelve el tamaño de la linea que se pasa como parametro.
 * @Note 	La informacion de los limites de las lineas es responsabilidad de este modulo, pero
 * 			otros modulo pueden necesitar saber cual es el tamaño de la linea.
 * @param	Linea 1 o 2 del LCD
 * @retval 	Cantidad de caracteres en la lineas
 *
 */
uint32_t FM_LCD_LL_GetRowSize(fm_lcd_ll_row_t row)
{
    uint32_t retval = 0;
    switch (row)
    {
    case FM_LCD_LL_ROW_1:
        retval = ROW_1_SIZE;
        break;
    case FM_LCD_LL_ROW_2:
        retval = ROW_2_SIZE;
        break;
    default:
        retval = 0;
        break;
    }
    return retval;
}

/*
 * @brief	Inicialización de la pantalla LCD escribiéndola por completo y luego borrándola.
 * @Note 	ninguno
 * @param	fill: util para iniciar el LCD con todos los segmentos encendidos (0xFF) o apagados (0x00)
 * @retval 	ninguno
 *
 */
void FM_LCD_LL_Init(uint8_t segments)
{
    FM_PCF8553_Init();
    FM_PCF8553_WriteAll(segments);
}

/*
 * @brief   Escribe en pantalla un caracter en la columna y fila elegida.
 * @note
 * @param	c, es el caracter a escribir.
 * 			col, se tienen 8 columna en la linea 1y 7 columnas en la lineas 2.
 * 			row, linea 1 o linea dos, caracteres de 8 segmentos.
 * @retval 	ninguno
 */
void FM_LCD_LL_PutChar(char c, uint8_t col, fm_lcd_ll_row_t row)
{

    // Control automático de parpadeo
    if (c == '.') // si hay que imprimir un punto el siguiente algoritmo no se usa.
    {
        // No se implementa parpadeo para el punto
    }
    else
    {
        switch (blink_number[row][col])
        {
        case FM_LCD_LL_BLINK_OFF:  // Sin parpadeo.
            break;
        case FM_LCD_LL_BLINK_ON_ON: // Parpadeo encendido, se visualizar el caracter.
            blink_number[row][col] = FM_LCD_LL_BLINK_ON_OFF;
            break;
        case FM_LCD_LL_BLINK_ON_OFF:
            blink_number[row][col] = FM_LCD_LL_BLINK_ON_ON;
            c = ' '; // Parpadeo encendido, se visualiza el cursor, corresponde a caracter apagado.
            blink_short_refresh = TRUE; // El símbolo debe durar poco apagado, se necesita refresco rapido;
            break;
        default:
            FM_DEBUG_LedError(1);
            break;
        }
    }

    g_row = row;

    switch (row)
    {
    case FM_LCD_LL_ROW_1:
        g_col = col;
        if (col != (ROW_1_SIZE - 1))
        {
            if (c != '.')
            {
                WriteLine(SEG_H, 0);
            }
        }
        break;
    case FM_LCD_LL_ROW_2:
        g_col = 6 - col;  // @suppress("Avoid magic numbers")
        break;
        if (col != (ROW_2_SIZE - 1))
        {
            if (c != '.')
            {
                WriteLine(SEG_H, 0);
            }
        }
    default:
        return;
    }

    switch (c)
    {
    case 0: // Si se imprime terminador nullo ser verán  los símbolos '_' y '-' superpuestos.
        WriteLine(SEG_A, 1);
        WriteLine(SEG_B, 0);
        WriteLine(SEG_C, 0);
        WriteLine(SEG_D, 1);
        WriteLine(SEG_E, 0);
        WriteLine(SEG_F, 0);
        WriteLine(SEG_G, 0);
        break;
    case '-':  //
        WriteLine(SEG_A, 0);
        WriteLine(SEG_B, 0);
        WriteLine(SEG_C, 0);
        WriteLine(SEG_D, 1);
        WriteLine(SEG_E, 0);
        WriteLine(SEG_F, 0);
        WriteLine(SEG_G, 0);
        break;
    case ' ':
        WriteLine(SEG_A, 0);
        WriteLine(SEG_B, 0);
        WriteLine(SEG_C, 0);
        WriteLine(SEG_D, 0);
        WriteLine(SEG_E, 0);
        WriteLine(SEG_F, 0);
        WriteLine(SEG_G, 0);
        break;
    case '.':
        WriteLine(SEG_H, 1);
        break;
    case '0':
        WriteLine(SEG_A, 1);
        WriteLine(SEG_B, 1);
        WriteLine(SEG_C, 1);
        WriteLine(SEG_D, 0);
        WriteLine(SEG_E, 1);
        WriteLine(SEG_F, 1);
        WriteLine(SEG_G, 1);
        break;
    case '1':
        WriteLine(SEG_A, 0);
        WriteLine(SEG_B, 0);
        WriteLine(SEG_C, 1);
        WriteLine(SEG_D, 0);
        WriteLine(SEG_E, 0);
        WriteLine(SEG_F, 1);
        WriteLine(SEG_G, 0);
        break;
    case '2':
        WriteLine(SEG_A, 1);
        WriteLine(SEG_B, 1);
        WriteLine(SEG_C, 0);
        WriteLine(SEG_D, 1);
        WriteLine(SEG_E, 0);
        WriteLine(SEG_F, 1);
        WriteLine(SEG_G, 1);
        break;
    case '3':
        WriteLine(SEG_A, 1);
        WriteLine(SEG_B, 0);
        WriteLine(SEG_C, 1);
        WriteLine(SEG_D, 1);
        WriteLine(SEG_E, 0);
        WriteLine(SEG_F, 1);
        WriteLine(SEG_G, 1);
        break;
    case '4':
        WriteLine(SEG_A, 0);
        WriteLine(SEG_B, 0);
        WriteLine(SEG_C, 1);
        WriteLine(SEG_D, 1);
        WriteLine(SEG_E, 1);
        WriteLine(SEG_F, 1);
        WriteLine(SEG_G, 0);
        break;
    case '5':
        WriteLine(SEG_A, 1);
        WriteLine(SEG_B, 0);
        WriteLine(SEG_C, 1);
        WriteLine(SEG_D, 1);
        WriteLine(SEG_E, 1);
        WriteLine(SEG_F, 0);
        WriteLine(SEG_G, 1);
        break;
    case '6':
        WriteLine(SEG_A, 1);
        WriteLine(SEG_B, 1);
        WriteLine(SEG_C, 1);
        WriteLine(SEG_D, 1);
        WriteLine(SEG_E, 1);
        WriteLine(SEG_F, 0);
        WriteLine(SEG_G, 1);
        break;
    case '7':
        WriteLine(SEG_A, 0);
        WriteLine(SEG_B, 0);
        WriteLine(SEG_C, 1);
        WriteLine(SEG_D, 0);
        WriteLine(SEG_E, 0);
        WriteLine(SEG_F, 1);
        WriteLine(SEG_G, 1);
        break;
    case '8':
        WriteLine(SEG_A, 1);
        WriteLine(SEG_B, 1);
        WriteLine(SEG_C, 1);
        WriteLine(SEG_D, 1);
        WriteLine(SEG_E, 1);
        WriteLine(SEG_F, 1);
        WriteLine(SEG_G, 1);
        break;
    case '9':
        WriteLine(SEG_A, 0);
        WriteLine(SEG_B, 0);
        WriteLine(SEG_C, 1);
        WriteLine(SEG_D, 1);
        WriteLine(SEG_E, 1);
        WriteLine(SEG_F, 1);
        WriteLine(SEG_G, 1);
        break;
    case 'A':
        WriteLine(SEG_A, 0);
        WriteLine(SEG_B, 1);
        WriteLine(SEG_C, 1);
        WriteLine(SEG_D, 1);
        WriteLine(SEG_E, 1);
        WriteLine(SEG_F, 1);
        WriteLine(SEG_G, 1);
        break;
    case 'E':
        WriteLine(SEG_A, 1);
        WriteLine(SEG_B, 1);
        WriteLine(SEG_C, 0);
        WriteLine(SEG_D, 1);
        WriteLine(SEG_E, 1);
        WriteLine(SEG_F, 0);
        WriteLine(SEG_G, 1);
        break;
    case 'L':
        WriteLine(SEG_A, 1);
        WriteLine(SEG_B, 1);
        WriteLine(SEG_C, 0);
        WriteLine(SEG_D, 0);
        WriteLine(SEG_E, 1);
        WriteLine(SEG_F, 0);
        WriteLine(SEG_G, 0);
        break;
    case 'O':
        WriteLine(SEG_A, 1);
        WriteLine(SEG_B, 1);
        WriteLine(SEG_C, 1);
        WriteLine(SEG_D, 0);
        WriteLine(SEG_E, 1);
        WriteLine(SEG_F, 1);
        WriteLine(SEG_G, 1);
        break;
    case 'P':
        WriteLine(SEG_A, 0);
        WriteLine(SEG_B, 1);
        WriteLine(SEG_C, 0);
        WriteLine(SEG_D, 1);
        WriteLine(SEG_E, 1);
        WriteLine(SEG_F, 1);
        WriteLine(SEG_G, 1);
        break;
    case 'S':
        WriteLine(SEG_A, 1);
        WriteLine(SEG_B, 0);
        WriteLine(SEG_C, 1);
        WriteLine(SEG_D, 1);
        WriteLine(SEG_E, 1);
        WriteLine(SEG_F, 0);
        WriteLine(SEG_G, 1);
        break;
    case 'U':
        WriteLine(SEG_A, 1);
        WriteLine(SEG_B, 1);
        WriteLine(SEG_C, 1);
        WriteLine(SEG_D, 0);
        WriteLine(SEG_E, 1);
        WriteLine(SEG_F, 1);
        WriteLine(SEG_G, 0);
        break;
    case '_':
        WriteLine(SEG_A, 1);
        WriteLine(SEG_B, 0);
        WriteLine(SEG_C, 0);
        WriteLine(SEG_D, 0);
        WriteLine(SEG_E, 0);
        WriteLine(SEG_F, 0);
        WriteLine(SEG_G, 0);
        break;
    case 'b':
        WriteLine(SEG_A, 0);
        WriteLine(SEG_B, 1);
        WriteLine(SEG_C, 0);
        WriteLine(SEG_D, 1);
        WriteLine(SEG_E, 1);
        WriteLine(SEG_F, 1);
        WriteLine(SEG_G, 1);
        WriteLine(SEG_H, 0);
        break;
    default:
        break;
    }
}

/*
 * @brief	En envía el contenido del buffer a la pantalla.
 * @nota	ninguno
 * @param 	ninguno
 * @retval 	ninguno
 *
 */
void FM_LCD_LL_Refresh()
{
    FM_PCF8553_Refresh();
}

/*
 * @brief	Función que escribe un símbolo en específico en la pantalla.
 * @note
 * @param 	Enumeraciones symbols_t y blink_t de lcd.h
 * @retval 	None
 *
 */
void FM_LCD_LL_SymbolWrite(fm_lcd_ll_sym_t symbol, uint8_t state)
{

    switch (blink_symbol[symbol])
    {
    case FM_LCD_LL_BLINK_OFF:
        break;
    case FM_LCD_LL_BLINK_ON_ON:
        blink_symbol[symbol] = FM_LCD_LL_BLINK_ON_OFF;
        break;
    case FM_LCD_LL_BLINK_ON_OFF:
        blink_short_refresh = TRUE;
        blink_symbol[symbol] = FM_LCD_LL_BLINK_ON_ON;
        state = 0;
        break;
    default:
        FM_DEBUG_LedError(1);
        break;
    }

    switch (symbol)
    {
    case FM_LCD_LL_SYM_POINT:
        if (state)
        {
            pcf8553_ram_map[REG_7] |= (1 << BIT_4);
        }
        else
        {
            pcf8553_ram_map[REG_7] &= ~(1 << BIT_4);
        }
        break;
    case FM_LCD_LL_SYM_BATTERY:
        if (state)
        {
            pcf8553_ram_map[REG_7] |= (1 << BIT_5);
        }
        else
        {
            pcf8553_ram_map[REG_7] &= ~(1 << BIT_5);
        }
        break;
    case FM_LCD_LL_SYM_POWER:
        if (state)
        {
            pcf8553_ram_map[REG_2] |= (1 << BIT_5);
        }
        else
        {
            pcf8553_ram_map[REG_2] &= ~(1 << BIT_5);
        }
        break;
    case FM_LCD_LL_SYM_RATE:
        if (state)
        {
            pcf8553_ram_map[REG_17] |= (1 << BIT_5);
        }
        else
        {
            pcf8553_ram_map[REG_17] &= ~(1 << BIT_5);
        }
        break;
    case FM_LCD_LL_SYM_E:
        if (state)
        {
            pcf8553_ram_map[REG_2] |= (1 << BIT_4);
        }
        else
        {
            pcf8553_ram_map[REG_2] &= ~(1 << BIT_4);
        }
        break;
    case FM_LCD_LL_SYM_BATCH:
        if (state)
        {
            pcf8553_ram_map[REG_12] |= (1 << BIT_5);
        }
        else
        {
            pcf8553_ram_map[REG_12] &= ~(1 << BIT_5);
        }
        break;
    case FM_LCD_LL_SYM_ACM_1:
        // implementar
        break;
    case FM_LCD_LL_SYM_TTL:
        if (state)
        {
            pcf8553_ram_map[REG_17] |= (1 << BIT_4);
        }
        else
        {
            pcf8553_ram_map[REG_17] &= ~(1 << BIT_4);
        }
        break;
    case FM_LCD_LL_SYM_BACKSLASH:
        if (state)
        {
            pcf8553_ram_map[REG_10] |= (1 << BIT_1);
        }
        else
        {
            pcf8553_ram_map[REG_10] &= ~(1 << BIT_1);
        }
        break;
    case FM_LCD_LL_SYM_ACM_2:
        if (state)
        {
            pcf8553_ram_map[REG_12] |= (1 << BIT_4);
        }
        else
        {
            pcf8553_ram_map[REG_12] &= ~(1 << BIT_4);
        }
        break;
    case FM_LCD_LL_SYM_S:
        if (state)
        {
            pcf8553_ram_map[REG_10] |= (1 << BIT_5);
        }
        else
        {
            pcf8553_ram_map[REG_10] &= ~(1 << BIT_5);
        }

        break;
    case FM_LCD_LL_SYM_M:
        if (state)
        {
            pcf8553_ram_map[REG_9] |= (1 << BIT_7);
        }
        else
        {
            pcf8553_ram_map[REG_9] &= ~(1 << BIT_7);
        }
        break;
    case FM_LCD_LL_SYM_H:
        if (state)
        {
            pcf8553_ram_map[REG_5] |= (1 << BIT_6);
        }
        else
        {
            pcf8553_ram_map[REG_5] &= ~(1 << BIT_6);
        }
        break;
    case FM_LCD_LL_SYM_D:
        if (state)
        {
            pcf8553_ram_map[REG_9] |= (1 << BIT_5);
        }
        else
        {
            pcf8553_ram_map[REG_9] &= ~(1 << BIT_5);
        }
        break;

    case DOT_ROW_1_DECI:
        if (state)
        {
            pcf8553_ram_map[REG_7] |= (1 << BIT_7);
        }
        else
        {
            pcf8553_ram_map[REG_7] &= ~(1 << BIT_7);
        }
        break;
    case DOT_ROW_1_CENTI:
        if (state)
        {
            pcf8553_ram_map[REG_8] |= (1 << BIT_1);
        }
        else
        {
            pcf8553_ram_map[REG_8] &= ~(1 << BIT_1);
        }
        break;
    case DOT_ROW_1_MILI:
        if (state)
        {
            pcf8553_ram_map[REG_8] |= (1 << BIT_3);
        }
        else
        {
            pcf8553_ram_map[REG_8] &= ~(1 << BIT_3);
        }

        break;
    case DOT_ROW_1_MICRO:
        if (state)
        {
            pcf8553_ram_map[REG_8] |= (1 << BIT_5);
        }
        else
        {
            pcf8553_ram_map[REG_8] &= ~(1 << BIT_5);
        }

        break;
    case DOT_ROW_1_NANO:
        if (state)
        {
            pcf8553_ram_map[REG_8] |= (1 << BIT_7);
        }
        else
        {
            pcf8553_ram_map[REG_8] &= ~(1 << BIT_7);
        }

        break;
    case DOT_ROW_1_PICO:
        if (state)
        {
            pcf8553_ram_map[REG_9] |= (1 << BIT_1);
        }
        else
        {
            pcf8553_ram_map[REG_9] &= ~(1 << BIT_1);
        }

        break;
    case DOT_ROW_1_FEMTO:
        if (state)
        {
            pcf8553_ram_map[REG_9] |= (1 << BIT_3);
        }
        else
        {
            pcf8553_ram_map[REG_9] &= ~(1 << BIT_3);
        }

        break;
    case DOT_ROW_2_DECI:
        if (state)
        {
            pcf8553_ram_map[REG_7] |= (1 << BIT_2);
        }
        else
        {
            pcf8553_ram_map[REG_7] &= ~(1 << BIT_2);
        }

        break;
    case DOT_ROW_2_CENTI:
        if (state)
        {
            pcf8553_ram_map[REG_7] |= (1 << BIT_0);
        }
        else
        {
            pcf8553_ram_map[REG_7] &= ~(1 << BIT_0);
        }

        break;
    case DOT_ROW_2_MILI:
        if (state)
        {
            pcf8553_ram_map[REG_6] |= (1 << BIT_6);
        }
        else
        {
            pcf8553_ram_map[REG_6] &= ~(1 << BIT_6);
        }

        break;
    case DOT_ROW_2_MICRO:
        if (state)
        {
            pcf8553_ram_map[REG_6] |= (1 << BIT_4);
        }
        else
        {
            pcf8553_ram_map[REG_6] &= ~(1 << BIT_4);
        }

        break;
    case DOT_ROW_2_NANO:
        if (state)
        {
            pcf8553_ram_map[REG_6] |= (1 << BIT_2);
        }
        else
        {
            pcf8553_ram_map[REG_6] &= ~(1 << BIT_2);
        }

        break;
    case DOT_ROW_2_PICO:
        if (state)
        {
            pcf8553_ram_map[REG_6] |= (1 << BIT_0);
        }
        else
        {
            pcf8553_ram_map[REG_6] &= ~(1 << BIT_0);
        }

        break;

    case FM_LCD_LL_SYM_END:
        // implementar
        break;
    default:
        break;
    }
}

/*
 * @brief 	Función que imprime la unidad de volumen a utilizar.
 * @note
 * @param 	Unidad a imprimir y velocidad de parpadeo.
 * @retval 	None
 *
 */
void FM_LCD_LL_PutChar_1(char ascii_char)
{

    // borro
    pcf8553_ram_map[REG_0] &= ~(1 << BIT_2);  // AMARILLO S2
    pcf8553_ram_map[REG_0] &= ~(1 << BIT_3);  // AMARILLO S3
    pcf8553_ram_map[REG_0] &= ~(1 << BIT_4);  // AMARILLO S4
    pcf8553_ram_map[REG_0] &= ~(1 << BIT_5);  // AMARILLO S5
    pcf8553_ram_map[REG_5] &= ~(1 << BIT_2);  // VERDE S2
    pcf8553_ram_map[REG_5] &= ~(1 << BIT_4);  // VERDE S4
    pcf8553_ram_map[REG_5] &= ~(1 << BIT_5);  // VERDE S5
    pcf8553_ram_map[REG_10] &= ~(1 << BIT_2);  // ROJO S2
    pcf8553_ram_map[REG_10] &= ~(1 << BIT_3);  // ROJO S3
    pcf8553_ram_map[REG_10] &= ~(1 << BIT_4);  // ROJO S4
    pcf8553_ram_map[REG_15] &= ~(1 << BIT_2);  // AZUL S2
    pcf8553_ram_map[REG_15] &= ~(1 << BIT_3);  // AZUL S3
    pcf8553_ram_map[REG_15] &= ~(1 << BIT_4);  // AZUL S4
    pcf8553_ram_map[REG_15] &= ~(1 << BIT_5);  // AZUL S5

    switch (blink_char_1)
    {
    case 0:
        break;
    case FM_LCD_LL_BLINK_ON_ON:
        blink_char_1 = FM_LCD_LL_BLINK_ON_OFF;
        break;
    case FM_LCD_LL_BLINK_ON_OFF:
        blink_char_1 = FM_LCD_LL_BLINK_ON_ON;
        ascii_char = ' ';
        blink_short_refresh = TRUE; // El símbolo debe durar poco apagado, se necesita refresco rapido;
        return;
        break;
    }

    switch (ascii_char)
    {

    case 0:
        break;
    case ' ':
        break;
    case 1:  // Enciendo todos los segmentos.
        pcf8553_ram_map[REG_0] |= (1 << BIT_2);  // AMARILLO S2
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);  // AMARILLO S3
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);  // AMARILLO S4
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);  // AMARILLO S5
        pcf8553_ram_map[REG_5] |= (1 << BIT_2);  // VERDE S2
        pcf8553_ram_map[REG_5] |= (1 << BIT_4);  // VERDE S4
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);  // VERDE S5
        pcf8553_ram_map[REG_10] |= (1 << BIT_2);  // ROJO S2
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);  // ROJO S3
        pcf8553_ram_map[REG_10] |= (1 << BIT_4);  // ROJO S4
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);  // AZUL S2
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);  // AZUL S3
        pcf8553_ram_map[REG_15] |= (1 << BIT_4);  // AZUL S4
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);  // AZUL S5
    case '0':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);  // AMARILLO S3
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);  // AMARILLO S5
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);  // VERDE S5
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);  // ROJO S3
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);  // AZUL S3
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);  // AZUL S5
        break;
    case '1':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);  // AMARILLO S3
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);  // AZUL S3
        break;
    case '2':
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);  // AMARILLO S4
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);  // AMARILLO S5
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);  // VERDE S5
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);  // ROJO S3
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);  // AZUL S2
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);  // AZUL S3
        break;
    case '3':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);  // AMARILLO S3
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);  // AMARILLO S4
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);  // VERDE S5
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);  // ROJO S3
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);  // AZUL S2
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);  // AZUL S3
        break;
    case '4':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);  // AMARILLO S3
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);  // AMARILLO S4
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);  // AZUL S2
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);  // AZUL S3
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);  // AZUL S5
        break;
    case '5':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);  // AMARILLO S3
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);  // VERDE S5
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);  // ROJO S3
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);  // AZUL S2
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);  // AZUL S5
        break;
    case '6':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);  // AMARILLO S3
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);  // AMARILLO S4
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);  // AMARILLO S5
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);  // VERDE S5
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);  // ROJO S3
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);  // AZUL S2
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);  // AZUL S5
        break;
    case '7':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);  // AMARILLO S3
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);  // ROJO S3
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);  // AZUL S3
        break;
    case '8':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);  // AMARILLO S3
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);  // AMARILLO S4
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);  // AMARILLO S5
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);  // VERDE S5
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);  // ROJO S3
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);  // AZUL S2
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);  // AZUL S3
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);  // AZUL S5
        break;
    case '9':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);  // AMARILLO S3
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);  // AMARILLO S4
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);  // ROJO S3
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);  // AZUL S2
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);  // AZUL S3
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);  // AZUL S5
        break;
    case 'A':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'B':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);
        pcf8553_ram_map[REG_5] |= (1 << BIT_2);
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_10] |= (1 << BIT_4);
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);
        break;
    case 'C':
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'D':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);
        pcf8553_ram_map[REG_5] |= (1 << BIT_2);
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_10] |= (1 << BIT_4);
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);
        break;
    case 'E':
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);
        break;
    case 'F':
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'G':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);
        break;
    case 'H':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'I':
        pcf8553_ram_map[REG_5] |= (1 << BIT_2);
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_10] |= (1 << BIT_4);
        break;
    case 'J':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);
        break;
    case 'K':
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_5] |= (1 << BIT_4);
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);
        pcf8553_ram_map[REG_15] |= (1 << BIT_4);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'L':
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'M':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_10] |= (1 << BIT_2);
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_4);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'N':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_5] |= (1 << BIT_4);
        pcf8553_ram_map[REG_10] |= (1 << BIT_2);
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'O':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'P':
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'Q':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_5] |= (1 << BIT_4);
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'R':
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_5] |= (1 << BIT_4);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'S':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'T':
        pcf8553_ram_map[REG_5] |= (1 << BIT_2);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_10] |= (1 << BIT_4);
        break;
    case 'U':
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'V':
        pcf8553_ram_map[REG_0] |= (1 << BIT_2);
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_15] |= (1 << BIT_4);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'W':
        pcf8553_ram_map[REG_0] |= (1 << BIT_2);
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);
        pcf8553_ram_map[REG_5] |= (1 << BIT_4);
        pcf8553_ram_map[REG_15] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);
        break;
    case 'X':
        pcf8553_ram_map[REG_0] |= (1 << BIT_2);
        pcf8553_ram_map[REG_5] |= (1 << BIT_4);
        pcf8553_ram_map[REG_10] |= (1 << BIT_2);
        pcf8553_ram_map[REG_5] |= (1 << BIT_4);
        pcf8553_ram_map[REG_15] |= (1 << BIT_4);
        break;
    case 'Y':
        pcf8553_ram_map[REG_5] |= (1 << BIT_2);
        pcf8553_ram_map[REG_10] |= (1 << BIT_2);
        pcf8553_ram_map[REG_15] |= (1 << BIT_4);
        break;
    case 'Z':
        pcf8553_ram_map[REG_0] |= (1 << BIT_2);
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);
        pcf8553_ram_map[REG_10] |= (1 << BIT_3);
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);
        pcf8553_ram_map[REG_15] |= (1 << BIT_4);
        break;
    case 'a':
        //pcf8553_ram_map[REG_0] |= (1 << BIT_2);  // AMARILLO S2
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);  // AMARILLO S3
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);  // AMARILLO S4
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);  // AMARILLO S5
        //pcf8553_ram_map[REG_5] |= (1 << BIT_2);  // VERDE S2
        //pcf8553_ram_map[REG_5] |= (1 << BIT_4);  // VERDE S4
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);  // VERDE S5
        //pcf8553_ram_map[REG_10] |= (1 << BIT_2);  // ROJO S2
        //pcf8553_ram_map[REG_10] |= (1 << BIT_3);  // ROJO S3
        //pcf8553_ram_map[REG_10] |= (1 << BIT_4);  // ROJO S4
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);  // AZUL S2
        //pcf8553_ram_map[REG_15] |= (1 << BIT_3);  // AZUL S3
        //pcf8553_ram_map[REG_15] |= (1 << BIT_4);  // AZUL S4
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);  // AZUL S5
           break;
	case 'b':
        //pcf8553_ram_map[REG_0] |= (1 << BIT_2);  // AMARILLO S2
        pcf8553_ram_map[REG_0] |= (1 << BIT_3);  // AMARILLO S3
        pcf8553_ram_map[REG_0] |= (1 << BIT_4);  // AMARILLO S4
        pcf8553_ram_map[REG_0] |= (1 << BIT_5);  // AMARILLO S5
        //pcf8553_ram_map[REG_5] |= (1 << BIT_2);  // VERDE S2
        //pcf8553_ram_map[REG_5] |= (1 << BIT_4);  // VERDE S4
        pcf8553_ram_map[REG_5] |= (1 << BIT_5);  // VERDE S5
        //pcf8553_ram_map[REG_10] |= (1 << BIT_2);  // ROJO S2
        //pcf8553_ram_map[REG_10] |= (1 << BIT_3);  // ROJO S3
        //pcf8553_ram_map[REG_10] |= (1 << BIT_4);  // ROJO S4
        pcf8553_ram_map[REG_15] |= (1 << BIT_2);  // AZUL S2
        //pcf8553_ram_map[REG_15] |= (1 << BIT_3);  // AZUL S3
        //pcf8553_ram_map[REG_15] |= (1 << BIT_4);  // AZUL S4
        pcf8553_ram_map[REG_15] |= (1 << BIT_5);  // AZUL S5
	   break;
	case 'c':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 'd':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_3);
	   break;
	case 'e':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_2);
	   break;
	case 'f':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 'g':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_2);
	   break;
	case 'h':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 'i':
	   pcf8553_ram_map[REG_5] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_4);
	   break;
	case 'j':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_3);
	   break;
	case 'k':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 'l':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 'm':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 'n':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 'o':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 'p':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 'q':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 'r':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 's':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 't':
	   pcf8553_ram_map[REG_5] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_4);
	   break;
	case 'u':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 'v':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 'w':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_5);
	   break;
	case 'x':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_4);
	   break;
	case 'y':
	   pcf8553_ram_map[REG_5] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_4);
	   break;
	case 'z':
	   pcf8553_ram_map[REG_0] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_0] |= (1 << BIT_4);
	   pcf8553_ram_map[REG_5] |= (1 << BIT_5);
	   pcf8553_ram_map[REG_10] |= (1 << BIT_3);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_2);
	   pcf8553_ram_map[REG_15] |= (1 << BIT_4);
	   break;
   case 176:  // Grados centidrados
       break;
    default:
        break;
    }
}

void FM_LCD_LL_PutChar_2(char ascii_char)
{

// Borro caracter antes de escribir uno nuevo
    pcf8553_ram_map[REG_0] &= ~(1 << BIT_0);  // AMARILLO S0
    pcf8553_ram_map[REG_0] &= ~(1 << BIT_1);  // AMARILLO S1
    pcf8553_ram_map[REG_4] &= ~(1 << BIT_6);  // AMARILLO S38
    pcf8553_ram_map[REG_4] &= ~(1 << BIT_7);  // AMARILLO S39
    pcf8553_ram_map[REG_5] &= ~(1 << BIT_0);  // VERDE S0
    pcf8553_ram_map[REG_5] &= ~(1 << BIT_1);  // VERDE S1
    pcf8553_ram_map[REG_9] &= ~(1 << BIT_6);  // VERDE S38
    pcf8553_ram_map[REG_10] &= ~(1 << BIT_0);  // ROJO S0
    pcf8553_ram_map[REG_14] &= ~(1 << BIT_6);  // ROJO 38
    pcf8553_ram_map[REG_14] &= ~(1 << BIT_7);  // ROJO S39
    pcf8553_ram_map[REG_15] &= ~(1 << BIT_0);  // AZUL S0
    pcf8553_ram_map[REG_15] &= ~(1 << BIT_1);  // AZUL S1
    pcf8553_ram_map[REG_19] &= ~(1 << BIT_6);  // AZUL S38
    pcf8553_ram_map[REG_19] &= ~(1 << BIT_7);  // AZUL S39

    switch (blink_char_2)
    {
    case FM_LCD_LL_BLINK_OFF:
        break;
    case FM_LCD_LL_BLINK_ON_ON:
        blink_char_2 = FM_LCD_LL_BLINK_ON_OFF;
        break;
    case FM_LCD_LL_BLINK_ON_OFF:
        blink_char_2 = FM_LCD_LL_BLINK_ON_ON;
        return;
        break;
    }

    switch (ascii_char)
    {
    case 0: // Con la sigueinte macro se construye cualquier cararter de 14-segmentos digito 2
        break;
    case '0':
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case '1':
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case '2':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);  // AMARILLO S0
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case '3':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);  // AMARILLO S0
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case '4':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);  // AMARILLO S0
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case '5':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);  // AMARILLO S0
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38
        break;
    case '6':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);  // AMARILLO S0
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38
        break;
    case '7':
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case '8':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);  // AMARILLO S0
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case '9':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);  // AMARILLO S0
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case 'A':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);
        break;
    case 'B':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);  // AMARILLO S0
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_9] |= (1 << BIT_6);  // VERDE S38
        pcf8553_ram_map[REG_10] |= (1 << BIT_0);  // ROJO S0
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case 'C':
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);
        break;
    case 'D':
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_9] |= (1 << BIT_6);  // VERDE S38
        pcf8553_ram_map[REG_10] |= (1 << BIT_0);  // ROJO S0
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case 'E':
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38

        break;
    case 'F':
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38

        break;
    case 'G':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);  // AMARILLO S0
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38

        break;
    case 'H':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);  // AMARILLO S0
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39

        break;
    case 'I':
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_9] |= (1 << BIT_6);  // VERDE S38
        pcf8553_ram_map[REG_10] |= (1 << BIT_0);  // ROJO S0
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        break;
    case 'J':
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case 'K':
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_5] |= (1 << BIT_0);  // VERDE S0
        pcf8553_ram_map[REG_15] |= (1 << BIT_0);  // AZUL S0
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38

        break;
    case 'L':
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        break;
    case 'M':
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_14] |= (1 << BIT_6);  // ROJO 38
        pcf8553_ram_map[REG_15] |= (1 << BIT_0);  // AZUL S0
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case 'N':
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_0);  // VERDE S0
        pcf8553_ram_map[REG_14] |= (1 << BIT_6);  // ROJO 38
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case 'O':
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case 'P':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);  // AMARILLO S0
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case 'Q':
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_0);  // VERDE S0
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case 'R':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);  // AMARILLO S0
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_5] |= (1 << BIT_0);  // VERDE S0
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case 'S':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);  // AMARILLO S0
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38
        break;
    case 'T':
        pcf8553_ram_map[REG_9] |= (1 << BIT_6);  // VERDE S38
        pcf8553_ram_map[REG_10] |= (1 << BIT_0);  // ROJO S0
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        break;
    case 'U':
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case 'V':
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_4] |= (1 << BIT_6);  // AMARILLO S38
        pcf8553_ram_map[REG_15] |= (1 << BIT_0);  // AZUL S0
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        break;
    case 'W':
        pcf8553_ram_map[REG_0] |= (1 << BIT_1);  // AMARILLO S1
        pcf8553_ram_map[REG_4] |= (1 << BIT_6);  // AMARILLO S38
        pcf8553_ram_map[REG_4] |= (1 << BIT_7);  // AMARILLO S39
        pcf8553_ram_map[REG_5] |= (1 << BIT_0);  // VERDE S0
        pcf8553_ram_map[REG_15] |= (1 << BIT_1);  // AZUL S1
        pcf8553_ram_map[REG_19] |= (1 << BIT_7);  // AZUL S39
        break;
    case 'X':
        pcf8553_ram_map[REG_4] |= (1 << BIT_6);  // AMARILLO S38
        pcf8553_ram_map[REG_5] |= (1 << BIT_0);  // VERDE S0
        pcf8553_ram_map[REG_14] |= (1 << BIT_6);  // ROJO 38
        pcf8553_ram_map[REG_15] |= (1 << BIT_0);  // AZUL S0
        break;
    case 'Y':
        pcf8553_ram_map[REG_9] |= (1 << BIT_6);  // VERDE S38
        pcf8553_ram_map[REG_14] |= (1 << BIT_6);  // ROJO 38
        pcf8553_ram_map[REG_15] |= (1 << BIT_0);  // AZUL S0
        break;
    case 'Z':
        pcf8553_ram_map[REG_0] |= (1 << BIT_0);  // AMARILLO S0
        pcf8553_ram_map[REG_4] |= (1 << BIT_6);  // AMARILLO S38
        pcf8553_ram_map[REG_5] |= (1 << BIT_1);  // VERDE S1
        pcf8553_ram_map[REG_14] |= (1 << BIT_7);  // ROJO S39
        pcf8553_ram_map[REG_15] |= (1 << BIT_0);  // AZUL S0
        pcf8553_ram_map[REG_19] |= (1 << BIT_6);  // AZUL S38
        break;
    case 176:  // Grados centidrados
        //pcf8553_ram_map[REG_0] |= (1 << BIT_0); // AMARILLO S0
        //pcf8553_ram_map[REG_0] |= (1 << BIT_1); // AMARILLO S1
        //pcf8553_ram_map[REG_4] |= (1 << BIT_6); // AMARILLO S38
        //pcf8553_ram_map[REG_4] |= (1 << BIT_7); // AMARILLO S39
        //pcf8553_ram_map[REG_5] |= (1 << BIT_0); // VERDE S0
        //pcf8553_ram_map[REG_5] |= (1 << BIT_1); // VERDE S1
        //pcf8553_ram_map[REG_9] |= (1 << BIT_6); // VERDE S38
        //pcf8553_ram_map[REG_10] |= (1 << BIT_0); // ROJO S0
        //pcf8553_ram_map[REG_14] |= (1 << BIT_6); // ROJO 38
        //pcf8553_ram_map[REG_14] |= (1 << BIT_7); // ROJO S39
        //pcf8553_ram_map[REG_15] |= (1 << BIT_1); // AZUL S1
        //pcf8553_ram_map[REG_19] |= (1 << BIT_6); // AZUL S38
        //pcf8553_ram_map[REG_19] |= (1 << BIT_7); // AZUL S39
        break;
    default:
        break;
    }
}

// Private function bodies.

/*
 * @brief
 * @nota	Esta funcion ajusta el estado de un segmento, encendido o apagado,
 * por cada llamada. El caracter a manipular, linea 1 o linea 2 y la posicion
 * del cursor, son informaciones guardadas en dos variables globales, g_cursor
 * y g_line. Con el mapeo de los segmentos, y sabiendo que segmento se quiere
 * manipular, notar que para el primer caracter de cada linea se tiene una matriz
 * con la posicion en el mapa de memoria para el pcf8553, con lo cual no se
 * requiere calculo, el calculo es de utilidad para los caracteres que no ocupan
 * la primera posicion de cada linea.
 * @param	seg:
 * @param 	data:
 * @retval	ninguno
 *
 */
void WriteLine(uint8_t seg, uint8_t data)
{
    uint8_t reg = 0;
    uint8_t pos = 0;

    /*
     *
     *
     */
    switch (g_row)
    {
    case FM_LCD_LL_ROW_1:
        pos = octal_1[seg].pos;
        pos += g_col * 2;
        reg = (pos / 8) + octal_1[seg].reg;  // @suppress("Avoid magic numbers")
        pos = pos % 8;  // @suppress("Avoid magic numbers")
        break;
    case FM_LCD_LL_ROW_2:
        pos = octal_2[seg].pos;
        pos += g_col * 2;
        reg = (pos / 8) + octal_2[seg].reg;  // @suppress("Avoid magic numbers")
        pos = pos % 8;  // @suppress("Avoid magic numbers")
        break;
    default:
        break;
    }

    if (data)
    {
        pcf8553_ram_map[reg] |= 1 << pos;
    }
    else
    {
        pcf8553_ram_map[reg] &= ~(1 << pos);
    }
}

/*** end of file ***/
