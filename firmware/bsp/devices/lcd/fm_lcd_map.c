/**
 * @file    fm_lcd_map.c
 * @brief   Pure LCD glass-to-RAM mapping for the redesign path.
 *
 * This glass is not wired as "one visible character per driver byte".
 * The PCF8553 RAM bits are interleaved across multiple visible fields, so
 * one numeric digit is assembled from bits that live in different registers
 * and shared bit lanes. The mapping layer therefore works as a table-driven
 * translator from visible LCD semantics into the 20-byte controller RAM image.
 *
 * In practice this means the unusual layout is a hardware fact of the glass
 * and controller wiring, not a software preference. The code below keeps that
 * detail local by describing the glass in visible order and then translating
 * each segment into its actual RAM bit position.
 */

#include "fm_lcd_map.h"

#include <stddef.h>
#include <string.h>

/* =========================== Private Types ============================== */
typedef struct
{
    uint8_t reg;
    uint8_t bit;
} fm_lcd_map_addr_t;

typedef enum
{
    FM_LCD_MAP_SEG_A = 0,
    FM_LCD_MAP_SEG_B,
    FM_LCD_MAP_SEG_C,
    FM_LCD_MAP_SEG_D,
    FM_LCD_MAP_SEG_E,
    FM_LCD_MAP_SEG_F,
    FM_LCD_MAP_SEG_G,
    FM_LCD_MAP_SEG_COUNT
} fm_lcd_map_segment_t;

/* =========================== Private Macros ============================= */
#define FM_LCD_MAP_SEG_A_MASK            (1U << 0)
#define FM_LCD_MAP_SEG_B_MASK            (1U << 1)
#define FM_LCD_MAP_SEG_C_MASK            (1U << 2)
#define FM_LCD_MAP_SEG_D_MASK            (1U << 3)
#define FM_LCD_MAP_SEG_E_MASK            (1U << 4)
#define FM_LCD_MAP_SEG_F_MASK            (1U << 5)
#define FM_LCD_MAP_SEG_G_MASK            (1U << 6)
#define FM_LCD_MAP_DIGIT_STRIDE_BITS     2U

/* =========================== Private Constants ========================== */
/* Base segment anchors for the top visible row, stored in visible segment
 * order. Consecutive digits advance through interleaved bit lanes rather than
 * staying inside one contiguous controller byte. */
static const fm_lcd_map_addr_t g_fm_lcd_map_top_row_base_segments[FM_LCD_MAP_SEG_COUNT] =
{
    { .reg = 7U,  .bit = 6U },
    { .reg = 2U,  .bit = 6U },
    { .reg = 2U,  .bit = 7U },
    { .reg = 17U, .bit = 6U },
    { .reg = 12U, .bit = 6U },
    { .reg = 17U, .bit = 7U },
    { .reg = 12U, .bit = 7U }
};

/* Base segment anchors for the bottom visible row. This row is wired in the
 * opposite visible direction, so column-to-bit translation is mirrored later
 * by fm_lcd_map_get_numeric_addr_(). */
static const fm_lcd_map_addr_t g_fm_lcd_map_bottom_row_base_segments[FM_LCD_MAP_SEG_COUNT] =
{
    { .reg = 5U,  .bit = 7U },
    { .reg = 0U,  .bit = 7U },
    { .reg = 0U,  .bit = 6U },
    { .reg = 15U, .bit = 7U },
    { .reg = 10U, .bit = 7U },
    { .reg = 15U, .bit = 6U },
    { .reg = 10U, .bit = 6U }
};

/* Decimal points are dedicated glass elements with their own scattered RAM
 * positions. They are not derived from the 7-segment digit stride. */
static const fm_lcd_map_addr_t g_fm_lcd_map_top_row_decimal_points[FM_LCD_LAYOUT_TOP_ROW_COLUMNS - 1U] =
{
    { .reg = 7U, .bit = 7U },
    { .reg = 8U, .bit = 1U },
    { .reg = 8U, .bit = 3U },
    { .reg = 8U, .bit = 5U },
    { .reg = 8U, .bit = 7U },
    { .reg = 9U, .bit = 1U },
    { .reg = 9U, .bit = 3U }
};

static const fm_lcd_map_addr_t g_fm_lcd_map_bottom_row_decimal_points[FM_LCD_LAYOUT_BOTTOM_ROW_COLUMNS - 1U] =
{
    { .reg = 7U, .bit = 2U },
    { .reg = 7U, .bit = 0U },
    { .reg = 6U, .bit = 6U },
    { .reg = 6U, .bit = 4U },
    { .reg = 6U, .bit = 2U },
    { .reg = 6U, .bit = 0U }
};

/* =========================== Private Prototypes ========================= */
static fm_lcd_map_status_t fm_lcd_map_validate_buffer_(const uint8_t *p_ram,
                                                       uint8_t p_ram_size);
static uint8_t fm_lcd_map_get_row_columns_(fm_lcd_layout_row_t p_row);
static uint8_t fm_lcd_map_get_row_decimal_points_(fm_lcd_layout_row_t p_row);
static bool fm_lcd_map_get_decimal_addr_(fm_lcd_layout_row_t p_row,
                                         uint8_t p_col,
                                         uint8_t *p_reg,
                                         uint8_t *p_bit);
static bool fm_lcd_map_get_indicator_addr_(fm_lcd_layout_indicator_t p_indicator,
                                           uint8_t *p_reg,
                                           uint8_t *p_bit);
static bool fm_lcd_map_get_numeric_addr_(fm_lcd_layout_row_t p_row,
                                         uint8_t p_col,
                                         fm_lcd_map_segment_t p_segment,
                                         uint8_t *p_reg,
                                         uint8_t *p_bit);
static uint8_t fm_lcd_map_encode_char_(char p_char);
static void fm_lcd_map_set_bit_(uint8_t *p_ram, uint8_t p_reg, uint8_t p_bit, bool p_on);
static void fm_lcd_map_write_decimal_point_(uint8_t *p_ram,
                                            fm_lcd_layout_row_t p_row,
                                            uint8_t p_col,
                                            bool p_on);
static void fm_lcd_map_write_pattern_(uint8_t *p_ram,
                                      fm_lcd_layout_row_t p_row,
                                      uint8_t p_col,
                                      uint8_t p_pattern);

/* =========================== Private Bodies ============================= */
static fm_lcd_map_status_t fm_lcd_map_validate_buffer_(const uint8_t *p_ram,
                                                       uint8_t p_ram_size)
{
    if (p_ram == NULL)
    {
        return FM_LCD_MAP_EINVAL;
    }

    if (p_ram_size < FM_LCD_MAP_RAM_SIZE)
    {
        return FM_LCD_MAP_ERANGE;
    }

    return FM_LCD_MAP_OK;
}

static uint8_t fm_lcd_map_get_row_columns_(fm_lcd_layout_row_t p_row)
{
    switch (p_row)
    {
    case FM_LCD_LAYOUT_ROW_TOP:
        return FM_LCD_LAYOUT_TOP_ROW_COLUMNS;
    case FM_LCD_LAYOUT_ROW_BOTTOM:
        return FM_LCD_LAYOUT_BOTTOM_ROW_COLUMNS;
    default:
        return 0U;
    }
}

static uint8_t fm_lcd_map_get_row_decimal_points_(fm_lcd_layout_row_t p_row)
{
    uint8_t row_columns;

    row_columns = fm_lcd_map_get_row_columns_(p_row);

    if (row_columns == 0U)
    {
        return 0U;
    }

    return (uint8_t) (row_columns - 1U);
}

static bool fm_lcd_map_get_decimal_addr_(fm_lcd_layout_row_t p_row,
                                         uint8_t p_col,
                                         uint8_t *p_reg,
                                         uint8_t *p_bit)
{
    if ((p_reg == NULL) || (p_bit == NULL))
    {
        return false;
    }

    switch (p_row)
    {
    case FM_LCD_LAYOUT_ROW_TOP:
        if (p_col >= (FM_LCD_LAYOUT_TOP_ROW_COLUMNS - 1U))
        {
            return false;
        }

        *p_reg = g_fm_lcd_map_top_row_decimal_points[p_col].reg;
        *p_bit = g_fm_lcd_map_top_row_decimal_points[p_col].bit;
        return true;

    case FM_LCD_LAYOUT_ROW_BOTTOM:
        if (p_col >= (FM_LCD_LAYOUT_BOTTOM_ROW_COLUMNS - 1U))
        {
            return false;
        }

        *p_reg = g_fm_lcd_map_bottom_row_decimal_points[p_col].reg;
        *p_bit = g_fm_lcd_map_bottom_row_decimal_points[p_col].bit;
        return true;

    default:
        return false;
    }
}

static bool fm_lcd_map_get_indicator_addr_(fm_lcd_layout_indicator_t p_indicator,
                                           uint8_t *p_reg,
                                           uint8_t *p_bit)
{
    if ((p_reg == NULL) || (p_bit == NULL))
    {
        return false;
    }

    switch (p_indicator)
    {
    case FM_LCD_LAYOUT_INDICATOR_POINT:
        *p_reg = 7U;
        *p_bit = 4U;
        return true;
    case FM_LCD_LAYOUT_INDICATOR_BATTERY:
        *p_reg = 7U;
        *p_bit = 5U;
        return true;
    case FM_LCD_LAYOUT_INDICATOR_POWER:
        *p_reg = 2U;
        *p_bit = 5U;
        return true;
    case FM_LCD_LAYOUT_INDICATOR_RATE:
        *p_reg = 17U;
        *p_bit = 5U;
        return true;
    case FM_LCD_LAYOUT_INDICATOR_E:
        *p_reg = 2U;
        *p_bit = 4U;
        return true;
    case FM_LCD_LAYOUT_INDICATOR_BATCH:
        *p_reg = 12U;
        *p_bit = 5U;
        return true;
    case FM_LCD_LAYOUT_INDICATOR_ACM_TOP:
        *p_reg = 5U;
        *p_bit = 3U;
        return true;
    case FM_LCD_LAYOUT_INDICATOR_TTL:
        *p_reg = 17U;
        *p_bit = 4U;
        return true;
    case FM_LCD_LAYOUT_INDICATOR_SLASH:
        *p_reg = 10U;
        *p_bit = 1U;
        return true;
    case FM_LCD_LAYOUT_INDICATOR_ACM_BOTTOM:
        *p_reg = 12U;
        *p_bit = 4U;
        return true;
    case FM_LCD_LAYOUT_INDICATOR_H:
        *p_reg = 5U;
        *p_bit = 6U;
        return true;
    case FM_LCD_LAYOUT_INDICATOR_D:
        *p_reg = 9U;
        *p_bit = 5U;
        return true;
    case FM_LCD_LAYOUT_INDICATOR_S:
        *p_reg = 10U;
        *p_bit = 5U;
        return true;
    case FM_LCD_LAYOUT_INDICATOR_M:
        *p_reg = 9U;
        *p_bit = 7U;
        return true;
    default:
        return false;
    }
}

static bool fm_lcd_map_get_numeric_addr_(fm_lcd_layout_row_t p_row,
                                         uint8_t p_col,
                                         fm_lcd_map_segment_t p_segment,
                                         uint8_t *p_reg,
                                         uint8_t *p_bit)
{
    const fm_lcd_map_addr_t *p_base_segments;
    uint8_t row_columns;
    uint8_t internal_index;
    uint8_t pos;

    if ((p_reg == NULL) || (p_bit == NULL) || (p_segment >= FM_LCD_MAP_SEG_COUNT))
    {
        return false;
    }

    row_columns = fm_lcd_map_get_row_columns_(p_row);

    if ((row_columns == 0U) || (p_col >= row_columns))
    {
        return false;
    }

    if (p_row == FM_LCD_LAYOUT_ROW_TOP)
    {
        p_base_segments = g_fm_lcd_map_top_row_base_segments;
        internal_index = p_col;
    }
    else
    {
        p_base_segments = g_fm_lcd_map_bottom_row_base_segments;
        internal_index = (uint8_t) ((FM_LCD_LAYOUT_BOTTOM_ROW_COLUMNS - 1U) - p_col);
    }

    /* Each visible digit advances by two bit positions inside the controller
     * RAM image. When that stride crosses a byte boundary, the next segment
     * bit lands in the next RAM register. */
    pos = (uint8_t) (p_base_segments[p_segment].bit +
                     (internal_index * FM_LCD_MAP_DIGIT_STRIDE_BITS));

    *p_reg = (uint8_t) (p_base_segments[p_segment].reg + (pos / 8U));
    *p_bit = (uint8_t) (pos % 8U);

    return true;
}

static uint8_t fm_lcd_map_encode_char_(char p_char)
{
    switch (p_char)
    {
    case ' ':
        return 0U;
    case '-':
        return FM_LCD_MAP_SEG_D_MASK;
    case '0':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_E_MASK |
               FM_LCD_MAP_SEG_F_MASK | FM_LCD_MAP_SEG_G_MASK;
    case '1':
        return FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_F_MASK;
    case '2':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_D_MASK | FM_LCD_MAP_SEG_F_MASK |
               FM_LCD_MAP_SEG_G_MASK;
    case '3':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_C_MASK |
               FM_LCD_MAP_SEG_D_MASK | FM_LCD_MAP_SEG_F_MASK |
               FM_LCD_MAP_SEG_G_MASK;
    case '4':
        return FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_D_MASK |
               FM_LCD_MAP_SEG_E_MASK | FM_LCD_MAP_SEG_F_MASK;
    case '5':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_C_MASK |
               FM_LCD_MAP_SEG_D_MASK | FM_LCD_MAP_SEG_E_MASK |
               FM_LCD_MAP_SEG_G_MASK;
    case '6':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_D_MASK |
               FM_LCD_MAP_SEG_E_MASK | FM_LCD_MAP_SEG_G_MASK;
    case '7':
        return FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_F_MASK |
               FM_LCD_MAP_SEG_G_MASK;
    case '8':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_D_MASK |
               FM_LCD_MAP_SEG_E_MASK | FM_LCD_MAP_SEG_F_MASK |
               FM_LCD_MAP_SEG_G_MASK;
    case '9':
        return FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_D_MASK |
               FM_LCD_MAP_SEG_E_MASK | FM_LCD_MAP_SEG_F_MASK |
               FM_LCD_MAP_SEG_G_MASK;
    case 'A':
    case 'a':
        return FM_LCD_MAP_SEG_B_MASK | FM_LCD_MAP_SEG_C_MASK |
               FM_LCD_MAP_SEG_D_MASK | FM_LCD_MAP_SEG_E_MASK |
               FM_LCD_MAP_SEG_F_MASK | FM_LCD_MAP_SEG_G_MASK;
    case 'E':
    case 'e':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_D_MASK | FM_LCD_MAP_SEG_E_MASK |
               FM_LCD_MAP_SEG_G_MASK;
    case 'L':
    case 'l':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_E_MASK;
    case 'O':
    case 'o':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_E_MASK |
               FM_LCD_MAP_SEG_F_MASK | FM_LCD_MAP_SEG_G_MASK;
    case 'P':
    case 'p':
        return FM_LCD_MAP_SEG_B_MASK | FM_LCD_MAP_SEG_D_MASK |
               FM_LCD_MAP_SEG_E_MASK | FM_LCD_MAP_SEG_F_MASK |
               FM_LCD_MAP_SEG_G_MASK;
    case 'S':
    case 's':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_C_MASK |
               FM_LCD_MAP_SEG_D_MASK | FM_LCD_MAP_SEG_E_MASK |
               FM_LCD_MAP_SEG_G_MASK;
    case 'U':
    case 'u':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_E_MASK |
               FM_LCD_MAP_SEG_F_MASK;
    case '_':
        return FM_LCD_MAP_SEG_A_MASK;
    default:
        return 0U;
    }
}

static void fm_lcd_map_set_bit_(uint8_t *p_ram, uint8_t p_reg, uint8_t p_bit, bool p_on)
{
    if ((p_reg >= FM_LCD_MAP_RAM_SIZE) || (p_bit >= 8U))
    {
        return;
    }

    if (p_on)
    {
        p_ram[p_reg] |= (uint8_t) (1U << p_bit);
    }
    else
    {
        p_ram[p_reg] &= (uint8_t) ~(1U << p_bit);
    }
}

static void fm_lcd_map_write_decimal_point_(uint8_t *p_ram,
                                            fm_lcd_layout_row_t p_row,
                                            uint8_t p_col,
                                            bool p_on)
{
    uint8_t reg;
    uint8_t bit;

    if (!fm_lcd_map_get_decimal_addr_(p_row, p_col, &reg, &bit))
    {
        return;
    }

    fm_lcd_map_set_bit_(p_ram, reg, bit, p_on);
}

static void fm_lcd_map_write_pattern_(uint8_t *p_ram,
                                      fm_lcd_layout_row_t p_row,
                                      uint8_t p_col,
                                      uint8_t p_pattern)
{
    uint8_t reg;
    uint8_t bit;
    uint8_t segment;

    /* A visible digit is rendered segment-by-segment because its final RAM
     * image is distributed across multiple controller registers. */
    for (segment = 0U; segment < FM_LCD_MAP_SEG_COUNT; segment++)
    {
        if (fm_lcd_map_get_numeric_addr_(p_row,
                                         p_col,
                                         (fm_lcd_map_segment_t) segment,
                                         &reg,
                                         &bit))
        {
            fm_lcd_map_set_bit_(p_ram,
                                reg,
                                bit,
                                ((p_pattern & (uint8_t) (1U << segment)) != 0U));
        }
    }
}

/* =========================== Public Bodies ============================== */
fm_lcd_map_status_t FM_LCD_MAP_Clear(uint8_t *p_ram, uint8_t p_ram_size)
{
    fm_lcd_map_status_t status;

    status = fm_lcd_map_validate_buffer_(p_ram, p_ram_size);

    if (status != FM_LCD_MAP_OK)
    {
        return status;
    }

    (void) memset(p_ram, 0, FM_LCD_MAP_RAM_SIZE);

    return FM_LCD_MAP_OK;
}

fm_lcd_map_status_t FM_LCD_MAP_ClearRow(uint8_t *p_ram,
                                        uint8_t p_ram_size,
                                        fm_lcd_layout_row_t p_row)
{
    fm_lcd_map_status_t status;
    uint8_t col;
    uint8_t row_columns;
    uint8_t decimal_points;

    status = fm_lcd_map_validate_buffer_(p_ram, p_ram_size);

    if (status != FM_LCD_MAP_OK)
    {
        return status;
    }

    row_columns = fm_lcd_map_get_row_columns_(p_row);

    if (row_columns == 0U)
    {
        return FM_LCD_MAP_ERANGE;
    }

    for (col = 0U; col < row_columns; col++)
    {
        fm_lcd_map_write_pattern_(p_ram, p_row, col, 0U);
    }

    decimal_points = fm_lcd_map_get_row_decimal_points_(p_row);

    for (col = 0U; col < decimal_points; col++)
    {
        fm_lcd_map_write_decimal_point_(p_ram, p_row, col, false);
    }

    return FM_LCD_MAP_OK;
}

fm_lcd_map_status_t FM_LCD_MAP_WriteText(uint8_t *p_ram,
                                         uint8_t p_ram_size,
                                         fm_lcd_layout_row_t p_row,
                                         const char *p_text,
                                         fm_lcd_align_t p_align,
                                         bool p_clear_rest)
{
    fm_lcd_map_status_t status;
    uint8_t row_columns;
    uint8_t rendered_patterns[FM_LCD_LAYOUT_MAX_ROW_COLUMNS] = { 0U };
    bool decimal_points[FM_LCD_LAYOUT_MAX_ROW_COLUMNS] = { false };
    uint8_t rendered_count = 0U;
    uint8_t source_index = 0U;
    uint8_t start_col;
    uint8_t write_index;

    status = fm_lcd_map_validate_buffer_(p_ram, p_ram_size);

    if (status != FM_LCD_MAP_OK)
    {
        return status;
    }

    if (p_text == NULL)
    {
        return FM_LCD_MAP_EINVAL;
    }

    row_columns = fm_lcd_map_get_row_columns_(p_row);

    if (row_columns == 0U)
    {
        return FM_LCD_MAP_ERANGE;
    }

    if ((p_align != FM_LCD_ALIGN_LEFT) && (p_align != FM_LCD_ALIGN_RIGHT))
    {
        return FM_LCD_MAP_EINVAL;
    }

    while ((p_text[source_index] != '\0') && (rendered_count < row_columns))
    {
        if (p_text[source_index] == '.')
        {
            if (rendered_count > 0U)
            {
                decimal_points[rendered_count - 1U] = true;
            }
        }
        else
        {
            rendered_patterns[rendered_count] = fm_lcd_map_encode_char_(p_text[source_index]);
            rendered_count++;
        }

        source_index++;
    }

    if (p_clear_rest)
    {
        status = FM_LCD_MAP_ClearRow(p_ram, p_ram_size, p_row);

        if (status != FM_LCD_MAP_OK)
        {
            return status;
        }
    }

    if ((p_align == FM_LCD_ALIGN_RIGHT) && (rendered_count < row_columns))
    {
        start_col = (uint8_t) (row_columns - rendered_count);
    }
    else
    {
        start_col = 0U;
    }

    for (write_index = 0U; write_index < rendered_count; write_index++)
    {
        fm_lcd_map_write_pattern_(p_ram,
                                  p_row,
                                  (uint8_t) (start_col + write_index),
                                  rendered_patterns[write_index]);

        if ((start_col + write_index) < fm_lcd_map_get_row_decimal_points_(p_row))
        {
            fm_lcd_map_write_decimal_point_(p_ram,
                                            p_row,
                                            (uint8_t) (start_col + write_index),
                                            decimal_points[write_index]);
        }
    }

    return FM_LCD_MAP_OK;
}

fm_lcd_map_status_t FM_LCD_MAP_SetIndicator(uint8_t *p_ram,
                                            uint8_t p_ram_size,
                                            fm_lcd_layout_indicator_t p_indicator,
                                            bool p_on)
{
    fm_lcd_map_status_t status;
    uint8_t reg;
    uint8_t bit;

    status = fm_lcd_map_validate_buffer_(p_ram, p_ram_size);

    if (status != FM_LCD_MAP_OK)
    {
        return status;
    }

    if (!fm_lcd_map_get_indicator_addr_(p_indicator, &reg, &bit))
    {
        return FM_LCD_MAP_ERANGE;
    }

    fm_lcd_map_set_bit_(p_ram, reg, bit, p_on);

    return FM_LCD_MAP_OK;
}

/*** end of file ***/
