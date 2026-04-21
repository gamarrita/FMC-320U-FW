/**
 * @file    fm_lcd.h
 * @brief   Public V1 LCD contract for the redesign path.
 *
 * This header defines the intended public API of the new LCD stack.
 * It is a contract pass artifact, not the final implementation.
 *
 * Current V1 intent:
 * - expose LCD behavior in semantic display terms
 * - keep setters side-effect free with respect to hardware I/O
 * - make flush explicit
 * - keep the initial public surface small and reviewable
 *
 * Current non-goals for this V1 contract:
 * - compatibility with the legacy `fm_lcd.c` API
 * - rich alphanumeric formatting helpers
 * - integer formatting helpers
 * - hardware-resume and recovery policy details
 * - timer ownership or scheduler integration for blink timing
 */

#ifndef FM_LCD_H_
#define FM_LCD_H_

#include <stdbool.h>
#include <stdint.h>

#include "fm_lcd_layout.h"
#include "fm_lcd_types.h"

/* =========================== Public Types ============================== */
/**
 * @brief Public result codes for the LCD API.
 *
 * Status code intent:
 * - FM_LCD_OK: operation completed successfully
 * - FM_LCD_EINVAL: invalid argument value or invalid argument combination
 * - FM_LCD_ERANGE: argument is outside the visible LCD domain
 * - FM_LCD_ESTATE: module state does not allow the requested operation
 * - FM_LCD_EIO: backend or controller I/O operation failed
 *
 * Contract rule:
 * - functions that only modify desired LCD state should normally report
 *   FM_LCD_OK, FM_LCD_EINVAL, FM_LCD_ERANGE, or FM_LCD_ESTATE
 * - FM_LCD_EIO should be reserved for operations that require backend or
 *   controller interaction
 */
typedef enum
{
    FM_LCD_OK = 0,
    FM_LCD_EINVAL,
    FM_LCD_ERANGE,
    FM_LCD_ESTATE,
    FM_LCD_EIO
} fm_lcd_status_t;

typedef enum
{
    FM_LCD_TEXT_FIELD_TOP_ROW = 0,
    FM_LCD_TEXT_FIELD_BOTTOM_ROW,
    FM_LCD_TEXT_FIELD_ALPHA
} fm_lcd_text_field_t;

typedef struct
{
    fm_lcd_text_field_t field;
    uint8_t start;
    uint8_t length;
} fm_lcd_blink_range_t;

/* =========================== Public Macros ============================ */
#define FM_LCD_BLINK_RANGE_MAX    4U

/* =========================== Public API ================================ */
/**
 * @brief Initialize the LCD module and its controller backend.
 *
 * Initialization prepares internal LCD state and backend ownership for the new
 * driver path. The desired display state starts cleared.
 *
 * This function may touch controller configuration state, but callers should
 * still use `FM_LCD_Flush()` as the explicit point that guarantees the display
 * RAM matches the desired LCD content.
 *
 * @return FM_LCD_OK on success.
 * @return FM_LCD_EIO when the backend cannot be initialized.
 */
fm_lcd_status_t FM_LCD_Init(void);

/**
 * @brief Clear the full desired LCD state.
 *
 * This clears numeric rows, decimal points, indicators, and any other visible
 * fields covered by the current implementation policy.
 *
 * This function updates desired state only. It does not perform display I/O.
 *
 * @return FM_LCD_OK on success.
 * @return FM_LCD_ESTATE when the module is not initialized.
 */
fm_lcd_status_t FM_LCD_Clear(void);

/**
 * @brief Clear one numeric row in desired state.
 *
 * Decimal points associated with the selected row are also cleared.
 *
 * This function updates desired state only. It does not perform display I/O.
 *
 * @param[in] p_row Target numeric row in visible semantic order.
 *
 * @return FM_LCD_OK on success.
 * @return FM_LCD_ERANGE when the row value is invalid.
 * @return FM_LCD_ESTATE when the module is not initialized.
 */
fm_lcd_status_t FM_LCD_ClearRow(fm_lcd_layout_row_t p_row);

/**
 * @brief Clear the full alphanumeric pair in desired state.
 *
 * This clears both visible alpha positions of the semantic alpha field.
 *
 * This function updates desired state only. It does not perform display I/O.
 *
 * @return FM_LCD_OK on success.
 * @return FM_LCD_ESTATE when the module is not initialized.
 */
fm_lcd_status_t FM_LCD_ClearAlpha(void);

/**
 * @brief Clear the full active logical blink selection.
 *
 * This removes every configured blink range, disables blink, and restores the
 * stored blink phase to `FM_LCD_BLINK_PHASE_ON`.
 *
 * This function updates desired LCD behavior only. It does not perform display
 * I/O.
 *
 * @return FM_LCD_OK on success.
 * @return FM_LCD_ESTATE when the module is not initialized.
 */
fm_lcd_status_t FM_LCD_BlinkClear(void);

/**
 * @brief Replace the full active logical blink selection.
 *
 * Blink ranges describe visible character cells only:
 * - top numeric row columns
 * - bottom numeric row columns
 * - alpha pair positions
 *
 * Overlapping ranges are allowed and behave as their visible union.
 *
 * This function updates desired LCD behavior only. It does not perform display
 * I/O.
 *
 * @param[in] p_ranges Blink range array.
 * @param[in] p_range_count Number of entries in `p_ranges`.
 *
 * @return FM_LCD_OK on success.
 * @return FM_LCD_EINVAL when `p_ranges` is NULL, `p_range_count` is zero, or
 *         `p_range_count` exceeds `FM_LCD_BLINK_RANGE_MAX`.
 * @return FM_LCD_ERANGE when any range exceeds the visible LCD text domain.
 * @return FM_LCD_ESTATE when the module is not initialized.
 */
fm_lcd_status_t FM_LCD_BlinkSetRanges(const fm_lcd_blink_range_t *p_ranges,
                                      uint8_t p_range_count);

/**
 * @brief Enable or disable logical blink over the configured blink selection.
 *
 * When enabled, the current blink phase applies to the configured blink
 * ranges. When disabled, the blink selection remains stored but has no visible
 * effect until re-enabled.
 *
 * This function updates desired LCD behavior only. It does not perform display
 * I/O.
 *
 * @param[in] p_enabled Enable state to apply.
 *
 * @return FM_LCD_OK on success.
 * @return FM_LCD_ESTATE when the module is not initialized or when enabling is
 *         requested without a configured blink selection.
 */
fm_lcd_status_t FM_LCD_BlinkSetEnabled(bool p_enabled);

/**
 * @brief Store the current logical blink phase.
 *
 * `FM_LCD_BLINK_PHASE_ON` means the selected text is shown normally.
 * `FM_LCD_BLINK_PHASE_OFF` means the selected text is hidden.
 *
 * This function updates desired LCD behavior only. It does not perform display
 * I/O.
 *
 * @param[in] p_phase Blink phase to store.
 *
 * @return FM_LCD_OK on success.
 * @return FM_LCD_EINVAL when `p_phase` is invalid.
 * @return FM_LCD_ESTATE when the module is not initialized.
 */
fm_lcd_status_t FM_LCD_BlinkSetPhase(fm_lcd_blink_phase_t p_phase);

/**
 * @brief Write text into one numeric row.
 *
 * Supported characters are rendered according to the current numeric field
 * encoding. Unsupported characters are rendered as blanks.
 *
 * A '.' character enables the decimal point of the previous visible numeric
 * character. A leading '.' with no previous visible character has no effect.
 *
 * Text that does not fit in the row is clipped according to visible row order.
 *
 * This function updates desired state only. It does not perform display I/O.
 *
 * @param[in] p_row        Target numeric row in visible semantic order.
 * @param[in] p_text       Null-terminated input text.
 * @param[in] p_align      Left or right alignment within the row.
 * @param[in] p_clear_rest When true, cells not covered by the rendered text are
 *                         cleared.
 *
 * @return FM_LCD_OK on success.
 * @return FM_LCD_EINVAL when `p_text` is NULL.
 * @return FM_LCD_ERANGE when the row value is invalid.
 * @return FM_LCD_ESTATE when the module is not initialized.
 */
fm_lcd_status_t FM_LCD_WriteText(fm_lcd_layout_row_t p_row,
                                 const char *p_text,
                                 fm_lcd_align_t p_align,
                                 bool p_clear_rest);

/**
 * @brief Write text into the visible alphanumeric pair.
 *
 * The alpha field is modeled as one visible field with two positions:
 * - left alpha digit
 * - right alpha digit
 *
 * Up to two visible characters are rendered. When a single visible character
 * is rendered, `p_align` decides whether it lands on the left or right alpha
 * position. Unsupported characters are rendered as blank.
 *
 * This function updates desired state only. It does not perform display I/O.
 *
 * @param[in] p_text       Null-terminated input text.
 * @param[in] p_align      Left or right alignment within the alpha pair.
 * @param[in] p_clear_rest When true, alpha positions not covered by the
 *                         rendered text are cleared.
 *
 * @return FM_LCD_OK on success.
 * @return FM_LCD_EINVAL when `p_text` is NULL.
 * @return FM_LCD_ESTATE when the module is not initialized.
 */
fm_lcd_status_t FM_LCD_WriteAlpha(const char *p_text,
                                  fm_lcd_align_t p_align,
                                  bool p_clear_rest);

/**
 * @brief Set or clear one standalone indicator in desired state.
 *
 * This function updates desired state only. It does not perform display I/O.
 *
 * @param[in] p_indicator Target indicator.
 * @param[in] p_on        Indicator state to apply in desired state.
 *
 * @return FM_LCD_OK on success.
 * @return FM_LCD_ERANGE when the indicator value is invalid.
 * @return FM_LCD_ESTATE when the module is not initialized.
 */
fm_lcd_status_t FM_LCD_SetIndicator(fm_lcd_layout_indicator_t p_indicator,
                                    bool p_on);

/**
 * @brief Flush the desired LCD state to hardware if needed.
 *
 * The implementation is expected to compare the currently visible state
 * against the last successfully flushed state and avoid unnecessary writes.
 *
 * @return FM_LCD_OK on success.
 * @return FM_LCD_ESTATE when the module is not initialized.
 * @return FM_LCD_EIO on communication or backend write failure.
 */
fm_lcd_status_t FM_LCD_Flush(void);

/**
 * @brief Report whether the desired visible LCD state is pending flush.
 *
 * @return true when the visible LCD result differs from the last flushed state.
 * @return false otherwise.
 */
bool FM_LCD_IsDirty(void);

/**
 * @brief Return the visible column count for one numeric row.
 *
 * This query follows the semantic row model defined by `fm_lcd_layout.h`.
 *
 * @param[in] p_row Target numeric row.
 *
 * @return Visible column count for the row.
 * @return 0 when the row value is invalid.
 */
uint8_t FM_LCD_GetRowColumns(fm_lcd_layout_row_t p_row);

#endif /* FM_LCD_H_ */

/*** end of file ***/
