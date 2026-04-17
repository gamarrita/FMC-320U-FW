/**
 * @file    fm_lcd_ll.c
 * @brief   LCD glass mapping layer for the FMC segmented display.
 *
 * The mappings in this file follow `lcd_true_source.yaml`.
 */

#include "fm_lcd_ll.h"

#include <stddef.h>

#include "pcf8553/fm_pcf8553.h"

/* =========================== Private Types ============================== */
typedef struct
{
    uint8_t reg;
    uint8_t bit;
} fm_lcd_ll_addr_t;

typedef enum
{
    FM_LCD_LL_SEG_A = 0,
    FM_LCD_LL_SEG_B,
    FM_LCD_LL_SEG_C,
    FM_LCD_LL_SEG_D,
    FM_LCD_LL_SEG_E,
    FM_LCD_LL_SEG_F,
    FM_LCD_LL_SEG_G,
    FM_LCD_LL_SEG_COUNT
} fm_lcd_ll_segment_t;

/* =========================== Private Macros ============================= */
#define FM_LCD_LL_SEG_A_MASK    (1U << 0)
#define FM_LCD_LL_SEG_B_MASK    (1U << 1)
#define FM_LCD_LL_SEG_C_MASK    (1U << 2)
#define FM_LCD_LL_SEG_D_MASK    (1U << 3)
#define FM_LCD_LL_SEG_E_MASK    (1U << 4)
#define FM_LCD_LL_SEG_F_MASK    (1U << 5)
#define FM_LCD_LL_SEG_G_MASK    (1U << 6)

#define FM_LCD_LL_DIGIT_STRIDE_BITS    2U

/* =========================== Private Constants ========================== */
static const fm_lcd_ll_addr_t g_fm_lcd_ll_row_1_base_segments_[FM_LCD_LL_SEG_COUNT] =
{
    { .reg = 7U,  .bit = 6U },
    { .reg = 2U,  .bit = 6U },
    { .reg = 2U,  .bit = 7U },
    { .reg = 17U, .bit = 6U },
    { .reg = 12U, .bit = 6U },
    { .reg = 17U, .bit = 7U },
    { .reg = 12U, .bit = 7U }
};

static const fm_lcd_ll_addr_t g_fm_lcd_ll_row_2_base_segments_[FM_LCD_LL_SEG_COUNT] =
{
    { .reg = 5U,  .bit = 7U },
    { .reg = 0U,  .bit = 7U },
    { .reg = 0U,  .bit = 6U },
    { .reg = 15U, .bit = 7U },
    { .reg = 10U, .bit = 7U },
    { .reg = 15U, .bit = 6U },
    { .reg = 10U, .bit = 6U }
};

static const fm_lcd_ll_addr_t g_fm_lcd_ll_row_1_decimal_points_[FM_LCD_LL_ROW_1_COLS] =
{
    { .reg = 7U, .bit = 7U },
    { .reg = 8U, .bit = 1U },
    { .reg = 8U, .bit = 3U },
    { .reg = 8U, .bit = 5U },
    { .reg = 8U, .bit = 7U },
    { .reg = 9U, .bit = 1U },
    { .reg = 9U, .bit = 3U },
    { .reg = 0U, .bit = 0U }
};

static const fm_lcd_ll_addr_t g_fm_lcd_ll_row_2_decimal_points_[FM_LCD_LL_ROW_2_COLS] =
{
    { .reg = 7U, .bit = 2U },
    { .reg = 7U, .bit = 0U },
    { .reg = 6U, .bit = 6U },
    { .reg = 6U, .bit = 4U },
    { .reg = 6U, .bit = 2U },
    { .reg = 6U, .bit = 0U },
    { .reg = 0U, .bit = 0U }
};

/* =========================== Private Prototypes ========================= */
static uint8_t fm_lcd_ll_encode_char_(char p_char);
static bool fm_lcd_ll_get_decimal_addr_(uint8_t p_col,
                                        fm_lcd_ll_row_t p_row,
                                        uint8_t *p_reg,
                                        uint8_t *p_bit);
static bool fm_lcd_ll_get_numeric_addr_(fm_lcd_ll_row_t p_row,
                                        uint8_t p_col,
                                        fm_lcd_ll_segment_t p_segment,
                                        uint8_t *p_reg,
                                        uint8_t *p_bit);
static uint8_t fm_lcd_ll_get_row_size_(fm_lcd_ll_row_t p_row);
static bool fm_lcd_ll_get_symbol_addr_(fm_lcd_ll_symbol_t p_symbol,
                                       uint8_t *p_reg,
                                       uint8_t *p_bit);
static void fm_lcd_ll_set_bit_(uint8_t p_reg, uint8_t p_bit, bool p_on);
static void fm_lcd_ll_write_pattern_(fm_lcd_ll_row_t p_row, uint8_t p_col, uint8_t p_pattern);

/* =========================== Private Bodies ============================= */
static uint8_t fm_lcd_ll_encode_char_(char p_char)
{
    switch (p_char)
    {
    case ' ':
        return 0U;
    case '-':
        return FM_LCD_LL_SEG_D_MASK;
    case '0':
        return FM_LCD_LL_SEG_A_MASK | FM_LCD_LL_SEG_B_MASK |
               FM_LCD_LL_SEG_C_MASK | FM_LCD_LL_SEG_E_MASK |
               FM_LCD_LL_SEG_F_MASK | FM_LCD_LL_SEG_G_MASK;
    case '1':
        return FM_LCD_LL_SEG_C_MASK | FM_LCD_LL_SEG_F_MASK;
    case '2':
        return FM_LCD_LL_SEG_A_MASK | FM_LCD_LL_SEG_B_MASK |
               FM_LCD_LL_SEG_D_MASK | FM_LCD_LL_SEG_F_MASK |
               FM_LCD_LL_SEG_G_MASK;
    case '3':
        return FM_LCD_LL_SEG_A_MASK | FM_LCD_LL_SEG_C_MASK |
               FM_LCD_LL_SEG_D_MASK | FM_LCD_LL_SEG_F_MASK |
               FM_LCD_LL_SEG_G_MASK;
    case '4':
        return FM_LCD_LL_SEG_C_MASK | FM_LCD_LL_SEG_D_MASK |
               FM_LCD_LL_SEG_E_MASK | FM_LCD_LL_SEG_F_MASK;
    case '5':
        return FM_LCD_LL_SEG_A_MASK | FM_LCD_LL_SEG_C_MASK |
               FM_LCD_LL_SEG_D_MASK | FM_LCD_LL_SEG_E_MASK |
               FM_LCD_LL_SEG_G_MASK;
    case '6':
        return FM_LCD_LL_SEG_A_MASK | FM_LCD_LL_SEG_B_MASK |
               FM_LCD_LL_SEG_C_MASK | FM_LCD_LL_SEG_D_MASK |
               FM_LCD_LL_SEG_E_MASK | FM_LCD_LL_SEG_G_MASK;
    case '7':
        return FM_LCD_LL_SEG_C_MASK | FM_LCD_LL_SEG_F_MASK |
               FM_LCD_LL_SEG_G_MASK;
    case '8':
        return FM_LCD_LL_SEG_A_MASK | FM_LCD_LL_SEG_B_MASK |
               FM_LCD_LL_SEG_C_MASK | FM_LCD_LL_SEG_D_MASK |
               FM_LCD_LL_SEG_E_MASK | FM_LCD_LL_SEG_F_MASK |
               FM_LCD_LL_SEG_G_MASK;
    case '9':
        return FM_LCD_LL_SEG_C_MASK | FM_LCD_LL_SEG_D_MASK |
               FM_LCD_LL_SEG_E_MASK | FM_LCD_LL_SEG_F_MASK |
               FM_LCD_LL_SEG_G_MASK;
    case 'A':
    case 'a':
        return FM_LCD_LL_SEG_B_MASK | FM_LCD_LL_SEG_C_MASK |
               FM_LCD_LL_SEG_D_MASK | FM_LCD_LL_SEG_E_MASK |
               FM_LCD_LL_SEG_F_MASK | FM_LCD_LL_SEG_G_MASK;
    case 'E':
    case 'e':
        return FM_LCD_LL_SEG_A_MASK | FM_LCD_LL_SEG_B_MASK |
               FM_LCD_LL_SEG_D_MASK | FM_LCD_LL_SEG_E_MASK |
               FM_LCD_LL_SEG_G_MASK;
    case 'L':
    case 'l':
        return FM_LCD_LL_SEG_A_MASK | FM_LCD_LL_SEG_B_MASK |
               FM_LCD_LL_SEG_E_MASK;
    case 'O':
    case 'o':
        return FM_LCD_LL_SEG_A_MASK | FM_LCD_LL_SEG_B_MASK |
               FM_LCD_LL_SEG_C_MASK | FM_LCD_LL_SEG_E_MASK |
               FM_LCD_LL_SEG_F_MASK | FM_LCD_LL_SEG_G_MASK;
    case 'P':
    case 'p':
        return FM_LCD_LL_SEG_B_MASK | FM_LCD_LL_SEG_D_MASK |
               FM_LCD_LL_SEG_E_MASK | FM_LCD_LL_SEG_F_MASK |
               FM_LCD_LL_SEG_G_MASK;
    case 'S':
    case 's':
        return FM_LCD_LL_SEG_A_MASK | FM_LCD_LL_SEG_C_MASK |
               FM_LCD_LL_SEG_D_MASK | FM_LCD_LL_SEG_E_MASK |
               FM_LCD_LL_SEG_G_MASK;
    case 'U':
    case 'u':
        return FM_LCD_LL_SEG_A_MASK | FM_LCD_LL_SEG_B_MASK |
               FM_LCD_LL_SEG_C_MASK | FM_LCD_LL_SEG_E_MASK |
               FM_LCD_LL_SEG_F_MASK;
    case '_':
        return FM_LCD_LL_SEG_A_MASK;
    default:
        return 0U;
    }
}

static bool fm_lcd_ll_get_decimal_addr_(uint8_t p_col,
                                        fm_lcd_ll_row_t p_row,
                                        uint8_t *p_reg,
                                        uint8_t *p_bit)
{
    if ((p_reg == NULL) || (p_bit == NULL))
    {
        return false;
    }

    switch (p_row)
    {
    case FM_LCD_LL_ROW_1:
        if (p_col >= (FM_LCD_LL_ROW_1_COLS - 1U))
        {
            return false;
        }

        *p_reg = g_fm_lcd_ll_row_1_decimal_points_[p_col].reg;
        *p_bit = g_fm_lcd_ll_row_1_decimal_points_[p_col].bit;
        return true;
    case FM_LCD_LL_ROW_2:
        if (p_col >= (FM_LCD_LL_ROW_2_COLS - 1U))
        {
            return false;
        }

        *p_reg = g_fm_lcd_ll_row_2_decimal_points_[p_col].reg;
        *p_bit = g_fm_lcd_ll_row_2_decimal_points_[p_col].bit;
        return true;
    default:
        return false;
    }
}

static bool fm_lcd_ll_get_numeric_addr_(fm_lcd_ll_row_t p_row,
                                        uint8_t p_col,
                                        fm_lcd_ll_segment_t p_segment,
                                        uint8_t *p_reg,
                                        uint8_t *p_bit)
{
    const fm_lcd_ll_addr_t *p_base_segments;
    uint8_t row_size;
    uint8_t internal_index;
    uint8_t pos;

    if ((p_reg == NULL) || (p_bit == NULL) || (p_segment >= FM_LCD_LL_SEG_COUNT))
    {
        return false;
    }

    row_size = fm_lcd_ll_get_row_size_(p_row);

    if ((row_size == 0U) || (p_col >= row_size))
    {
        return false;
    }

    if (p_row == FM_LCD_LL_ROW_1)
    {
        p_base_segments = g_fm_lcd_ll_row_1_base_segments_;
        internal_index = p_col;
    }
    else
    {
        p_base_segments = g_fm_lcd_ll_row_2_base_segments_;
        internal_index = (uint8_t)((FM_LCD_LL_ROW_2_COLS - 1U) - p_col);
    }

    pos = (uint8_t)(p_base_segments[p_segment].bit +
                    (internal_index * FM_LCD_LL_DIGIT_STRIDE_BITS));

    *p_reg = (uint8_t)(p_base_segments[p_segment].reg + (pos / 8U));
    *p_bit = (uint8_t)(pos % 8U);

    return true;
}

static uint8_t fm_lcd_ll_get_row_size_(fm_lcd_ll_row_t p_row)
{
    switch (p_row)
    {
    case FM_LCD_LL_ROW_1:
        return FM_LCD_LL_ROW_1_COLS;
    case FM_LCD_LL_ROW_2:
        return FM_LCD_LL_ROW_2_COLS;
    default:
        return 0U;
    }
}

static bool fm_lcd_ll_get_symbol_addr_(fm_lcd_ll_symbol_t p_symbol,
                                       uint8_t *p_reg,
                                       uint8_t *p_bit)
{
    if ((p_reg == NULL) || (p_bit == NULL))
    {
        return false;
    }

    switch (p_symbol)
    {
    case FM_LCD_LL_SYM_POINT:
        *p_reg = 7U;
        *p_bit = 4U;
        return true;
    case FM_LCD_LL_SYM_BATTERY:
        *p_reg = 7U;
        *p_bit = 5U;
        return true;
    case FM_LCD_LL_SYM_POWER:
        *p_reg = 2U;
        *p_bit = 5U;
        return true;
    case FM_LCD_LL_SYM_RATE:
        *p_reg = 17U;
        *p_bit = 5U;
        return true;
    case FM_LCD_LL_SYM_E:
        *p_reg = 2U;
        *p_bit = 4U;
        return true;
    case FM_LCD_LL_SYM_BATCH:
        *p_reg = 12U;
        *p_bit = 5U;
        return true;
    case FM_LCD_LL_SYM_ACM_1:
        return false;
    case FM_LCD_LL_SYM_TTL:
        *p_reg = 17U;
        *p_bit = 4U;
        return true;
    case FM_LCD_LL_SYM_SLASH:
        *p_reg = 10U;
        *p_bit = 1U;
        return true;
    case FM_LCD_LL_SYM_ACM_2:
        *p_reg = 12U;
        *p_bit = 4U;
        return true;
    case FM_LCD_LL_SYM_H:
        *p_reg = 5U;
        *p_bit = 6U;
        return true;
    case FM_LCD_LL_SYM_D:
        *p_reg = 9U;
        *p_bit = 5U;
        return true;
    case FM_LCD_LL_SYM_S:
        *p_reg = 10U;
        *p_bit = 5U;
        return true;
    case FM_LCD_LL_SYM_M:
        *p_reg = 9U;
        *p_bit = 7U;
        return true;
    default:
        return false;
    }
}

static void fm_lcd_ll_set_bit_(uint8_t p_reg, uint8_t p_bit, bool p_on)
{
    if ((p_reg >= PCF8553_RAM_SIZE) || (p_bit >= 8U))
    {
        return;
    }

    if (p_on)
    {
        pcf8553_ram_map[p_reg] |= (uint8_t)(1U << p_bit);
    }
    else
    {
        pcf8553_ram_map[p_reg] &= (uint8_t) ~(1U << p_bit);
    }
}

static void fm_lcd_ll_write_pattern_(fm_lcd_ll_row_t p_row, uint8_t p_col, uint8_t p_pattern)
{
    uint8_t reg;
    uint8_t bit;
    uint8_t segment;

    for (segment = 0U; segment < FM_LCD_LL_SEG_COUNT; segment++)
    {
        if (fm_lcd_ll_get_numeric_addr_(p_row,
                                        p_col,
                                        (fm_lcd_ll_segment_t)segment,
                                        &reg,
                                        &bit))
        {
            fm_lcd_ll_set_bit_(reg, bit, ((p_pattern & (1U << segment)) != 0U));
        }
    }
}

/* =========================== Public Bodies ============================== */
void FM_LCD_LL_Clear(void)
{
    FM_LCD_LL_Fill(PCF8553_SEGMENTS_OFF);
}

void FM_LCD_LL_DecimalPointWrite(uint8_t p_col, fm_lcd_ll_row_t p_row, bool p_on)
{
    uint8_t reg;
    uint8_t bit;

    if (!fm_lcd_ll_get_decimal_addr_(p_col, p_row, &reg, &bit))
    {
        return;
    }

    fm_lcd_ll_set_bit_(reg, bit, p_on);
}

void FM_LCD_LL_Fill(uint8_t p_fill)
{
    FM_PCF8553_WriteAll(p_fill);
}

uint8_t FM_LCD_LL_GetRowSize(fm_lcd_ll_row_t p_row)
{
    return fm_lcd_ll_get_row_size_(p_row);
}

void FM_LCD_LL_Init(uint8_t p_fill)
{
    FM_PCF8553_Init();
    FM_LCD_LL_Fill(p_fill);
}

void FM_LCD_LL_PutChar(char p_char, uint8_t p_col, fm_lcd_ll_row_t p_row)
{
    if (FM_LCD_LL_GetRowSize(p_row) <= p_col)
    {
        return;
    }

    if (p_char == '.')
    {
        FM_LCD_LL_DecimalPointWrite(p_col, p_row, true);
        return;
    }

    FM_LCD_LL_DecimalPointWrite(p_col, p_row, false);
    fm_lcd_ll_write_pattern_(p_row, p_col, fm_lcd_ll_encode_char_(p_char));
}

void FM_LCD_LL_Refresh(void)
{
    FM_PCF8553_Refresh();
}

void FM_LCD_LL_SymbolWrite(fm_lcd_ll_symbol_t p_symbol, bool p_on)
{
    uint8_t reg;
    uint8_t bit;

    if (!fm_lcd_ll_get_symbol_addr_(p_symbol, &reg, &bit))
    {
        return;
    }

    fm_lcd_ll_set_bit_(reg, bit, p_on);
}

/*** end of file ***/
