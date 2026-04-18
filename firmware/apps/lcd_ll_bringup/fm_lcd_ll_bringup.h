/**
 * @file    fm_lcd_ll_bringup.h
 * @brief   Bring-up entry point for validating the low-level LCD driver.
 */

#ifndef FM_LCD_LL_BRINGUP_H_
#define FM_LCD_LL_BRINGUP_H_

/**
 * @brief Run the staged LCD LL bring-up validation sequence.
 *
 * @details
 *  - Initializes board and debug services.
 *  - Exercises the LCD elements currently confirmed by the YAML source of truth.
 *  - Emits stable UART checkpoints intended for human and agent verification.
 *  - Includes the visually confirmed ACM_1 symbol in the staged validation flow.
 */
void FM_LcdLlBringup_Run(void);

#endif /* FM_LCD_LL_BRINGUP_H_ */
