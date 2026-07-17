/**
 * @file    fm_display_format_lcd_bringup.h
 * @brief   Human-observed display-format to LCD bring-up.
 *
 * This app validates the boundary between `display_format` output strings and
 * the LCD text API by cycling through fixed UART-labeled scenes on hardware.
 */
#ifndef FM_DISPLAY_FORMAT_LCD_BRINGUP_H
#define FM_DISPLAY_FORMAT_LCD_BRINGUP_H

/**
 * @brief Run the display-format/LCD bring-up loop.
 *
 * Initializes board/debug/LCD services, formats each predefined scene, writes
 * it to the LCD, emits the expected UART label, and panics on unexpected
 * formatter or LCD status.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void FM_DisplayFormatLcdBringup_Run(void);

#endif /* FM_DISPLAY_FORMAT_LCD_BRINGUP_H */
