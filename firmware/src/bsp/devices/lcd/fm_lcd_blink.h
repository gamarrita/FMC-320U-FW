/**
 * @file    fm_lcd_blink.h
 * @brief   Timer-neutral companion blink policy for the LCD redesign path.
 */

#ifndef FM_LCD_BLINK_H_
#define FM_LCD_BLINK_H_

#include <stdint.h>

#include "fm_lcd_types.h"

/* =========================== Public Types ============================== */
typedef enum
{
    FM_LCD_BLINK_OK = 0,
    FM_LCD_BLINK_EINVAL,
    FM_LCD_BLINK_ESTATE
} fm_lcd_blink_status_t;

/* =========================== Public API ================================ */
/**
 * @brief Start or restart one logical blink program.
 *
 * This module owns phase sequencing and timing values only. It does not own
 * the real timer primitive or any LCD-visible state.
 *
 * @param[in]  p_timing Blink timing to store.
 * @param[in]  p_initial_phase Initial phase to enter.
 * @param[out] p_phase_out Phase that should be applied immediately by the
 *                         caller.
 * @param[out] p_wait_ms_out Delay until the next phase transition.
 *
 * @return FM_LCD_BLINK_OK on success.
 * @return FM_LCD_BLINK_EINVAL on invalid arguments or invalid timing values.
 */
fm_lcd_blink_status_t FM_LCD_BLINK_Start(const fm_lcd_blink_timing_t *p_timing,
                                         fm_lcd_blink_phase_t p_initial_phase,
                                         fm_lcd_blink_phase_t *p_phase_out,
                                         uint32_t *p_wait_ms_out);

/**
 * @brief Advance one active logical blink program to its next phase.
 *
 * This should be called by whichever external timing mechanism expires next.
 *
 * @param[out] p_phase_out New phase to apply.
 * @param[out] p_wait_ms_out Delay until the next phase transition.
 *
 * @return FM_LCD_BLINK_OK on success.
 * @return FM_LCD_BLINK_EINVAL on invalid output pointers.
 * @return FM_LCD_BLINK_ESTATE when no blink program is active.
 */
fm_lcd_blink_status_t FM_LCD_BLINK_Advance(fm_lcd_blink_phase_t *p_phase_out,
                                           uint32_t *p_wait_ms_out);

/**
 * @brief Stop the active logical blink program.
 *
 * Stopping forces the output phase to `FM_LCD_BLINK_PHASE_ON` so the caller can
 * leave highlighted text visible.
 *
 * @param[out] p_phase_out Phase to apply after stop.
 *
 * @return FM_LCD_BLINK_OK on success.
 * @return FM_LCD_BLINK_EINVAL when `p_phase_out` is NULL.
 * @return FM_LCD_BLINK_ESTATE when no blink program is active.
 */
fm_lcd_blink_status_t FM_LCD_BLINK_Stop(fm_lcd_blink_phase_t *p_phase_out);

#endif /* FM_LCD_BLINK_H_ */

/*** end of file ***/
