/**
 * @file    fm_lcd_bringup.h
 * @brief   Minimal bring-up flow for the redesigned LCD stack.
 *
 * This app initializes board/debug/LCD services and cycles through UART-labeled
 * LCD scenes intended for human visual validation.
 */

#ifndef FM_LCD_BRINGUP_H_
#define FM_LCD_BRINGUP_H_

/**
 * @brief Run the LCD human-observed bring-up scene loop.
 *
 * Each scene writes expected LCD content, flushes it to hardware, emits a UART
 * label, and waits before moving to the next scene. Unexpected LCD failures
 * enter the shared panic path.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void FM_LcdBringup_Run(void);

#endif /* FM_LCD_BRINGUP_H_ */

/*** end of file ***/
