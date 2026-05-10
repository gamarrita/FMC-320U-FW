/**
 * @file    fm_lcd_map.h
 * @brief   Pure LCD glass-to-RAM mapping contract for the redesign path.
 *
 * This header defines the contract of the mapping layer used by the new LCD
 * stack. The mapping layer converts semantic LCD elements into RAM bit updates
 * on caller-provided buffers.
 *
 * Current module intent:
 * - keep the glass mapping pure and explicit
 * - keep encoding/font rules local to the mapping layer
 * - expose small semantic cell operations when they improve mapping clarity
 * - avoid hardware I/O and runtime ownership concerns
 * - stay aligned with the semantic vocabulary defined in `fm_lcd_layout.h`
 *
 * Current non-goals:
 * - backend/controller communication
 * - dirty tracking
 * - blink timing or phase policy
 * - module initialization state
 */

#ifndef FM_LCD_MAP_H_
#define FM_LCD_MAP_H_

#include <stdbool.h>
#include <stdint.h>

#include "fm_lcd_layout.h"
#include "fm_lcd_types.h"

/* =========================== Public Macros ============================= */
#define FM_LCD_MAP_RAM_SIZE    20U

/* =========================== Public Types ============================== */
/**
 * @brief Result codes returned by the pure LCD mapping layer.
 *
 * Contract rule:
 * - the mapping layer is pure and does not perform controller I/O
 * - mapping functions therefore do not report runtime state or transport
 *   failures
 * - unsupported render cases should be handled explicitly by the mapping layer
 *   instead of leaking ambiguity upward
 */
typedef enum
{
    FM_LCD_MAP_OK = 0,
    FM_LCD_MAP_EINVAL,
    FM_LCD_MAP_ERANGE,
    FM_LCD_MAP_ENOTSUP
} fm_lcd_map_status_t;

/* =========================== Public API ================================ */
/**
 * @brief Clear the full LCD RAM image represented by the provided buffer.
 *
 * This function clears the mapped RAM image only. It performs no hardware I/O.
 *
 * @param[in,out] p_ram Target RAM buffer.
 * @param[in]     p_ram_size Size in bytes of `p_ram`.
 *
 * @return FM_LCD_MAP_OK on success.
 * @return FM_LCD_MAP_EINVAL when `p_ram` is NULL.
 * @return FM_LCD_MAP_ERANGE when `p_ram_size` is smaller than
 *         `FM_LCD_MAP_RAM_SIZE`.
 */
fm_lcd_map_status_t FM_LCD_MAP_Clear(uint8_t *p_ram, uint8_t p_ram_size);

/**
 * @brief Clear one numeric row and its decimal points in the provided buffer.
 *
 * This function operates in visible semantic row order.
 *
 * @param[in,out] p_ram Target RAM buffer.
 * @param[in]     p_ram_size Size in bytes of `p_ram`.
 * @param[in]     p_row Target numeric row.
 *
 * @return FM_LCD_MAP_OK on success.
 * @return FM_LCD_MAP_EINVAL when `p_ram` is NULL.
 * @return FM_LCD_MAP_ERANGE when the buffer is too small or `p_row` is invalid.
 */
fm_lcd_map_status_t FM_LCD_MAP_ClearRow(uint8_t *p_ram,
                                        uint8_t p_ram_size,
                                        fm_lcd_layout_row_t p_row);

/**
 * @brief Clear the full alphanumeric pair in the provided buffer.
 *
 * This clears both visible alpha positions of the semantic alpha field.
 * This function operates only on the RAM image in `p_ram`.
 *
 * @param[in,out] p_ram Target RAM buffer.
 * @param[in]     p_ram_size Size in bytes of `p_ram`.
 *
 * @return FM_LCD_MAP_OK on success.
 * @return FM_LCD_MAP_EINVAL when `p_ram` is NULL.
 * @return FM_LCD_MAP_ERANGE when `p_ram_size` is smaller than
 *         `FM_LCD_MAP_RAM_SIZE`.
 */
fm_lcd_map_status_t FM_LCD_MAP_ClearAlpha(uint8_t *p_ram,
                                          uint8_t p_ram_size);

/**
 * @brief Clear one visible numeric cell in the provided buffer.
 *
 * This function clears only the segments that belong to the selected visible
 * numeric cell. Decimal points remain separate semantic row elements and are
 * not modified by this helper.
 *
 * Column indexing follows the visible left-to-right row order defined by
 * `fm_lcd_layout.h`.
 *
 * @param[in,out] p_ram Target RAM buffer.
 * @param[in]     p_ram_size Size in bytes of `p_ram`.
 * @param[in]     p_row Target numeric row.
 * @param[in]     p_col Zero-based visible column index in the selected row.
 *
 * @return FM_LCD_MAP_OK on success.
 * @return FM_LCD_MAP_EINVAL when `p_ram` is NULL.
 * @return FM_LCD_MAP_ERANGE when the buffer is too small, `p_row` is invalid,
 *         or `p_col` is outside the visible row domain.
 */
fm_lcd_map_status_t FM_LCD_MAP_ClearRowCell(uint8_t *p_ram,
                                            uint8_t p_ram_size,
                                            fm_lcd_layout_row_t p_row,
                                            uint8_t p_col);

/**
 * @brief Clear one visible alphanumeric digit in the provided buffer.
 *
 * This function clears only the selected visible alpha digit in the semantic
 * alpha pair. The other alpha digit is left unchanged.
 *
 * @param[in,out] p_ram Target RAM buffer.
 * @param[in]     p_ram_size Size in bytes of `p_ram`.
 * @param[in]     p_digit Target visible alpha digit.
 *
 * @return FM_LCD_MAP_OK on success.
 * @return FM_LCD_MAP_EINVAL when `p_ram` is NULL.
 * @return FM_LCD_MAP_ERANGE when the buffer is too small or `p_digit` is
 *         invalid.
 */
fm_lcd_map_status_t FM_LCD_MAP_ClearAlphaDigit(uint8_t *p_ram,
                                               uint8_t p_ram_size,
                                               fm_lcd_layout_alpha_digit_t p_digit);

/**
 * @brief Write one text string into one numeric row of the provided buffer.
 *
 * Supported characters are rendered according to the mapping layer encoding
 * tables for the numeric field. Unsupported characters are rendered as blank.
 *
 * A '.' character enables the decimal point of the previous visible numeric
 * character. A leading '.' with no previous visible character has no effect.
 *
 * This function operates only on the RAM image in `p_ram`.
 *
 * @param[in,out] p_ram Target RAM buffer.
 * @param[in]     p_ram_size Size in bytes of `p_ram`.
 * @param[in]     p_row Target numeric row.
 * @param[in]     p_text Null-terminated input text.
 * @param[in]     p_align Alignment policy in visible row order.
 * @param[in]     p_clear_rest When true, cells not covered by the rendered text
 *                             are cleared.
 *
 * @return FM_LCD_MAP_OK on success.
 * @return FM_LCD_MAP_EINVAL when `p_ram` or `p_text` is NULL.
 * @return FM_LCD_MAP_ERANGE when the buffer is too small or `p_row` is invalid.
 * @return FM_LCD_MAP_ENOTSUP when a requested render feature is intentionally
 *         not supported by the current mapping contract.
 */
fm_lcd_map_status_t FM_LCD_MAP_WriteText(uint8_t *p_ram,
                                         uint8_t p_ram_size,
                                         fm_lcd_layout_row_t p_row,
                                         const char *p_text,
                                         fm_lcd_align_t p_align,
                                         bool p_clear_rest);

/**
 * @brief Write text into the visible alphanumeric pair of the provided buffer.
 *
 * The alpha field is modeled as one visible field with two positions:
 * - left alpha digit
 * - right alpha digit
 *
 * Up to two visible characters are rendered. When a single visible character
 * is rendered, `p_align` decides whether it lands on the left or right alpha
 * position. Unsupported characters are rendered as blank.
 *
 * This function operates only on the RAM image in `p_ram`.
 *
 * @param[in,out] p_ram Target RAM buffer.
 * @param[in]     p_ram_size Size in bytes of `p_ram`.
 * @param[in]     p_text Null-terminated input text.
 * @param[in]     p_align Alignment policy within the alpha pair.
 * @param[in]     p_clear_rest When true, alpha positions not covered by the
 *                             rendered text are cleared.
 *
 * @return FM_LCD_MAP_OK on success.
 * @return FM_LCD_MAP_EINVAL when `p_ram` or `p_text` is NULL.
 * @return FM_LCD_MAP_ERANGE when the buffer is too small.
 * @return FM_LCD_MAP_ENOTSUP when the requested alpha render feature is part
 *         of the public contract but intentionally not mapped yet.
 */
fm_lcd_map_status_t FM_LCD_MAP_WriteAlpha(uint8_t *p_ram,
                                          uint8_t p_ram_size,
                                          const char *p_text,
                                          fm_lcd_align_t p_align,
                                          bool p_clear_rest);

/**
 * @brief Set or clear one standalone indicator in the provided buffer.
 *
 * @param[in,out] p_ram Target RAM buffer.
 * @param[in]     p_ram_size Size in bytes of `p_ram`.
 * @param[in]     p_indicator Target indicator.
 * @param[in]     p_on Indicator state to apply.
 *
 * @return FM_LCD_MAP_OK on success.
 * @return FM_LCD_MAP_EINVAL when `p_ram` is NULL.
 * @return FM_LCD_MAP_ERANGE when the buffer is too small or the indicator is
 *         invalid.
 * @return FM_LCD_MAP_ENOTSUP when the requested indicator is part of the public
 *         contract but intentionally not mapped yet.
 */
fm_lcd_map_status_t FM_LCD_MAP_SetIndicator(uint8_t *p_ram,
                                            uint8_t p_ram_size,
                                            fm_lcd_layout_indicator_t p_indicator,
                                            bool p_on);

#endif /* FM_LCD_MAP_H_ */

/*** end of file ***/
