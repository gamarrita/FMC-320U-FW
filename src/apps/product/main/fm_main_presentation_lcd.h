/**
 * @file    fm_main_presentation_lcd.h
 * @brief   Product-main adapter from semantic presentation frames to LCD.
 */

#ifndef FM_MAIN_PRESENTATION_LCD_H
#define FM_MAIN_PRESENTATION_LCD_H

#include "fm_status.h"
#include "fmc_presentation.h"

/**
 * @brief Initialize the LCD used by the product presentation.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_ESTATE` when LCD initialization fails.
 */
fm_status_t FM_MAIN_PRESENTATION_LCD_Init(void);

/**
 * @brief Present one semantic FMC frame and flush it to hardware.
 *
 * The function does not touch backlight state. A failed flush leaves physical
 * LCD content indeterminate and returns failure to the presentation owner.
 *
 * @param p_frame Frame to present.
 * @param p_context Unused callback context.
 *
 * @return `FM_STATUS_OK` only after a successful flush.
 * @return `FM_STATUS_EINVAL` for a null frame.
 * @return `FM_STATUS_ERANGE` for invalid LCD-visible content.
 * @return `FM_STATUS_ESTATE` for LCD state or I/O failures.
 */
fm_status_t FM_MAIN_PRESENTATION_LCD_Present(
    const fmc_presentation_frame_t *p_frame,
    void *p_context);

#endif /* FM_MAIN_PRESENTATION_LCD_H */
