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
 * - enable debug UART messages before reset
 * - open the ST-LINK VCP at 115200 8N1
 * - wait for `DISPLAY_FORMAT_LCD_BRINGUP:LCD_INIT_OK`
 * - read each `DISPLAY_FORMAT_LCD_BRINGUP:SCENE=<name> TOP=<text>
 *   BOTTOM=<text>` line
 * - compare the LCD rows against the UART values for that scene
 * - verify that decimal points are attached to the previous visible digit
 * - verify that leading zero padding, rounding, and overflow markers match
 *
 * Expected scene observations:
 * - `PADDED_INTEGER`: TOP=`00000123`, BOTTOM=`0000123`
 * - `PADDED_DECIMAL_1`: TOP=`0000012.3`, BOTTOM=`000012.3`
 * - `PADDED_DECIMAL_2`: TOP=`000001.23`, BOTTOM=`00001.23`
 * - `PADDED_DECIMAL_3`: TOP=`00000.123`, BOTTOM=`0000.123`
 * - `ROUNDED_DECIMAL`: TOP=`0000123.5`, BOTTOM=`000123.5`
 * - `LOW_DECIMAL`: TOP=`0000000.1`, BOTTOM=`000000.1`
 * - `OVERFLOW_FILL`: TOP=`--------`, BOTTOM=`-------`
 *
 * Error communication:
 * - format, expected-text, LCD clear, LCD write, flush, or LCD init failures
 *   call `FM_DEBUG_PanicMsg()`
 * - with debug UART enabled, the board prints the panic reason
 * - the red error LED is expected to turn on during panic
 * - the LCD usually remains frozen on the last successfully written case
 *
 * Report failures with the UART log tail, red LED state, and frozen LCD text.
 * Example: "UART ended at TEXT_MISMATCH after SCENE=PADDED_DECIMAL_3;
 * red LED on; LCD frozen at 00000.123."
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
#define FM_DISPLAY_FORMAT_LCD_BRINGUP_TOP_WIDTH        FM_LCD_LAYOUT_TOP_ROW_COLUMNS
#define FM_DISPLAY_FORMAT_LCD_BRINGUP_BOTTOM_WIDTH     FM_LCD_LAYOUT_BOTTOM_ROW_COLUMNS
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
    fm_display_format_lcd_bringup_kind_t kind;
    int64_t scaled_value;
    uint32_t unsigned_value;
    uint8_t scale_digits;
    display_format_field_t field;
    const char *expected_text;
} fm_display_format_lcd_bringup_value_t;

typedef struct
{
    const char *name;
    fm_display_format_lcd_bringup_value_t top;
    fm_display_format_lcd_bringup_value_t bottom;
} fm_display_format_lcd_bringup_scene_t;

/* =========================== Private Constants ========================== */
static const fm_display_format_lcd_bringup_scene_t
    g_fm_display_format_lcd_bringup_scenes[] =
{
    {
        .name = "PADDED_INTEGER",
        .top = {
            .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_UNSIGNED,
            .scaled_value = 0,
            .unsigned_value = 123U,
            .scale_digits = 0U,
            .field = {
                .visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_TOP_WIDTH,
                .fractional_digits = 0U,
                .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                .pad_char = '0',
                .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
                .overflow_char = '-'
            },
            .expected_text = "00000123"
        },
        .bottom = {
            .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_UNSIGNED,
            .scaled_value = 0,
            .unsigned_value = 123U,
            .scale_digits = 0U,
            .field = {
                .visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_BOTTOM_WIDTH,
                .fractional_digits = 0U,
                .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                .pad_char = '0',
                .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
                .overflow_char = '-'
            },
            .expected_text = "0000123"
        }
    },
    {
        .name = "PADDED_DECIMAL_1",
        .top = {
            .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED,
            .scaled_value = 123,
            .unsigned_value = 0U,
            .scale_digits = 1U,
            .field = {
                .visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_TOP_WIDTH,
                .fractional_digits = 1U,
                .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                .pad_char = '0',
                .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
                .overflow_char = '-'
            },
            .expected_text = "0000012.3"
        },
        .bottom = {
            .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED,
            .scaled_value = 123,
            .unsigned_value = 0U,
            .scale_digits = 1U,
            .field = {
                .visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_BOTTOM_WIDTH,
                .fractional_digits = 1U,
                .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                .pad_char = '0',
                .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
                .overflow_char = '-'
            },
            .expected_text = "000012.3"
        }
    },
    {
        .name = "PADDED_DECIMAL_2",
        .top = {
            .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED,
            .scaled_value = 123,
            .unsigned_value = 0U,
            .scale_digits = 2U,
            .field = {
                .visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_TOP_WIDTH,
                .fractional_digits = 2U,
                .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                .pad_char = '0',
                .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
                .overflow_char = '-'
            },
            .expected_text = "000001.23"
        },
        .bottom = {
            .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED,
            .scaled_value = 123,
            .unsigned_value = 0U,
            .scale_digits = 2U,
            .field = {
                .visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_BOTTOM_WIDTH,
                .fractional_digits = 2U,
                .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                .pad_char = '0',
                .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
                .overflow_char = '-'
            },
            .expected_text = "00001.23"
        }
    },
    {
        .name = "PADDED_DECIMAL_3",
        .top = {
            .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED,
            .scaled_value = 123,
            .unsigned_value = 0U,
            .scale_digits = 3U,
            .field = {
                .visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_TOP_WIDTH,
                .fractional_digits = 3U,
                .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                .pad_char = '0',
                .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
                .overflow_char = '-'
            },
            .expected_text = "00000.123"
        },
        .bottom = {
            .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED,
            .scaled_value = 123,
            .unsigned_value = 0U,
            .scale_digits = 3U,
            .field = {
                .visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_BOTTOM_WIDTH,
                .fractional_digits = 3U,
                .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                .pad_char = '0',
                .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
                .overflow_char = '-'
            },
            .expected_text = "0000.123"
        }
    },
    {
        .name = "ROUNDED_DECIMAL",
        .top = {
            .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED,
            .scaled_value = 12345,
            .unsigned_value = 0U,
            .scale_digits = 2U,
            .field = {
                .visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_TOP_WIDTH,
                .fractional_digits = 1U,
                .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                .pad_char = '0',
                .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
                .overflow_char = '-'
            },
            .expected_text = "0000123.5"
        },
        .bottom = {
            .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED,
            .scaled_value = 12345,
            .unsigned_value = 0U,
            .scale_digits = 2U,
            .field = {
                .visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_BOTTOM_WIDTH,
                .fractional_digits = 1U,
                .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                .pad_char = '0',
                .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
                .overflow_char = '-'
            },
            .expected_text = "000123.5"
        }
    },
    {
        .name = "LOW_DECIMAL",
        .top = {
            .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED,
            .scaled_value = 1,
            .unsigned_value = 0U,
            .scale_digits = 1U,
            .field = {
                .visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_TOP_WIDTH,
                .fractional_digits = 1U,
                .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                .pad_char = '0',
                .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
                .overflow_char = '-'
            },
            .expected_text = "0000000.1"
        },
        .bottom = {
            .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_SCALED,
            .scaled_value = 1,
            .unsigned_value = 0U,
            .scale_digits = 1U,
            .field = {
                .visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_BOTTOM_WIDTH,
                .fractional_digits = 1U,
                .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                .pad_char = '0',
                .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
                .overflow_char = '-'
            },
            .expected_text = "000000.1"
        }
    },
    {
        .name = "OVERFLOW_FILL",
        .top = {
            .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_UNSIGNED,
            .scaled_value = 0,
            .unsigned_value = 123456789U,
            .scale_digits = 0U,
            .field = {
                .visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_TOP_WIDTH,
                .fractional_digits = 0U,
                .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                .pad_char = '0',
                .overflow_policy = DISPLAY_FORMAT_OVERFLOW_FILL,
                .overflow_char = '-'
            },
            .expected_text = "--------"
        },
        .bottom = {
            .kind = FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_UNSIGNED,
            .scaled_value = 0,
            .unsigned_value = 123456789U,
            .scale_digits = 0U,
            .field = {
                .visible_width = FM_DISPLAY_FORMAT_LCD_BRINGUP_BOTTOM_WIDTH,
                .fractional_digits = 0U,
                .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                .pad_char = '0',
                .overflow_policy = DISPLAY_FORMAT_OVERFLOW_FILL,
                .overflow_char = '-'
            },
            .expected_text = "-------"
        }
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
    const fm_display_format_lcd_bringup_scene_t *p_scene,
    const char *p_top,
    const char *p_bottom);
static void fm_display_format_lcd_bringup_format_value_(
    const fm_display_format_lcd_bringup_value_t *p_value,
    char *p_text,
    size_t p_text_size);
static bool fm_display_format_lcd_bringup_text_eq_(const char *p_actual,
                                                   const char *p_expected);
static void fm_display_format_lcd_bringup_apply_case_(
    const fm_display_format_lcd_bringup_scene_t *p_scene);

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
    const fm_display_format_lcd_bringup_scene_t *p_scene,
    const char *p_top,
    const char *p_bottom)
{
    (void) FM_DEBUG_UartStr("DISPLAY_FORMAT_LCD_BRINGUP:SCENE=");
    (void) FM_DEBUG_UartStr(p_scene->name);
    (void) FM_DEBUG_UartStr(" TOP=");
    (void) FM_DEBUG_UartStr(p_top);
    (void) FM_DEBUG_UartStr(" BOTTOM=");
    (void) FM_DEBUG_UartStr(p_bottom);
    (void) FM_DEBUG_UartStr("\n");
}

static void fm_display_format_lcd_bringup_format_value_(
    const fm_display_format_lcd_bringup_value_t *p_value,
    char *p_text,
    size_t p_text_size)
{
    fm_status_t status;
    fm_status_t expected_status = FM_STATUS_OK;

    if (p_value->field.overflow_policy == DISPLAY_FORMAT_OVERFLOW_FILL)
    {
        expected_status = FM_STATUS_ERANGE;
    }

    if (p_value->kind == FM_DISPLAY_FORMAT_LCD_BRINGUP_KIND_UNSIGNED)
    {
        status = DISPLAY_FORMAT_Unsigned(p_value->unsigned_value,
                                         &p_value->field,
                                         p_text,
                                         p_text_size);
    }
    else
    {
        status = DISPLAY_FORMAT_Scaled(p_value->scaled_value,
                                       p_value->scale_digits,
                                       &p_value->field,
                                       p_text,
                                       p_text_size);
    }

    fm_display_format_lcd_bringup_require_format_status_(
        status,
        expected_status,
        "DISPLAY_FORMAT_LCD_BRINGUP:FORMAT_FAIL\n");

    if (!fm_display_format_lcd_bringup_text_eq_(p_text,
                                                p_value->expected_text))
    {
        fm_display_format_lcd_bringup_panic_(
            "DISPLAY_FORMAT_LCD_BRINGUP:TEXT_MISMATCH\n");
    }
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
    const fm_display_format_lcd_bringup_scene_t *p_scene)
{
    char top[FM_DISPLAY_FORMAT_LCD_BRINGUP_TEXT_SIZE];
    char bottom[FM_DISPLAY_FORMAT_LCD_BRINGUP_TEXT_SIZE];

    fm_display_format_lcd_bringup_format_value_(&p_scene->top,
                                                top,
                                                sizeof(top));
    fm_display_format_lcd_bringup_format_value_(&p_scene->bottom,
                                                bottom,
                                                sizeof(bottom));

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
        FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_BOTTOM,
                         bottom,
                         FM_LCD_ALIGN_LEFT,
                         true),
        "DISPLAY_FORMAT_LCD_BRINGUP:BOTTOM_FAIL\n");
    fm_display_format_lcd_bringup_require_lcd_ok_(
        FM_LCD_Flush(),
        "DISPLAY_FORMAT_LCD_BRINGUP:FLUSH_FAIL\n");

    fm_display_format_lcd_bringup_emit_case_(p_scene, top, bottom);
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
             index < (uint8_t) (sizeof(g_fm_display_format_lcd_bringup_scenes) /
                                sizeof(g_fm_display_format_lcd_bringup_scenes[0]));
             index++)
        {
            fm_display_format_lcd_bringup_apply_case_(
                &g_fm_display_format_lcd_bringup_scenes[index]);
            FM_PORT_TIME_SleepMs(FM_DISPLAY_FORMAT_LCD_BRINGUP_SCENE_DELAY_MS);
        }

        (void) FM_DEBUG_UartStr("DISPLAY_FORMAT_LCD_BRINGUP:IDLE\n");
        FM_DEBUG_Flush();
        FM_PORT_TIME_SleepMs(FM_DISPLAY_FORMAT_LCD_BRINGUP_IDLE_DELAY_MS);
    }
}
