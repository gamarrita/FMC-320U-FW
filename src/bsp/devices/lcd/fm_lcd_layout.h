/**
 * @file    fm_lcd_layout.h
 * @brief   Interim semantic LCD glass contract for the redesign path.
 *
 * This header captures the layout-specific part of the active LCD redesign
 * contract currently described in `docs/contexts/lcd_redesign.md`.
 *
 * Current module intent:
 * - model the LCD glass as coherent visible fields
 * - provide stable naming for rows, alpha digits, and indicators
 * - preserve practical traceability to the LCD datasheet
 * - stay independent from RAM bit mapping and hardware I/O
 *
 * Current semantic direction:
 * - one top numeric row with 8 visible character positions
 * - one bottom numeric row with 7 visible character positions
 * - one pair of 14-segment alphanumeric characters
 * - one set of standalone indicators and product-specific legends
 *
 * Important rule while this contract is active:
 * - public naming should prefer meaning over raw datasheet labels
 * - datasheet labels should remain traceable through comments or mapping data
 * - old inconsistent field treatment should not be carried forward
 */

#ifndef FM_LCD_LAYOUT_H_
#define FM_LCD_LAYOUT_H_

#include <stdbool.h>
#include <stdint.h>

/* =========================== Public Macros ============================= */
#define FM_LCD_LAYOUT_TOP_ROW_COLUMNS        8U
#define FM_LCD_LAYOUT_BOTTOM_ROW_COLUMNS     7U
#define FM_LCD_LAYOUT_ALPHA_DIGIT_COUNT      2U
#define FM_LCD_LAYOUT_INDICATOR_COUNT        14U
#define FM_LCD_LAYOUT_MAX_ROW_COLUMNS        FM_LCD_LAYOUT_TOP_ROW_COLUMNS

/* =========================== Public Types ============================== */
typedef enum
{
    FM_LCD_LAYOUT_FIELD_TOP_ROW = 0,
    FM_LCD_LAYOUT_FIELD_BOTTOM_ROW,
    FM_LCD_LAYOUT_FIELD_ALPHA_PAIR,
    FM_LCD_LAYOUT_FIELD_INDICATORS
} fm_lcd_layout_field_t;

typedef enum
{
    FM_LCD_LAYOUT_ROW_TOP = 0,
    FM_LCD_LAYOUT_ROW_BOTTOM
} fm_lcd_layout_row_t;

typedef enum
{
    FM_LCD_LAYOUT_ALPHA_LEFT = 0,
    FM_LCD_LAYOUT_ALPHA_RIGHT
} fm_lcd_layout_alpha_digit_t;

typedef enum
{
    /*
     * Standalone round point at the upper-left area of the glass.
     * Keep this distinct from per-row decimal points.
     * Datasheet reference: X6.
     */
    FM_LCD_LAYOUT_INDICATOR_POINT = 0,

    /*
     * Battery icon at the lower-left area of the glass.
     * Datasheet reference: X12.
     */
    FM_LCD_LAYOUT_INDICATOR_BATTERY,

    /*
     * Product legend indicators.
     * Public naming uses visible meaning.
     * Comments preserve datasheet or legacy traceability where useful.
     */
    FM_LCD_LAYOUT_INDICATOR_POWER,
    FM_LCD_LAYOUT_INDICATOR_RATE,
    FM_LCD_LAYOUT_INDICATOR_E,
    FM_LCD_LAYOUT_INDICATOR_BATCH,

    /* Upper ACM legend. Legacy traceability: ACM_1. */
    FM_LCD_LAYOUT_INDICATOR_ACM_TOP,

    FM_LCD_LAYOUT_INDICATOR_TTL,
    FM_LCD_LAYOUT_INDICATOR_SLASH,

    /* Lower ACM legend. Legacy traceability: ACM_2. */
    FM_LCD_LAYOUT_INDICATOR_ACM_BOTTOM,

    FM_LCD_LAYOUT_INDICATOR_H,
    FM_LCD_LAYOUT_INDICATOR_D,
    FM_LCD_LAYOUT_INDICATOR_S,
    FM_LCD_LAYOUT_INDICATOR_M
} fm_lcd_layout_indicator_t;

/* =========================== Public API ================================ */
/**
 * @brief Return the visible column count for one numeric row.
 *
 * Column indexing for public LCD semantics is left-to-right in visible order.
 * This is intentional even if the physical glass wiring uses a different order.
 *
 * Top row traceability:
 * - visible positions correspond to datasheet digits 10..17
 *
 * Bottom row traceability:
 * - visible positions correspond to datasheet digits 1..7
 *
 * @param[in] p_row Target numeric row.
 *
 * @return Visible column count for the row.
 * @return 0 when the row value is invalid.
 */
uint8_t FM_LCD_LAYOUT_GetRowColumns(fm_lcd_layout_row_t p_row);

/**
 * @brief Return the number of decimal point positions for one numeric row.
 *
 * Decimal points belong semantically to numeric rows and are not modeled as
 * standalone indicators.
 *
 * @param[in] p_row Target numeric row.
 *
 * @return Decimal point count for the row.
 * @return 0 when the row value is invalid.
 */
uint8_t FM_LCD_LAYOUT_GetRowDecimalPointCount(fm_lcd_layout_row_t p_row);

/**
 * @brief Check whether one row column index is valid in visible order.
 *
 * @param[in] p_row Target numeric row.
 * @param[in] p_col Zero-based visible column index.
 *
 * @return true when the column exists in the selected row.
 * @return false otherwise.
 */
bool FM_LCD_LAYOUT_IsRowColumnValid(fm_lcd_layout_row_t p_row, uint8_t p_col);

/**
 * @brief Check whether one decimal point index is valid in visible order.
 *
 * Decimal point indexing follows the visible left-to-right order of the row.
 *
 * @param[in] p_row Target numeric row.
 * @param[in] p_col Zero-based decimal point index in visible order.
 *
 * @return true when the decimal point exists in the selected row.
 * @return false otherwise.
 */
bool FM_LCD_LAYOUT_IsRowDecimalPointValid(fm_lcd_layout_row_t p_row, uint8_t p_col);

#endif /* FM_LCD_LAYOUT_H_ */

/*** end of file ***/
