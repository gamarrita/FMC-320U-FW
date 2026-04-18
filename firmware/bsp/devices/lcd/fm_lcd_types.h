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

#endif /* FM_LCD_TYPES_H_ */

/*** end of file ***/
