/**
 * @file    fm_lcd_ll_bringup.h
 * @brief   Bring-up entry point for the legacy LCD low-level driver.
 */

#ifndef FM_LCD_LL_BRINGUP_H_
#define FM_LCD_LL_BRINGUP_H_

/**
 * @brief Run the minimal LCD LL bring-up sequence.
 *
 * @details
 *  - Initializes board and debug services.
 *  - Exercises a small set of visible legacy LCD patterns.
 *  - Emits stable UART checkpoints intended for human and agent verification.
 *  - Intentionally stays app-local and does not define lower-layer contracts.
 */
void FM_LcdLlBringup_Run(void);

#endif /* FM_LCD_LL_BRINGUP_H_ */
