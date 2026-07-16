/**
 * @file    fm_display_format_lcd_bringup.c
 * @brief   Human-observed bring-up for DISPLAY_FORMAT output on the LCD.
 *
 * This app validates the practical chain:
 * - numeric value
 * - `DISPLAY_FORMAT_*`
 * - text containing optional decimal point
 * - `FM_LCD_WriteText()`
 * - physical LCD glass
 *
 * Human validation contract:
 * - read the UART line for each case
 * - compare `TOP=<text>` against the LCD top row
 * - verify zero padding, decimal position, rounding, and overflow marker
 */
#include "fm_display_format_lcd_bringup.h"

#include <stddef.h>
#include <stdint.h>

#include "display_format.h"
#include "fm_board.h"
#include "fm_debug.h"
#include "fm_port_time.h"
#include "fm_status.h"
#include "devices/lcd/fm_lcd.h"

/* =========================== Private Macros ============================= */
#define FM_DISPLAY_FORMAT_LCD_BRINGUP_TOP_WIDTH        8U
#define FM_DISPLAY_FORMAT_LCD_BRINGUP_SCENE_DELAY_MS   3000U
#define FM_DISPLAY_FORMAT_LCD_BRINGUP_IDLE_DELAY_MS    500U
#define FM_DISPLAY_FORMAT_LCD_BRINGUP_TEXT_SIZE        16U

/* =========================== Private Types ============================== */
typedef enum
{
    FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED = 0,
    FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_UNSIGNED
} fm_display_format_lcd_bringup_kind_t;

typedef struct
{
    const char *name;
    fm_display_format_lcd_bringup_kind_t kind;
    int64_t scaled_value;
    uint32_t unsigned_value;
    uint8_t scale_digits;
    uint8_t fractional_digits;
    display_format_overflow_policy_t overflow_policy;
    const char *expected_top;
} fm_display_format_lcd_bringup_case_t;

/* =========================== Private Constants ========================== */
static const fm_display_format_lcd_bringup_case_t
    g_fm_display_format_lcd_bringup_cases[] =
{
    {
        .name = "PADDED_INTEGER",
        .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_UNSIGNED,
        .scaled_value = 0,
        .unsigned_value = 123U,
        .scale_digits = 0U,
        .fractional_digits = 0U,
        .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
        .expected_top = "00000123"
    },
    {
        .name = "PADDED_DECIMAL_1",
        .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED,
        .scaled_value = 123,
        .unsigned_value = 0U,
        .scale_digits = 1U,
        .fractional_digits = 1U,
        .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
        .expected_top = "0000012.3"
    },
    {
        .name = "PADDED_DECIMAL_2",
        .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED,
        .scaled_value = 123,
        .unsigned_value = 0U,
        .scale_digits = 2U,
        .fractional_digits = 2U,
        .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
        .expected_top = "000001.23"
    },
    {
        .name = "PADDED_DECIMAL_3",
        .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED,
        .scaled_value = 123,
        .unsigned_value = 0U,
        .scale_digits = 3U,
        .fractional_digits = 3U,
        .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
        .expected_top = "00000.123"
    },
    {
        .name = "ROUNDED_DECIMAL",
        .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED,
        .scaled_value = 12345,
        .unsigned_value = 0U,
        .scale_digits = 2U,
        .fractional_digits = 1U,
        .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
        .expected_top = "000123.5"
    },
    {
        .name = "OVERFLOW_FILL",
        .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_UNSIGNED,
        .scaled_value = 0,
        .unsigned_value = 123456789U,
        .scale_digits = 0U,
        .fractional_digits = 0U,
        .overflow_policy = DISPLAY_FORMAT_OVERFLOW_FILL,
        .expected_top = "--------"
    }
};

/* =========================== Private Prototypes ========================= */
static void fm_display_format_lcd_bringup_panic_(const char *p_msg);
static void fm_display_format_lcd_bringup_require_lcd_ok_(fm_lcd_status_t p_status,
                                                          const char *p_msg);
static void fm_display_format_lcd_bringup_require_format_status_(fm_status_t p_status,
                                                                 fm_status_t p_expected,
                                                                 const char *p_msg);
static void fm_display_format_lcd_bringup_emit_case_(
    const fm_display_format_lcd_bringup_case_t *p_case,
    const char *p_top);
static bool fm_display_format_lcd_bringup_text_eq_(const char *p_actual,
                                                   const char *p_expected);
static void fm_display_format_lcd_bringup_apply_case_(
    const fm_display_format_lcd_bringup_case_t *p_case);

/* =========================== Private Bodies ============================= */
static void fm_display_format_lcd_bringup_panic_(const char *p_msg)
{
    FM_DEBUG_PanicMsg(p_msg);
}

static void fm_display_format_lcd_bringup_require_lcd_ok_(fm_lcd_status_t p_status,
                                                          const char *p_msg)
{
    if (p_status != FM_LCD_OK)
    {
        fm_display_format_lcd_bringup_panic_(p_msg);
    }
}

static void fm_display_format_lcd_bringup_require_format_status_(fm_status_t p_status,
                                                                 fm_status_t p_expected,
                                                                 const char *p_msg)
{
    if (p_status != p_expected)
    {
        fm_display_format_lcd_bringup_panic_(p_msg);
    }
}

static void fm_display_format_lcd_bringup_emit_case_(
    const fm_display_format_lcd_bringup_case_t *p_case,
    const char *p_top)
{
    (void) FM_DEBUG_UartStr("DISPLAY_FORMAT_LCD_BRINGUP:CASE=");
    (void) FM_DEBUG_UartStr(p_case->name);
    (void) FM_DEBUG_UartStr(" TOP=");
    (void) FM_DEBUG_UartStr(p_top);
    (void) FM_DEBUG_UartStr("\n");
}

static bool fm_display_format_lcd_bringup_text_eq_(const char *p_actual,
                                                   const char *p_expected)
{
    while ((*p_actual != '\0') && (*p_expected != '\0'))
    {
        if (*p_actual != *p_expected)
        {
            return false;
        }

        p_actual++;
        p_expected++;
    }

    return (*p_actual == '\0') && (*p_expected == '\0');
}

static void fm_display_format_lcd_bringup_apply_case_(
    const fm_display_format_lcd_bringup_case_t *p_case)
{
    display_format_field_t field;
    char top[FM_DISPLAY_FORMAT_LCD_BRINGUP_TEXT_SIZE];
    fm_status_t status;
    fm_status_t expected_status = FM_STATUS_OK;

    field.visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_TOP_WIDTH;
    field.fractional_digits = p_case->fractional_digits;
    field.align = DISPLAY_FORMAT_ALIGN_RIGHT;
    field.pad_char = '0';
    field.overflow_policy = p_case->overflow_policy;
    field.overflow_char = '-';

    if (p_case->overflow_policy == DISPLAY_FORMAT_OVERFLOW_FILL)
    {
        expected_status = FM_STATUS_ERANGE;
    }

    if (p_case->kind == FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_UNSIGNED)
    {
        status = DISPLAY_FORMAT_Unsigned(p_case->unsigned_value,
                                         &field,
                                         top,
                                         sizeof(top));
    }
    else
    {
        status = DISPLAY_FORMAT_Scaled(p_case->scaled_value,
                                       p_case->scale_digits,
                                       &field,
                                       top,
                                       sizeof(top));
    }

    fm_display_format_lcd_bringup_require_format_status_(
        status,
        expected_status,
        "DISPLAY_FORMAT_LCD_BRINGUP:FORMAT_FAIL\n");

    if (!fm_display_format_lcd_bringup_text_eq_(top, p_case->expected_top))
    {
        fm_display_format_lcd_bringup_panic_(
            "DISPLAY_FORMAT_LCD_BRINGUP:TEXT_MISMATCH\n");
    }

    fm_display_format_lcd_bringup_require_lcd_ok_(
        FM_LCD_Clear(),
        "DISPLAY_FORMAT_LCD_BRINGUP:CLEAR_FAIL\n");
    fm_display_format_lcd_bringup_require_lcd_ok_(
        FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_TOP,
                         top,
                         FM_LCD_ALIGN_LEFT,
                         true),
        "DISPLAY_FORMAT_LCD_BRINGUP:TOP_FAIL\n");
    fm_display_format_lcd_bringup_require_lcd_ok_(
        FM_LCD_Flush(),
        "DISPLAY_FORMAT_LCD_BRINGUP:FLUSH_FAIL\n");

    fm_display_format_lcd_bringup_emit_case_(p_case, top);
    FM_DEBUG_Flush();
}

/* =========================== Public Bodies ============================== */
void FM_DisplayFormatLcdBringup_Run(void)
{
    uint8_t index;

    FM_BOARD_Init();
    FM_DEBUG_Init();

    (void) FM_DEBUG_UartStr("DISPLAY_FORMAT_LCD_BRINGUP:START\n");

    fm_display_format_lcd_bringup_require_lcd_ok_(
        FM_LCD_Init(),
        "DISPLAY_FORMAT_LCD_BRINGUP:LCD_INIT_FAIL\n");

    (void) FM_DEBUG_UartStr("DISPLAY_FORMAT_LCD_BRINGUP:LCD_INIT_OK\n");
    FM_DEBUG_LedRun(FM_DEBUG_LED_ON);

    for (;;)
    {
        (void) FM_DEBUG_UartStr("DISPLAY_FORMAT_LCD_BRINGUP:LOOP_RESTART\n");

        for (index = 0U;
             index < (uint8_t) (sizeof(g_fm_display_format_lcd_bringup_cases) /
                                sizeof(g_fm_display_format_lcd_bringup_cases[0]));
             index++)
        {
            fm_display_format_lcd_bringup_apply_case_(
                &g_fm_display_format_lcd_bringup_cases[index]);
            FM_PORT_TIME_SleepMs(FM_DISPLAY_FORMAT_LCD_BRINGUP_SCENE_DELAY_MS);
        }

        (void) FM_DEBUG_UartStr("DISPLAY_FORMAT_LCD_BRINGUP:IDLE\n");
        FM_DEBUG_Flush();
        FM_PORT_TIME_SleepMs(FM_DISPLAY_FORMAT_LCD_BRINGUP_IDLE_DELAY_MS);
    }
}
