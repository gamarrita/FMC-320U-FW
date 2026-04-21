/**
 * @file    fm_lcd_types.h
 * @brief   Shared public LCD types for the redesign path.
 *
 * This header exists only to host small public LCD types that are shared across
 * the new LCD modules without forcing an upward include dependency.
 *
 * It should stay narrow.
 * It is not a catch-all header for unrelated definitions.
 */

#ifndef FM_LCD_TYPES_H_
#define FM_LCD_TYPES_H_

#include <stdint.h>

/* =========================== Public Types ============================== */
/**
 * @brief Horizontal alignment policy for multi-column LCD field writes.
 *
 * This alignment is intended for public write operations that render content
 * into visible fields with more than one character position, such as numeric
 * rows handled by `FM_LCD_WriteText()` and the mapping operations that support
 * that public behavior.
 */
typedef enum
{
    FM_LCD_ALIGN_LEFT = 0,
    FM_LCD_ALIGN_RIGHT
} fm_lcd_align_t;

/**
 * @brief Visible blink phase for logical LCD blink behavior.
 *
 * `ON` means the selected text is shown normally.
 * `OFF` means the selected text is hidden.
 */
typedef enum
{
    FM_LCD_BLINK_PHASE_ON = 0,
    FM_LCD_BLINK_PHASE_OFF
} fm_lcd_blink_phase_t;

/**
 * @brief On/off timing parameters for one logical blink program.
 *
 * Timer ownership is intentionally external to the LCD core. These values are
 * shared so a companion blink-policy module can remain timer-neutral while
 * still using LCD-facing blink vocabulary.
 */
typedef struct
{
    uint32_t on_ms;
    uint32_t off_ms;
} fm_lcd_blink_timing_t;

#endif /* FM_LCD_TYPES_H_ */

/*** end of file ***/
