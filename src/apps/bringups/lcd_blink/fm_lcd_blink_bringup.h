/**
 * @file    fm_lcd_blink_bringup.h
 * @brief   Dedicated blink bring-up flow for the redesigned LCD stack.
 *
 * This app initializes board/debug/LCD services, applies one LCD scene, and
 * toggles logical blink phase through a timer-neutral blink policy plus
 * blocking foreground delays.
 */

#ifndef FM_LCD_BLINK_BRINGUP_H_
#define FM_LCD_BLINK_BRINGUP_H_

/**
 * @brief Run the LCD blink human-observed bring-up.
 *
 * Emits UART scene/phase labels, flushes the LCD after each phase change, and
 * panics through `FM_DEBUG_PanicMsg()` on unexpected LCD or blink-policy
 * failures.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void FM_LcdBlinkBringup_Run(void);

#endif /* FM_LCD_BLINK_BRINGUP_H_ */

/*** end of file ***/
