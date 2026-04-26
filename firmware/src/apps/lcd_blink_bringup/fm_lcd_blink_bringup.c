/**
 * @file    fm_lcd_blink_bringup.c
 * @brief   Dedicated blink bring-up for the redesigned LCD stack.
 *
 * This app keeps blink timing outside `fm_lcd.*` and validates one simple
 * "hello world" scene by toggling logical blink phase with a blocking delay.
 *
 * Visible intent of the single scene:
 * - top row base text: "HELLO123"
 * - bottom row base text: "WORLD42"
 * - alpha base text: "HI"
 *
 * Blink selection:
 * - top row visible columns 0..4  -> "HELLO"
 * - bottom row visible columns 0..4 -> "WORLD"
 * - alpha digits 0..1 -> "HI"
 *
 * Human expectation:
 * - ON phase: "HELLO123" / "WORLD42" / "HI"
 * - OFF phase: only the non-selected suffix remains visible, so
 *   top row shows "     123", bottom row shows "     42", and alpha is blank
 */

#include "fm_lcd_blink_bringup.h"

#include <stdint.h>

#include "fm_board.h"
#include "fm_debug.h"
#include "fm_port_time.h"
#include "devices/lcd/fm_lcd.h"
#include "devices/lcd/fm_lcd_blink.h"

/* =========================== Private Macros ============================= */
#define FM_LCD_BLINK_BRINGUP_ON_MS    700U
#define FM_LCD_BLINK_BRINGUP_OFF_MS   300U

/* =========================== Private Constants ========================== */
static const fm_lcd_blink_range_t g_fm_lcd_blink_bringup_ranges[] =
{
    {
        .field = FM_LCD_TEXT_FIELD_TOP_ROW,
        .start = 0U,
        .length = 5U
    },
    {
        .field = FM_LCD_TEXT_FIELD_BOTTOM_ROW,
        .start = 0U,
        .length = 5U
    },
    {
        .field = FM_LCD_TEXT_FIELD_ALPHA,
        .start = 0U,
        .length = 2U
    }
};

/* =========================== Private Prototypes ========================= */
static void fm_lcd_blink_bringup_panic_(const char *p_msg);
static void fm_lcd_blink_bringup_require_ok_(fm_lcd_status_t p_status,
                                             const char *p_msg);
static void fm_lcd_blink_bringup_require_policy_ok_(fm_lcd_blink_status_t p_status,
                                                    const char *p_msg);
static void fm_lcd_blink_bringup_prepare_scene_(void);
static void fm_lcd_blink_bringup_apply_scene_(void);
static void fm_lcd_blink_bringup_apply_phase_(fm_lcd_blink_phase_t p_phase);

/* =========================== Private Bodies ============================= */
static void fm_lcd_blink_bringup_panic_(const char *p_msg)
{
    FM_DEBUG_PanicMsg(p_msg);
}

static void fm_lcd_blink_bringup_require_ok_(fm_lcd_status_t p_status,
                                             const char *p_msg)
{
    if (p_status != FM_LCD_OK)
    {
        fm_lcd_blink_bringup_panic_(p_msg);
    }
}

static void fm_lcd_blink_bringup_require_policy_ok_(fm_lcd_blink_status_t p_status,
                                                    const char *p_msg)
{
    if (p_status != FM_LCD_BLINK_OK)
    {
        fm_lcd_blink_bringup_panic_(p_msg);
    }
}

static void fm_lcd_blink_bringup_prepare_scene_(void)
{
    fm_lcd_blink_bringup_require_ok_(FM_LCD_Clear(), "LCD_BLINK_BRINGUP:CLEAR_FAIL\n");
    fm_lcd_blink_bringup_require_ok_(FM_LCD_BlinkClear(),
                                     "LCD_BLINK_BRINGUP:BLINK_CLEAR_FAIL\n");
}

static void fm_lcd_blink_bringup_apply_scene_(void)
{
    fm_lcd_blink_bringup_prepare_scene_();

    fm_lcd_blink_bringup_require_ok_(FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_TOP,
                                                      "HELLO123",
                                                      FM_LCD_ALIGN_LEFT,
                                                      true),
                                     "LCD_BLINK_BRINGUP:ROW_TOP_FAIL\n");
    fm_lcd_blink_bringup_require_ok_(FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_BOTTOM,
                                                      "WORLD42",
                                                      FM_LCD_ALIGN_LEFT,
                                                      true),
                                     "LCD_BLINK_BRINGUP:ROW_BOTTOM_FAIL\n");
    fm_lcd_blink_bringup_require_ok_(FM_LCD_WriteAlpha("HI",
                                                       FM_LCD_ALIGN_LEFT,
                                                       true),
                                     "LCD_BLINK_BRINGUP:ALPHA_FAIL\n");
    fm_lcd_blink_bringup_require_ok_(
        FM_LCD_BlinkSetRanges(g_fm_lcd_blink_bringup_ranges,
                              (uint8_t) (sizeof(g_fm_lcd_blink_bringup_ranges) /
                                         sizeof(g_fm_lcd_blink_bringup_ranges[0]))),
        "LCD_BLINK_BRINGUP:BLINK_RANGE_FAIL\n");
    fm_lcd_blink_bringup_require_ok_(FM_LCD_BlinkSetEnabled(true),
                                     "LCD_BLINK_BRINGUP:BLINK_ENABLE_FAIL\n");
}

static void fm_lcd_blink_bringup_apply_phase_(fm_lcd_blink_phase_t p_phase)
{
    if (p_phase == FM_LCD_BLINK_PHASE_ON)
    {
        (void) FM_DEBUG_UartStr(
            "LCD_BLINK_BRINGUP:SCENE_HELLO_WORLD PHASE=ON TOP=HELLO123 BOT=WORLD42 ALPHA=HI\n");
    }
    else
    {
        (void) FM_DEBUG_UartStr(
            "LCD_BLINK_BRINGUP:SCENE_HELLO_WORLD PHASE=OFF TOP=_____123 BOT=_____42 ALPHA=__\n");
    }

    fm_lcd_blink_bringup_require_ok_(FM_LCD_BlinkSetPhase(p_phase),
                                     "LCD_BLINK_BRINGUP:BLINK_PHASE_FAIL\n");
    fm_lcd_blink_bringup_require_ok_(FM_LCD_Flush(), "LCD_BLINK_BRINGUP:FLUSH_FAIL\n");
    FM_DEBUG_Flush();
}

/* =========================== Public Bodies ============================== */
void FM_LcdBlinkBringup_Run(void)
{
    fm_lcd_blink_timing_t timing;
    fm_lcd_blink_phase_t phase;
    uint32_t wait_ms;

    FM_BOARD_Init();
    FM_DEBUG_Init();

    (void) FM_DEBUG_UartStr("LCD_BLINK_BRINGUP:START\n");

    fm_lcd_blink_bringup_require_ok_(FM_LCD_Init(), "LCD_BLINK_BRINGUP:LCD_INIT_FAIL\n");

    (void) FM_DEBUG_UartStr("LCD_BLINK_BRINGUP:LCD_INIT_OK\n");
    FM_DEBUG_LedRun(FM_DEBUG_LED_ON);

    fm_lcd_blink_bringup_apply_scene_();

    timing.on_ms = FM_LCD_BLINK_BRINGUP_ON_MS;
    timing.off_ms = FM_LCD_BLINK_BRINGUP_OFF_MS;

    fm_lcd_blink_bringup_require_policy_ok_(
        FM_LCD_BLINK_Start(&timing,
                           FM_LCD_BLINK_PHASE_ON,
                           &phase,
                           &wait_ms),
        "LCD_BLINK_BRINGUP:POLICY_START_FAIL\n");

    for (;;)
    {
        fm_lcd_blink_bringup_apply_phase_(phase);
        FM_PORT_TIME_SleepMs(wait_ms);

        fm_lcd_blink_bringup_require_policy_ok_(
            FM_LCD_BLINK_Advance(&phase, &wait_ms),
            "LCD_BLINK_BRINGUP:POLICY_ADVANCE_FAIL\n");
    }
}

/*** end of file ***/
