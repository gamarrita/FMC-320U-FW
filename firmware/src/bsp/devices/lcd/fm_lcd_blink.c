/**
 * @file    fm_lcd_blink.c
 * @brief   Timer-neutral companion blink policy for the LCD redesign path.
 */

#include "fm_lcd_blink.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* =========================== Private Types ============================= */
typedef struct
{
    bool active;
    fm_lcd_blink_timing_t timing;
    fm_lcd_blink_phase_t phase;
} fm_lcd_blink_context_t;

/* =========================== Private Data ============================== */
static fm_lcd_blink_context_t g_fm_lcd_blink_context;

/* =========================== Private Prototypes ======================== */
static bool fm_lcd_blink_phase_is_valid_(fm_lcd_blink_phase_t p_phase);
static uint32_t fm_lcd_blink_phase_wait_ms_(fm_lcd_blink_phase_t p_phase);

/* =========================== Private Bodies ============================ */
static bool fm_lcd_blink_phase_is_valid_(fm_lcd_blink_phase_t p_phase)
{
    return ((p_phase == FM_LCD_BLINK_PHASE_ON) ||
            (p_phase == FM_LCD_BLINK_PHASE_OFF));
}

static uint32_t fm_lcd_blink_phase_wait_ms_(fm_lcd_blink_phase_t p_phase)
{
    if (p_phase == FM_LCD_BLINK_PHASE_ON)
    {
        return g_fm_lcd_blink_context.timing.on_ms;
    }

    return g_fm_lcd_blink_context.timing.off_ms;
}

/* =========================== Public Bodies ============================= */
fm_lcd_blink_status_t FM_LCD_BLINK_Start(const fm_lcd_blink_timing_t *p_timing,
                                         fm_lcd_blink_phase_t p_initial_phase,
                                         fm_lcd_blink_phase_t *p_phase_out,
                                         uint32_t *p_wait_ms_out)
{
    if ((p_timing == NULL) || (p_phase_out == NULL) || (p_wait_ms_out == NULL))
    {
        return FM_LCD_BLINK_EINVAL;
    }

    if ((p_timing->on_ms == 0U) || (p_timing->off_ms == 0U))
    {
        return FM_LCD_BLINK_EINVAL;
    }

    if (!fm_lcd_blink_phase_is_valid_(p_initial_phase))
    {
        return FM_LCD_BLINK_EINVAL;
    }

    g_fm_lcd_blink_context.active = true;
    g_fm_lcd_blink_context.timing = *p_timing;
    g_fm_lcd_blink_context.phase = p_initial_phase;

    *p_phase_out = g_fm_lcd_blink_context.phase;
    *p_wait_ms_out = fm_lcd_blink_phase_wait_ms_(g_fm_lcd_blink_context.phase);

    return FM_LCD_BLINK_OK;
}

fm_lcd_blink_status_t FM_LCD_BLINK_Advance(fm_lcd_blink_phase_t *p_phase_out,
                                           uint32_t *p_wait_ms_out)
{
    if ((p_phase_out == NULL) || (p_wait_ms_out == NULL))
    {
        return FM_LCD_BLINK_EINVAL;
    }

    if (!g_fm_lcd_blink_context.active)
    {
        return FM_LCD_BLINK_ESTATE;
    }

    if (g_fm_lcd_blink_context.phase == FM_LCD_BLINK_PHASE_ON)
    {
        g_fm_lcd_blink_context.phase = FM_LCD_BLINK_PHASE_OFF;
    }
    else
    {
        g_fm_lcd_blink_context.phase = FM_LCD_BLINK_PHASE_ON;
    }

    *p_phase_out = g_fm_lcd_blink_context.phase;
    *p_wait_ms_out = fm_lcd_blink_phase_wait_ms_(g_fm_lcd_blink_context.phase);

    return FM_LCD_BLINK_OK;
}

fm_lcd_blink_status_t FM_LCD_BLINK_Stop(fm_lcd_blink_phase_t *p_phase_out)
{
    if (p_phase_out == NULL)
    {
        return FM_LCD_BLINK_EINVAL;
    }

    if (!g_fm_lcd_blink_context.active)
    {
        return FM_LCD_BLINK_ESTATE;
    }

    g_fm_lcd_blink_context.active = false;
    g_fm_lcd_blink_context.phase = FM_LCD_BLINK_PHASE_ON;
    *p_phase_out = FM_LCD_BLINK_PHASE_ON;

    return FM_LCD_BLINK_OK;
}

/*** end of file ***/
