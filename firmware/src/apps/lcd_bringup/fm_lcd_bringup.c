/**
 * @file    fm_lcd_bringup.c
 * @brief   Sequential human-validation bring-up for the redesigned LCD stack.
 *
 * This app is intentionally a human-observed validation flow, not a generic
 * test harness. It cycles through a small set of scenes that cover the parts
 * of the new LCD stack already exposed by the public API:
 * - top numeric row
 * - bottom numeric row
 * - decimal points
 * - standalone indicators
 * - alphanumeric pair
 *
 * The human validation contract for this file is:
 * - read the UART scene label
 * - compare it against the visible LCD scene
 * - report any missing, mirrored, shifted, or cross-coupled segments
 *
 * This file is the canonical place for LCD bring-up validation evidence:
 * - UART scene labels for human comparison
 * - expected visible LCD behavior per scene
 * - any future scene additions that extend human validation coverage
 *
 * What the human should see in each scene:
 * - SCENE_CLEAR:
 *   full display off, no ghost segments
 * - SCENE_ALL_DIGITS_ON:
 *   top row "88888888", bottom row "8888888"
 * - SCENE_ALL_DECIMALS_ON:
 *   all decimal points on in both numeric rows
 * - SCENE_DIGIT_PATTERN:
 *   top row "12345678", bottom row "1234567" in visible order
 * - SCENE_BASIC_ICONS:
 *   only POINT and BATTERY indicators on
 * - SCENE_UPPER_LEGENDS:
 *   POWER, RATE, E, BATCH, ACM_TOP, TTL on
 * - SCENE_LOWER_RIGHT_LEGENDS:
 *   SLASH, ACM_BOTTOM, H, D, S, M on
 * - SCENE_ALL_INDICATORS:
 *   every standalone indicator on
 * - SCENE_NOMINAL_USE:
 *   top row "12.34", bottom row "56.7", BATTERY and POWER on
 * - SCENE_ALPHA_LEFT_FULL:
 *   left alpha digit fully on, right alpha digit off
 * - SCENE_ALPHA_RIGHT_FULL:
 *   right alpha digit fully on, left alpha digit off
 * - SCENE_ALPHA_CHARSET_SWEEP:
 *   final long test, both alpha digits walk through the application-facing
 *   alpha character set currently supported by the public string API
 */

#include "fm_lcd_bringup.h"

#include "fm_board.h"
#include "fm_debug.h"
#include "fm_port_time.h"
#include "devices/lcd/fm_lcd.h"

/* =========================== Private Macros ============================= */
#define FM_LCD_BRINGUP_SCENE_DELAY_MS       2000U
#define FM_LCD_BRINGUP_IDLE_DELAY_MS        100U
#define FM_LCD_BRINGUP_ALPHA_CHAR_DELAY_MS  750U

/* =========================== Private Types ============================== */
typedef enum
{
    FM_LCD_BRINGUP_SCENE_CLEAR = 0,
    FM_LCD_BRINGUP_SCENE_ALL_DIGITS_ON,
    FM_LCD_BRINGUP_SCENE_ALL_DECIMALS_ON,
    FM_LCD_BRINGUP_SCENE_DIGIT_PATTERN,
    FM_LCD_BRINGUP_SCENE_BASIC_ICONS,
    FM_LCD_BRINGUP_SCENE_UPPER_LEGENDS,
    FM_LCD_BRINGUP_SCENE_LOWER_RIGHT_LEGENDS,
    FM_LCD_BRINGUP_SCENE_ALL_INDICATORS,
    FM_LCD_BRINGUP_SCENE_NOMINAL_USE,
    FM_LCD_BRINGUP_SCENE_ALPHA_LEFT_FULL,
    FM_LCD_BRINGUP_SCENE_ALPHA_RIGHT_FULL,
    FM_LCD_BRINGUP_SCENE_ALPHA_CHARSET_SWEEP,
    FM_LCD_BRINGUP_SCENE_COUNT
} fm_lcd_bringup_scene_t;

/* =========================== Private Constants ========================== */
static const char g_fm_lcd_bringup_alpha_charset[] =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static const fm_lcd_layout_indicator_t g_fm_lcd_bringup_basic_icons[] =
{
    FM_LCD_LAYOUT_INDICATOR_POINT,
    FM_LCD_LAYOUT_INDICATOR_BATTERY
};

static const fm_lcd_layout_indicator_t g_fm_lcd_bringup_upper_legends[] =
{
    FM_LCD_LAYOUT_INDICATOR_POWER,
    FM_LCD_LAYOUT_INDICATOR_RATE,
    FM_LCD_LAYOUT_INDICATOR_E,
    FM_LCD_LAYOUT_INDICATOR_BATCH,
    FM_LCD_LAYOUT_INDICATOR_ACM_TOP,
    FM_LCD_LAYOUT_INDICATOR_TTL
};

static const fm_lcd_layout_indicator_t g_fm_lcd_bringup_lower_right_legends[] =
{
    FM_LCD_LAYOUT_INDICATOR_SLASH,
    FM_LCD_LAYOUT_INDICATOR_ACM_BOTTOM,
    FM_LCD_LAYOUT_INDICATOR_H,
    FM_LCD_LAYOUT_INDICATOR_D,
    FM_LCD_LAYOUT_INDICATOR_S,
    FM_LCD_LAYOUT_INDICATOR_M
};

static const fm_lcd_layout_indicator_t g_fm_lcd_bringup_all_indicators[] =
{
    FM_LCD_LAYOUT_INDICATOR_POINT,
    FM_LCD_LAYOUT_INDICATOR_BATTERY,
    FM_LCD_LAYOUT_INDICATOR_POWER,
    FM_LCD_LAYOUT_INDICATOR_RATE,
    FM_LCD_LAYOUT_INDICATOR_E,
    FM_LCD_LAYOUT_INDICATOR_BATCH,
    FM_LCD_LAYOUT_INDICATOR_ACM_TOP,
    FM_LCD_LAYOUT_INDICATOR_TTL,
    FM_LCD_LAYOUT_INDICATOR_SLASH,
    FM_LCD_LAYOUT_INDICATOR_ACM_BOTTOM,
    FM_LCD_LAYOUT_INDICATOR_H,
    FM_LCD_LAYOUT_INDICATOR_D,
    FM_LCD_LAYOUT_INDICATOR_S,
    FM_LCD_LAYOUT_INDICATOR_M
};

/* =========================== Private Prototypes ========================= */
static void fm_lcd_bringup_panic_(const char *p_msg);
static void fm_lcd_bringup_require_ok_(fm_lcd_status_t p_status, const char *p_msg);
static void fm_lcd_bringup_set_indicators_(const fm_lcd_layout_indicator_t *p_indicators,
                                           uint8_t p_count);
static void fm_lcd_bringup_run_alpha_charset_sweep_(void);
static void fm_lcd_bringup_prepare_scene_(void);
static void fm_lcd_bringup_emit_scene_(fm_lcd_bringup_scene_t p_scene);
static void fm_lcd_bringup_apply_scene_(fm_lcd_bringup_scene_t p_scene);

/* =========================== Private Bodies ============================= */
static void fm_lcd_bringup_panic_(const char *p_msg)
{
    FM_DEBUG_PanicMsg(p_msg);
}

static void fm_lcd_bringup_require_ok_(fm_lcd_status_t p_status, const char *p_msg)
{
    if (p_status != FM_LCD_OK)
    {
        fm_lcd_bringup_panic_(p_msg);
    }
}

static void fm_lcd_bringup_set_indicators_(const fm_lcd_layout_indicator_t *p_indicators,
                                           uint8_t p_count)
{
    uint8_t index;

    for (index = 0U; index < p_count; index++)
    {
        fm_lcd_bringup_require_ok_(FM_LCD_SetIndicator(p_indicators[index], true),
                                   "LCD_BRINGUP:INDICATOR_FAIL\n");
    }
}

static void fm_lcd_bringup_run_alpha_charset_sweep_(void)
{
    uint8_t index;
    char alpha_pair[3];
    char uart_msg[] = "LCD_BRINGUP:SCENE_ALPHA_CHARSET_SWEEP CHAR=?\n";

    for (index = 0U;
         index < (uint8_t) (sizeof(g_fm_lcd_bringup_alpha_charset) - 1U);
         index++)
    {
        alpha_pair[0] = g_fm_lcd_bringup_alpha_charset[index];
        alpha_pair[1] = g_fm_lcd_bringup_alpha_charset[index];
        alpha_pair[2] = '\0';

        fm_lcd_bringup_prepare_scene_();
        fm_lcd_bringup_require_ok_(FM_LCD_WriteAlpha(alpha_pair,
                                                     FM_LCD_ALIGN_LEFT,
                                                     true),
                                   "LCD_BRINGUP:ALPHA_FAIL\n");
        fm_lcd_bringup_require_ok_(FM_LCD_Flush(), "LCD_BRINGUP:FLUSH_FAIL\n");

        uart_msg[sizeof(uart_msg) - 3U] = alpha_pair[0];
        (void) FM_DEBUG_UartStr(uart_msg);
        FM_DEBUG_Flush();
        FM_PORT_TIME_SleepMs(FM_LCD_BRINGUP_ALPHA_CHAR_DELAY_MS);
    }
}

static void fm_lcd_bringup_prepare_scene_(void)
{
    fm_lcd_bringup_require_ok_(FM_LCD_Clear(), "LCD_BRINGUP:CLEAR_FAIL\n");
}

static void fm_lcd_bringup_emit_scene_(fm_lcd_bringup_scene_t p_scene)
{
    switch (p_scene)
    {
    case FM_LCD_BRINGUP_SCENE_CLEAR:
        (void) FM_DEBUG_UartStr("LCD_BRINGUP:SCENE_CLEAR\n");
        break;

    case FM_LCD_BRINGUP_SCENE_ALL_DIGITS_ON:
        (void) FM_DEBUG_UartStr("LCD_BRINGUP:SCENE_ALL_DIGITS_ON TOP=88888888 BOT=8888888\n");
        break;

    case FM_LCD_BRINGUP_SCENE_ALL_DECIMALS_ON:
        (void) FM_DEBUG_UartStr("LCD_BRINGUP:SCENE_ALL_DECIMALS_ON\n");
        break;

    case FM_LCD_BRINGUP_SCENE_DIGIT_PATTERN:
        (void) FM_DEBUG_UartStr("LCD_BRINGUP:SCENE_DIGIT_PATTERN TOP=12345678 BOT=1234567\n");
        break;

    case FM_LCD_BRINGUP_SCENE_BASIC_ICONS:
        (void) FM_DEBUG_UartStr("LCD_BRINGUP:SCENE_BASIC_ICONS POINT+BATT\n");
        break;

    case FM_LCD_BRINGUP_SCENE_UPPER_LEGENDS:
        (void) FM_DEBUG_UartStr("LCD_BRINGUP:SCENE_UPPER_LEGENDS\n");
        break;

    case FM_LCD_BRINGUP_SCENE_LOWER_RIGHT_LEGENDS:
        (void) FM_DEBUG_UartStr("LCD_BRINGUP:SCENE_LOWER_RIGHT_LEGENDS\n");
        break;

    case FM_LCD_BRINGUP_SCENE_ALL_INDICATORS:
        (void) FM_DEBUG_UartStr("LCD_BRINGUP:SCENE_ALL_INDICATORS\n");
        break;

    case FM_LCD_BRINGUP_SCENE_NOMINAL_USE:
        (void) FM_DEBUG_UartStr("LCD_BRINGUP:SCENE_NOMINAL_USE TOP=12.34 BOT=56.7\n");
        break;

    case FM_LCD_BRINGUP_SCENE_ALPHA_LEFT_FULL:
        (void) FM_DEBUG_UartStr("LCD_BRINGUP:SCENE_ALPHA_LEFT_FULL\n");
        break;

    case FM_LCD_BRINGUP_SCENE_ALPHA_RIGHT_FULL:
        (void) FM_DEBUG_UartStr("LCD_BRINGUP:SCENE_ALPHA_RIGHT_FULL\n");
        break;

    case FM_LCD_BRINGUP_SCENE_ALPHA_CHARSET_SWEEP:
        (void) FM_DEBUG_UartStr("LCD_BRINGUP:SCENE_ALPHA_CHARSET_SWEEP\n");
        break;

    default:
        (void) FM_DEBUG_UartStr("LCD_BRINGUP:SCENE_UNKNOWN\n");
        break;
    }
}

static void fm_lcd_bringup_apply_scene_(fm_lcd_bringup_scene_t p_scene)
{
    fm_lcd_bringup_prepare_scene_();

    switch (p_scene)
    {
    case FM_LCD_BRINGUP_SCENE_CLEAR:
        break;

    case FM_LCD_BRINGUP_SCENE_ALL_DIGITS_ON:
        fm_lcd_bringup_require_ok_(FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_TOP,
                                                    "88888888",
                                                    FM_LCD_ALIGN_LEFT,
                                                    true),
                                   "LCD_BRINGUP:ROW_TOP_FAIL\n");
        fm_lcd_bringup_require_ok_(FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_BOTTOM,
                                                    "8888888",
                                                    FM_LCD_ALIGN_LEFT,
                                                    true),
                                   "LCD_BRINGUP:ROW_BOTTOM_FAIL\n");
        break;

    case FM_LCD_BRINGUP_SCENE_ALL_DECIMALS_ON:
        fm_lcd_bringup_require_ok_(FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_TOP,
                                                    "8.8.8.8.8.8.8.8",
                                                    FM_LCD_ALIGN_LEFT,
                                                    true),
                                   "LCD_BRINGUP:ROW_TOP_FAIL\n");
        fm_lcd_bringup_require_ok_(FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_BOTTOM,
                                                    "8.8.8.8.8.8.8",
                                                    FM_LCD_ALIGN_LEFT,
                                                    true),
                                   "LCD_BRINGUP:ROW_BOTTOM_FAIL\n");
        break;

    case FM_LCD_BRINGUP_SCENE_DIGIT_PATTERN:
        fm_lcd_bringup_require_ok_(FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_TOP,
                                                    "12345678",
                                                    FM_LCD_ALIGN_LEFT,
                                                    true),
                                   "LCD_BRINGUP:ROW_TOP_FAIL\n");
        fm_lcd_bringup_require_ok_(FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_BOTTOM,
                                                    "1234567",
                                                    FM_LCD_ALIGN_LEFT,
                                                    true),
                                   "LCD_BRINGUP:ROW_BOTTOM_FAIL\n");
        break;

    case FM_LCD_BRINGUP_SCENE_BASIC_ICONS:
        fm_lcd_bringup_set_indicators_(g_fm_lcd_bringup_basic_icons,
                                       (uint8_t) (sizeof(g_fm_lcd_bringup_basic_icons) /
                                                  sizeof(g_fm_lcd_bringup_basic_icons[0])));
        break;

    case FM_LCD_BRINGUP_SCENE_UPPER_LEGENDS:
        fm_lcd_bringup_set_indicators_(g_fm_lcd_bringup_upper_legends,
                                       (uint8_t) (sizeof(g_fm_lcd_bringup_upper_legends) /
                                                  sizeof(g_fm_lcd_bringup_upper_legends[0])));
        break;

    case FM_LCD_BRINGUP_SCENE_LOWER_RIGHT_LEGENDS:
        fm_lcd_bringup_set_indicators_(g_fm_lcd_bringup_lower_right_legends,
                                       (uint8_t) (sizeof(g_fm_lcd_bringup_lower_right_legends) /
                                                  sizeof(g_fm_lcd_bringup_lower_right_legends[0])));
        break;

    case FM_LCD_BRINGUP_SCENE_ALL_INDICATORS:
        fm_lcd_bringup_set_indicators_(g_fm_lcd_bringup_all_indicators,
                                       (uint8_t) (sizeof(g_fm_lcd_bringup_all_indicators) /
                                                  sizeof(g_fm_lcd_bringup_all_indicators[0])));
        break;

    case FM_LCD_BRINGUP_SCENE_NOMINAL_USE:
        fm_lcd_bringup_require_ok_(FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_TOP,
                                                    "12.34",
                                                    FM_LCD_ALIGN_LEFT,
                                                    true),
                                   "LCD_BRINGUP:ROW_TOP_FAIL\n");
        fm_lcd_bringup_require_ok_(FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_BOTTOM,
                                                    "56.7",
                                                    FM_LCD_ALIGN_LEFT,
                                                    true),
                                   "LCD_BRINGUP:ROW_BOTTOM_FAIL\n");
        fm_lcd_bringup_require_ok_(FM_LCD_SetIndicator(FM_LCD_LAYOUT_INDICATOR_BATTERY, true),
                                   "LCD_BRINGUP:INDICATOR_FAIL\n");
        fm_lcd_bringup_require_ok_(FM_LCD_SetIndicator(FM_LCD_LAYOUT_INDICATOR_POWER, true),
                                   "LCD_BRINGUP:INDICATOR_FAIL\n");
        break;

    case FM_LCD_BRINGUP_SCENE_ALPHA_LEFT_FULL:
        fm_lcd_bringup_require_ok_(FM_LCD_WriteAlpha("#",
                                                     FM_LCD_ALIGN_LEFT,
                                                     true),
                                   "LCD_BRINGUP:ALPHA_FAIL\n");
        break;

    case FM_LCD_BRINGUP_SCENE_ALPHA_RIGHT_FULL:
        fm_lcd_bringup_require_ok_(FM_LCD_WriteAlpha("#",
                                                     FM_LCD_ALIGN_RIGHT,
                                                     true),
                                   "LCD_BRINGUP:ALPHA_FAIL\n");
        break;

    case FM_LCD_BRINGUP_SCENE_ALPHA_CHARSET_SWEEP:
        fm_lcd_bringup_emit_scene_(p_scene);
        fm_lcd_bringup_run_alpha_charset_sweep_();
        return;

    default:
        fm_lcd_bringup_panic_("LCD_BRINGUP:SCENE_INVALID\n");
        break;
    }

    fm_lcd_bringup_require_ok_(FM_LCD_Flush(), "LCD_BRINGUP:FLUSH_FAIL\n");
    fm_lcd_bringup_emit_scene_(p_scene);
}

/* =========================== Public Bodies ============================== */
void FM_LcdBringup_Run(void)
{
    fm_lcd_bringup_scene_t scene;

    FM_BOARD_Init();
    FM_DEBUG_Init();

    (void) FM_DEBUG_UartStr("LCD_BRINGUP:START\n");

    fm_lcd_bringup_require_ok_(FM_LCD_Init(), "LCD_BRINGUP:LCD_INIT_FAIL\n");

    (void) FM_DEBUG_UartStr("LCD_BRINGUP:LCD_INIT_OK\n");
    FM_DEBUG_LedRun(FM_DEBUG_LED_ON);

    for (;;)
    {
        (void) FM_DEBUG_UartStr("LCD_BRINGUP:LOOP_RESTART\n");

        for (scene = FM_LCD_BRINGUP_SCENE_CLEAR;
             scene < FM_LCD_BRINGUP_SCENE_COUNT;
             scene = (fm_lcd_bringup_scene_t) (scene + 1U))
        {
            fm_lcd_bringup_apply_scene_(scene);
            FM_DEBUG_Flush();
            FM_PORT_TIME_SleepMs(FM_LCD_BRINGUP_SCENE_DELAY_MS);
        }

        (void) FM_DEBUG_UartStr("LCD_BRINGUP:IDLE\n");
        FM_DEBUG_Flush();
        FM_PORT_TIME_SleepMs(FM_LCD_BRINGUP_IDLE_DELAY_MS);
    }
}

/*** end of file ***/
