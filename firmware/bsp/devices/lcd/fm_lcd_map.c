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

typedef struct
{
    char ch;
    uint16_t left_pattern;
    uint16_t right_pattern;
} fm_lcd_map_alpha_encoding_t;

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
#define FM_LCD_MAP_ALPHA_SEG_COUNT       14U

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

static const fm_lcd_map_addr_t g_fm_lcd_map_alpha_left_segments[FM_LCD_MAP_ALPHA_SEG_COUNT] =
{
    { .reg = 0U,  .bit = 2U },
    { .reg = 0U,  .bit = 3U },
    { .reg = 0U,  .bit = 4U },
    { .reg = 0U,  .bit = 5U },
    { .reg = 5U,  .bit = 2U },
    { .reg = 5U,  .bit = 4U },
    { .reg = 5U,  .bit = 5U },
    { .reg = 10U, .bit = 2U },
    { .reg = 10U, .bit = 3U },
    { .reg = 10U, .bit = 4U },
    { .reg = 15U, .bit = 2U },
    { .reg = 15U, .bit = 3U },
    { .reg = 15U, .bit = 4U },
    { .reg = 15U, .bit = 5U }
};

static const fm_lcd_map_addr_t g_fm_lcd_map_alpha_right_segments[FM_LCD_MAP_ALPHA_SEG_COUNT] =
{
    { .reg = 0U,  .bit = 0U },
    { .reg = 0U,  .bit = 1U },
    { .reg = 4U,  .bit = 6U },
    { .reg = 4U,  .bit = 7U },
    { .reg = 5U,  .bit = 0U },
    { .reg = 5U,  .bit = 1U },
    { .reg = 9U,  .bit = 6U },
    { .reg = 10U, .bit = 0U },
    { .reg = 14U, .bit = 6U },
    { .reg = 14U, .bit = 7U },
    { .reg = 15U, .bit = 0U },
    { .reg = 15U, .bit = 1U },
    { .reg = 19U, .bit = 6U },
    { .reg = 19U, .bit = 7U }
};

/*
 * Keep alpha patterns per visible digit because left and right glass wiring do
 * not share the same segment ordering.
 *
 * Public string-facing policy for special or extended alpha glyphs:
 * - prefer single-byte printable ASCII inputs
 * - avoid non-printable control-byte encodings for normal application text
 * - render unsupported characters as blank
 *
 * Lowercase support is intentionally explicit in this table. Where a distinct
 * lowercase glyph has not been characterized and hardware-validated yet, the
 * lowercase entry falls back to the validated uppercase rendering.
 */
static const fm_lcd_map_alpha_encoding_t g_fm_lcd_map_alpha_encodings[] =
{
    { .ch = ' ', .left_pattern = 0x0000U, .right_pattern = 0x0000U },
    { .ch = '#', .left_pattern = 0x3FFFU, .right_pattern = 0x3FFFU },
    { .ch = '0', .left_pattern = 0x294AU, .right_pattern = 0x2A2AU },
    { .ch = '1', .left_pattern = 0x0802U, .right_pattern = 0x2008U },
    { .ch = '2', .left_pattern = 0x0D4CU, .right_pattern = 0x3223U },
    { .ch = '3', .left_pattern = 0x0D46U, .right_pattern = 0x3229U },
    { .ch = '4', .left_pattern = 0x2C06U, .right_pattern = 0x3809U },
    { .ch = '5', .left_pattern = 0x2542U, .right_pattern = 0x1A29U },
    { .ch = '6', .left_pattern = 0x254EU, .right_pattern = 0x1A2BU },
    { .ch = '7', .left_pattern = 0x0902U, .right_pattern = 0x2208U },
    { .ch = '8', .left_pattern = 0x2D4EU, .right_pattern = 0x3A2BU },
    { .ch = '9', .left_pattern = 0x2D06U, .right_pattern = 0x3A09U },
    { .ch = 'A', .left_pattern = 0x2D0EU, .right_pattern = 0x3A0BU },
    { .ch = 'B', .left_pattern = 0x0B56U, .right_pattern = 0x22E9U },
    { .ch = 'C', .left_pattern = 0x2148U, .right_pattern = 0x0A22U },
    { .ch = 'D', .left_pattern = 0x0B52U, .right_pattern = 0x22E8U },
    { .ch = 'E', .left_pattern = 0x2548U, .right_pattern = 0x1A22U },
    { .ch = 'F', .left_pattern = 0x2508U, .right_pattern = 0x1A02U },
    { .ch = 'G', .left_pattern = 0x254EU, .right_pattern = 0x1A2BU },
    { .ch = 'H', .left_pattern = 0x2C0EU, .right_pattern = 0x380BU },
    { .ch = 'I', .left_pattern = 0x0350U, .right_pattern = 0x02E0U },
    { .ch = 'J', .left_pattern = 0x0842U, .right_pattern = 0x2028U },
    { .ch = 'K', .left_pattern = 0x3428U, .right_pattern = 0x1C12U },
    { .ch = 'L', .left_pattern = 0x2048U, .right_pattern = 0x0822U },
    { .ch = 'M', .left_pattern = 0x388AU, .right_pattern = 0x2D0AU },
    { .ch = 'N', .left_pattern = 0x28AAU, .right_pattern = 0x291AU },
    { .ch = 'O', .left_pattern = 0x294AU, .right_pattern = 0x2A2AU },
    { .ch = 'P', .left_pattern = 0x2D0CU, .right_pattern = 0x3A03U },
    { .ch = 'Q', .left_pattern = 0x296AU, .right_pattern = 0x2A3AU },
    { .ch = 'R', .left_pattern = 0x2D2CU, .right_pattern = 0x3A13U },
    { .ch = 'S', .left_pattern = 0x2546U, .right_pattern = 0x1A29U },
    { .ch = 'T', .left_pattern = 0x0310U, .right_pattern = 0x02C0U },
    { .ch = 'U', .left_pattern = 0x284AU, .right_pattern = 0x282AU },
    { .ch = 'V', .left_pattern = 0x3009U, .right_pattern = 0x0C06U },
    { .ch = 'W', .left_pattern = 0x282BU, .right_pattern = 0x281EU },
    { .ch = 'X', .left_pattern = 0x10A1U, .right_pattern = 0x0514U },
    { .ch = 'Y', .left_pattern = 0x1090U, .right_pattern = 0x0540U },
    { .ch = 'Z', .left_pattern = 0x1545U, .right_pattern = 0x1625U },
    { .ch = 'a', .left_pattern = 0x2D0EU, .right_pattern = 0x3A0BU },
    { .ch = 'b', .left_pattern = 0x0B56U, .right_pattern = 0x22E9U },
    { .ch = 'c', .left_pattern = 0x2148U, .right_pattern = 0x0A22U },
    { .ch = 'd', .left_pattern = 0x0B52U, .right_pattern = 0x22E8U },
    { .ch = 'e', .left_pattern = 0x2548U, .right_pattern = 0x1A22U },
    { .ch = 'f', .left_pattern = 0x2508U, .right_pattern = 0x1A02U },
    { .ch = 'g', .left_pattern = 0x254EU, .right_pattern = 0x1A2BU },
    { .ch = 'h', .left_pattern = 0x2C0EU, .right_pattern = 0x380BU },
    { .ch = 'i', .left_pattern = 0x0350U, .right_pattern = 0x02E0U },
    { .ch = 'j', .left_pattern = 0x0842U, .right_pattern = 0x2028U },
    { .ch = 'k', .left_pattern = 0x3428U, .right_pattern = 0x1C12U },
    { .ch = 'l', .left_pattern = 0x2048U, .right_pattern = 0x0822U },
    { .ch = 'm', .left_pattern = 0x388AU, .right_pattern = 0x2D0AU },
    { .ch = 'n', .left_pattern = 0x28AAU, .right_pattern = 0x291AU },
    { .ch = 'o', .left_pattern = 0x294AU, .right_pattern = 0x2A2AU },
    { .ch = 'p', .left_pattern = 0x2D0CU, .right_pattern = 0x3A03U },
    { .ch = 'q', .left_pattern = 0x296AU, .right_pattern = 0x2A3AU },
    { .ch = 'r', .left_pattern = 0x2D2CU, .right_pattern = 0x3A13U },
    { .ch = 's', .left_pattern = 0x2546U, .right_pattern = 0x1A29U },
    { .ch = 't', .left_pattern = 0x0310U, .right_pattern = 0x02C0U },
    { .ch = 'u', .left_pattern = 0x284AU, .right_pattern = 0x282AU },
    { .ch = 'v', .left_pattern = 0x3009U, .right_pattern = 0x0C06U },
    { .ch = 'w', .left_pattern = 0x282BU, .right_pattern = 0x281EU },
    { .ch = 'x', .left_pattern = 0x10A1U, .right_pattern = 0x0514U },
    { .ch = 'y', .left_pattern = 0x1090U, .right_pattern = 0x0540U },
    { .ch = 'z', .left_pattern = 0x1545U, .right_pattern = 0x1625U }
};

/* =========================== Private Prototypes ========================= */
static fm_lcd_map_status_t fm_lcd_map_validate_buffer_(const uint8_t *p_ram,
                                                       uint8_t p_ram_size);
static const fm_lcd_map_addr_t *fm_lcd_map_get_alpha_segments_(fm_lcd_layout_alpha_digit_t p_digit);
static uint8_t fm_lcd_map_get_row_columns_(fm_lcd_layout_row_t p_row);
static uint8_t fm_lcd_map_get_row_decimal_points_(fm_lcd_layout_row_t p_row);
static uint16_t fm_lcd_map_encode_alpha_char_(char p_char,
                                              fm_lcd_layout_alpha_digit_t p_digit);
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
static char fm_lcd_map_get_canonical_numeric_glyph_(char p_char);
static uint8_t fm_lcd_map_encode_char_(char p_char);
static void fm_lcd_map_set_bit_(uint8_t *p_ram, uint8_t p_reg, uint8_t p_bit, bool p_on);
static void fm_lcd_map_write_alpha_pattern_(uint8_t *p_ram,
                                            fm_lcd_layout_alpha_digit_t p_digit,
                                            uint16_t p_pattern);
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

static const fm_lcd_map_addr_t *fm_lcd_map_get_alpha_segments_(fm_lcd_layout_alpha_digit_t p_digit)
{
    switch (p_digit)
    {
    case FM_LCD_LAYOUT_ALPHA_LEFT:
        return g_fm_lcd_map_alpha_left_segments;
    case FM_LCD_LAYOUT_ALPHA_RIGHT:
        return g_fm_lcd_map_alpha_right_segments;
    default:
        return NULL;
    }
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

static uint16_t fm_lcd_map_encode_alpha_char_(char p_char,
                                              fm_lcd_layout_alpha_digit_t p_digit)
{
    uint8_t index;

    for (index = 0U;
         index < (uint8_t) (sizeof(g_fm_lcd_map_alpha_encodings) /
                            sizeof(g_fm_lcd_map_alpha_encodings[0]));
         index++)
    {
        if (g_fm_lcd_map_alpha_encodings[index].ch == p_char)
        {
            if (p_digit == FM_LCD_LAYOUT_ALPHA_LEFT)
            {
                return g_fm_lcd_map_alpha_encodings[index].left_pattern;
            }

            if (p_digit == FM_LCD_LAYOUT_ALPHA_RIGHT)
            {
                return g_fm_lcd_map_alpha_encodings[index].right_pattern;
            }

            return 0U;
        }
    }

    if ((p_char >= 'a') && (p_char <= 'z'))
    {
        return fm_lcd_map_encode_alpha_char_(
            (char) (p_char - ('a' - 'A')),
            p_digit);
    }

    return 0U;
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

static char fm_lcd_map_get_canonical_numeric_glyph_(char p_char)
{
    if (((p_char >= '0') && (p_char <= '9')) ||
        (p_char == ' ') ||
        (p_char == '-') ||
        (p_char == '_'))
    {
        return p_char;
    }

    switch (p_char)
    {
    case 'A':
    case 'a':
        return 'A';
    case 'B':
    case 'b':
        return 'b';
    case 'C':
    case 'c':
        return 'C';
    case 'D':
    case 'd':
        return 'd';
    case 'E':
    case 'e':
        return 'E';
    case 'F':
    case 'f':
        return 'F';
    case 'G':
    case 'g':
        return '-';
    case 'H':
    case 'h':
        return 'H';
    case 'I':
    case 'i':
        return 'I';
    case 'J':
    case 'j':
        return 'J';
    case 'K':
    case 'k':
        return '-';
    case 'L':
    case 'l':
        return 'L';
    case 'M':
    case 'm':
        return '-';
    case 'N':
    case 'n':
        return 'n';
    case 'O':
    case 'o':
        return 'O';
    case 'P':
    case 'p':
        return 'P';
    case 'Q':
    case 'q':
        return 'q';
    case 'R':
    case 'r':
        return 'r';
    case 'S':
    case 's':
        return 'S';
    case 'T':
    case 't':
        return 't';
    case 'U':
    case 'u':
        return 'U';
    case 'V':
    case 'v':
        return 'u';
    case 'W':
    case 'w':
        return '-';
    case 'X':
    case 'x':
        return '-';
    case 'Y':
    case 'y':
        return 'y';
    case 'Z':
    case 'z':
        return '-';
    default:
        return '-';
    }
}

static uint8_t fm_lcd_map_encode_char_(char p_char)
{
    p_char = fm_lcd_map_get_canonical_numeric_glyph_(p_char);

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
        return FM_LCD_MAP_SEG_B_MASK | FM_LCD_MAP_SEG_C_MASK |
               FM_LCD_MAP_SEG_D_MASK | FM_LCD_MAP_SEG_E_MASK |
               FM_LCD_MAP_SEG_F_MASK | FM_LCD_MAP_SEG_G_MASK;
    case 'b':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_D_MASK |
               FM_LCD_MAP_SEG_E_MASK;
    case 'C':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_E_MASK | FM_LCD_MAP_SEG_G_MASK;
    case 'd':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_D_MASK |
               FM_LCD_MAP_SEG_F_MASK;
    case 'E':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_D_MASK | FM_LCD_MAP_SEG_E_MASK |
               FM_LCD_MAP_SEG_G_MASK;
    case 'F':
        return FM_LCD_MAP_SEG_B_MASK | FM_LCD_MAP_SEG_D_MASK |
               FM_LCD_MAP_SEG_E_MASK | FM_LCD_MAP_SEG_G_MASK;
    case 'H':
        return FM_LCD_MAP_SEG_B_MASK | FM_LCD_MAP_SEG_C_MASK |
               FM_LCD_MAP_SEG_D_MASK | FM_LCD_MAP_SEG_E_MASK |
               FM_LCD_MAP_SEG_F_MASK;
    case 'I':
        return FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_F_MASK;
    case 'J':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_F_MASK;
    case 'L':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_E_MASK;
    case 'n':
        return FM_LCD_MAP_SEG_B_MASK | FM_LCD_MAP_SEG_C_MASK |
               FM_LCD_MAP_SEG_D_MASK;
    case 'O':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_E_MASK |
               FM_LCD_MAP_SEG_F_MASK | FM_LCD_MAP_SEG_G_MASK;
    case 'P':
        return FM_LCD_MAP_SEG_B_MASK | FM_LCD_MAP_SEG_D_MASK |
               FM_LCD_MAP_SEG_E_MASK | FM_LCD_MAP_SEG_F_MASK |
               FM_LCD_MAP_SEG_G_MASK;
    case 'q':
        return FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_D_MASK |
               FM_LCD_MAP_SEG_E_MASK | FM_LCD_MAP_SEG_F_MASK |
               FM_LCD_MAP_SEG_G_MASK;
    case 'r':
        return FM_LCD_MAP_SEG_B_MASK | FM_LCD_MAP_SEG_D_MASK;
    case 'S':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_C_MASK |
               FM_LCD_MAP_SEG_D_MASK | FM_LCD_MAP_SEG_E_MASK |
               FM_LCD_MAP_SEG_G_MASK;
    case 't':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_D_MASK | FM_LCD_MAP_SEG_E_MASK;
    case 'U':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_B_MASK |
               FM_LCD_MAP_SEG_C_MASK | FM_LCD_MAP_SEG_E_MASK |
               FM_LCD_MAP_SEG_F_MASK;
    case 'y':
        return FM_LCD_MAP_SEG_A_MASK | FM_LCD_MAP_SEG_C_MASK |
               FM_LCD_MAP_SEG_D_MASK | FM_LCD_MAP_SEG_F_MASK;
    case '_':
        return FM_LCD_MAP_SEG_A_MASK;
    default:
        return FM_LCD_MAP_SEG_D_MASK;
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

static void fm_lcd_map_write_alpha_pattern_(uint8_t *p_ram,
                                            fm_lcd_layout_alpha_digit_t p_digit,
                                            uint16_t p_pattern)
{
    const fm_lcd_map_addr_t *p_segments;
    uint8_t segment;

    p_segments = fm_lcd_map_get_alpha_segments_(p_digit);

    if (p_segments == NULL)
    {
        return;
    }

    for (segment = 0U; segment < FM_LCD_MAP_ALPHA_SEG_COUNT; segment++)
    {
        fm_lcd_map_set_bit_(p_ram,
                            p_segments[segment].reg,
                            p_segments[segment].bit,
                            ((p_pattern & ((uint16_t) 1U << segment)) != 0U));
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

fm_lcd_map_status_t FM_LCD_MAP_ClearAlpha(uint8_t *p_ram,
                                          uint8_t p_ram_size)
{
    fm_lcd_map_status_t status;

    status = fm_lcd_map_validate_buffer_(p_ram, p_ram_size);

    if (status != FM_LCD_MAP_OK)
    {
        return status;
    }

    fm_lcd_map_write_alpha_pattern_(p_ram, FM_LCD_LAYOUT_ALPHA_LEFT, 0U);
    fm_lcd_map_write_alpha_pattern_(p_ram, FM_LCD_LAYOUT_ALPHA_RIGHT, 0U);

    return FM_LCD_MAP_OK;
}

fm_lcd_map_status_t FM_LCD_MAP_ClearRowCell(uint8_t *p_ram,
                                            uint8_t p_ram_size,
                                            fm_lcd_layout_row_t p_row,
                                            uint8_t p_col)
{
    fm_lcd_map_status_t status;
    uint8_t row_columns;

    status = fm_lcd_map_validate_buffer_(p_ram, p_ram_size);

    if (status != FM_LCD_MAP_OK)
    {
        return status;
    }

    row_columns = fm_lcd_map_get_row_columns_(p_row);

    if ((row_columns == 0U) || (p_col >= row_columns))
    {
        return FM_LCD_MAP_ERANGE;
    }

    fm_lcd_map_write_pattern_(p_ram, p_row, p_col, 0U);

    return FM_LCD_MAP_OK;
}

fm_lcd_map_status_t FM_LCD_MAP_ClearAlphaDigit(uint8_t *p_ram,
                                               uint8_t p_ram_size,
                                               fm_lcd_layout_alpha_digit_t p_digit)
{
    fm_lcd_map_status_t status;

    status = fm_lcd_map_validate_buffer_(p_ram, p_ram_size);

    if (status != FM_LCD_MAP_OK)
    {
        return status;
    }

    if (fm_lcd_map_get_alpha_segments_(p_digit) == NULL)
    {
        return FM_LCD_MAP_ERANGE;
    }

    fm_lcd_map_write_alpha_pattern_(p_ram, p_digit, 0U);

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

fm_lcd_map_status_t FM_LCD_MAP_WriteAlpha(uint8_t *p_ram,
                                          uint8_t p_ram_size,
                                          const char *p_text,
                                          fm_lcd_align_t p_align,
                                          bool p_clear_rest)
{
    fm_lcd_map_status_t status;
    char rendered_chars[FM_LCD_LAYOUT_ALPHA_DIGIT_COUNT] = { ' ', ' ' };
    uint8_t rendered_count;
    uint8_t source_index;
    uint8_t start_index;
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

    if ((p_align != FM_LCD_ALIGN_LEFT) && (p_align != FM_LCD_ALIGN_RIGHT))
    {
        return FM_LCD_MAP_EINVAL;
    }

    rendered_count = 0U;
    source_index = 0U;

    while ((p_text[source_index] != '\0') &&
           (rendered_count < FM_LCD_LAYOUT_ALPHA_DIGIT_COUNT))
    {
        rendered_chars[rendered_count] = p_text[source_index];
        rendered_count++;
        source_index++;
    }

    if (p_clear_rest)
    {
        status = FM_LCD_MAP_ClearAlpha(p_ram, p_ram_size);

        if (status != FM_LCD_MAP_OK)
        {
            return status;
        }
    }

    if ((p_align == FM_LCD_ALIGN_RIGHT) &&
        (rendered_count < FM_LCD_LAYOUT_ALPHA_DIGIT_COUNT))
    {
        start_index = (uint8_t) (FM_LCD_LAYOUT_ALPHA_DIGIT_COUNT - rendered_count);
    }
    else
    {
        start_index = 0U;
    }

    for (write_index = 0U; write_index < rendered_count; write_index++)
    {
        fm_lcd_map_write_alpha_pattern_(
            p_ram,
            (fm_lcd_layout_alpha_digit_t) (start_index + write_index),
            fm_lcd_map_encode_alpha_char_(
                rendered_chars[write_index],
                (fm_lcd_layout_alpha_digit_t) (start_index + write_index)));
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
