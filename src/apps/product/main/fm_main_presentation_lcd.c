/**
 * @file    fm_main_presentation_lcd.c
 * @brief   Product-main semantic presentation to LCD adapter.
 */

#include "fm_main_presentation_lcd.h"

#include <stdbool.h>
#include <stddef.h>

#include "devices/lcd/fm_lcd.h"

static fm_status_t fm_main_presentation_lcd_from_status_(
    fm_lcd_status_t p_status);
static fm_status_t fm_main_presentation_lcd_require_(
    fm_lcd_status_t p_status);
static fm_status_t fm_main_presentation_lcd_set_indicator_(
    fm_lcd_layout_indicator_t p_indicator,
    bool p_on);

fm_status_t FM_MAIN_PRESENTATION_LCD_Init(void)
{
    return fm_main_presentation_lcd_from_status_(FM_LCD_Init());
}

fm_status_t FM_MAIN_PRESENTATION_LCD_Present(
    const fmc_presentation_frame_t *p_frame,
    void *p_context)
{
    fm_status_t status;

    (void) p_context;

    if (p_frame == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    if (p_frame->all_segments)
    {
        status = fm_main_presentation_lcd_require_(
            FM_LCD_SetAllSegments());
        if (status != FM_STATUS_OK)
        {
            return status;
        }

        return fm_main_presentation_lcd_from_status_(FM_LCD_Flush());
    }

    status = fm_main_presentation_lcd_require_(FM_LCD_Clear());
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = fm_main_presentation_lcd_require_(
        FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_TOP,
                         p_frame->top_text,
                         FM_LCD_ALIGN_LEFT,
                         true));
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = fm_main_presentation_lcd_require_(
        FM_LCD_WriteText(FM_LCD_LAYOUT_ROW_BOTTOM,
                         p_frame->bottom_text,
                         FM_LCD_ALIGN_LEFT,
                         true));
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = fm_main_presentation_lcd_require_(
        FM_LCD_WriteAlpha(p_frame->alpha_text,
                          FM_LCD_ALIGN_LEFT,
                          true));
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = fm_main_presentation_lcd_set_indicator_(
        FM_LCD_LAYOUT_INDICATOR_TTL,
        p_frame->indicator_ttl);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = fm_main_presentation_lcd_set_indicator_(
        FM_LCD_LAYOUT_INDICATOR_RATE,
        p_frame->indicator_rate);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = fm_main_presentation_lcd_set_indicator_(
        FM_LCD_LAYOUT_INDICATOR_SLASH,
        p_frame->indicator_slash);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = fm_main_presentation_lcd_set_indicator_(
        FM_LCD_LAYOUT_INDICATOR_S,
        p_frame->indicator_second);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = fm_main_presentation_lcd_set_indicator_(
        FM_LCD_LAYOUT_INDICATOR_M,
        p_frame->indicator_minute);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    return fm_main_presentation_lcd_from_status_(FM_LCD_Flush());
}

static fm_status_t fm_main_presentation_lcd_from_status_(
    fm_lcd_status_t p_status)
{
    switch (p_status)
    {
    case FM_LCD_OK:
        return FM_STATUS_OK;
    case FM_LCD_EINVAL:
        return FM_STATUS_EINVAL;
    case FM_LCD_ERANGE:
        return FM_STATUS_ERANGE;
    case FM_LCD_ESTATE:
    case FM_LCD_EIO:
    default:
        return FM_STATUS_ESTATE;
    }
}

static fm_status_t fm_main_presentation_lcd_require_(
    fm_lcd_status_t p_status)
{
    return fm_main_presentation_lcd_from_status_(p_status);
}

static fm_status_t fm_main_presentation_lcd_set_indicator_(
    fm_lcd_layout_indicator_t p_indicator,
    bool p_on)
{
    return fm_main_presentation_lcd_require_(
        FM_LCD_SetIndicator(p_indicator, p_on));
}
