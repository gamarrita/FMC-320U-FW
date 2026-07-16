/**
 * @file    display_format.h
 * @brief   Reusable bounded display-field formatting contract.
 *
 * This module formats numeric values into bounded text fields that can later
 * be written to display drivers such as the custom LCD BSP.
 *
 * It intentionally does not:
 * - write to LCD hardware,
 * - know LCD RAM, segments, or controller details,
 * - choose FMC screen content,
 * - choose product unit labels,
 * - own runtime, RTOS, persistence, or acquisition state.
 *
 * Decimal points are emitted as '.' characters in the output string. This is
 * compatible with the current LCD text contract, where '.' enables the decimal
 * point of the previous visible numeric character.
 */
#ifndef DISPLAY_FORMAT_H
#define DISPLAY_FORMAT_H

#include <stddef.h>
#include <stdint.h>

#include "fm_status.h"

/* =========================== Public Macros ============================= */
#define DISPLAY_FORMAT_VISIBLE_WIDTH_MAX        32U
#define DISPLAY_FORMAT_FRACTIONAL_DIGITS_MAX    9U

/* =========================== Public Types ============================== */
typedef enum
{
    DISPLAY_FORMAT_ALIGN_RIGHT = 0,
    DISPLAY_FORMAT_ALIGN_LEFT
} display_format_align_t;

typedef enum
{
    DISPLAY_FORMAT_OVERFLOW_ERROR = 0,
    DISPLAY_FORMAT_OVERFLOW_FILL
} display_format_overflow_policy_t;

typedef struct
{
    /*
     * Number of visible display character positions.
     *
     * Decimal-point separators do not count as visible positions. For example,
     * formatting 1234.5 with a visible width of 5 may produce "1234.5", which
     * has five visible characters plus one decimal-point separator.
     */
    uint8_t visible_width;

    /*
     * Number of fractional digits to emit after rounding.
     *
     * A value of zero formats without a decimal point.
     */
    uint8_t fractional_digits;

    /*
     * Alignment and fill policy inside the visible field.
     *
     * `pad_char` should normally be ' ' or '0'. Unsupported display characters
     * are the caller's responsibility because this module stays display-driver
     * independent.
     */
    display_format_align_t align;
    char pad_char;

    /*
     * Overflow policy for values that do not fit in `visible_width`.
     *
     * With DISPLAY_FORMAT_OVERFLOW_ERROR, the function returns
     * FM_STATUS_ERANGE and leaves output content unspecified.
     *
     * With DISPLAY_FORMAT_OVERFLOW_FILL, the output is filled with
     * `overflow_char` for the visible width and the function still returns
     * FM_STATUS_ERANGE so callers can react if needed.
     */
    display_format_overflow_policy_t overflow_policy;
    char overflow_char;
} display_format_field_t;

/* =========================== Public API ================================ */
/**
 * @brief Format an unsigned integer into a bounded display field.
 *
 * `fractional_digits` must be zero for this function.
 *
 * @param[in]  p_value Value to format.
 * @param[in]  p_field Field formatting policy.
 * @param[out] p_text Output null-terminated text buffer.
 * @param[in]  p_text_size Size of `p_text`, including the null terminator.
 *
 * @return FM_STATUS_OK on success.
 * @return FM_STATUS_EINVAL on invalid pointers, invalid buffer size, or
 *         invalid field policy.
 * @return FM_STATUS_ERANGE when the value does not fit.
 */
fm_status_t DISPLAY_FORMAT_Unsigned(
    uint32_t p_value,
    const display_format_field_t *p_field,
    char *p_text,
    size_t p_text_size);

/**
 * @brief Format a signed integer into a bounded display field.
 *
 * `fractional_digits` must be zero for this function.
 *
 * @param[in]  p_value Value to format.
 * @param[in]  p_field Field formatting policy.
 * @param[out] p_text Output null-terminated text buffer.
 * @param[in]  p_text_size Size of `p_text`, including the null terminator.
 *
 * @return FM_STATUS_OK on success.
 * @return FM_STATUS_EINVAL on invalid pointers, invalid buffer size, or
 *         invalid field policy.
 * @return FM_STATUS_ERANGE when the value does not fit.
 */
fm_status_t DISPLAY_FORMAT_Signed(
    int32_t p_value,
    const display_format_field_t *p_field,
    char *p_text,
    size_t p_text_size);

/**
 * @brief Format a signed fixed-point value into a bounded display field.
 *
 * `p_scaled_value` is interpreted as `value * 10^p_scale_digits`.
 * The formatted result is rounded to `p_field->fractional_digits`.
 *
 * Example:
 * - `p_scaled_value = 12345`
 * - `p_scale_digits = 2`
 * - `p_field->fractional_digits = 1`
 * - output value is `123.5`
 *
 * @param[in]  p_scaled_value Fixed-point integer value.
 * @param[in]  p_scale_digits Decimal digits used by `p_scaled_value`.
 * @param[in]  p_field Field formatting policy.
 * @param[out] p_text Output null-terminated text buffer.
 * @param[in]  p_text_size Size of `p_text`, including the null terminator.
 *
 * @return FM_STATUS_OK on success.
 * @return FM_STATUS_EINVAL on invalid pointers, invalid buffer size, or
 *         invalid field policy.
 * @return FM_STATUS_ERANGE when scaling, rounding, or the final field cannot
 *         be represented.
 */
fm_status_t DISPLAY_FORMAT_Scaled(
    int64_t p_scaled_value,
    uint8_t p_scale_digits,
    const display_format_field_t *p_field,
    char *p_text,
    size_t p_text_size);

/**
 * @brief Format a floating-point value into a bounded display field.
 *
 * This is a convenience contract for callers that already own floating-point
 * values. Firmware paths that can keep values as fixed point should prefer
 * `DISPLAY_FORMAT_Scaled()` for predictability.
 *
 * The formatted result is rounded to `p_field->fractional_digits`.
 *
 * @param[in]  p_value Value to format.
 * @param[in]  p_field Field formatting policy.
 * @param[out] p_text Output null-terminated text buffer.
 * @param[in]  p_text_size Size of `p_text`, including the null terminator.
 *
 * @return FM_STATUS_OK on success.
 * @return FM_STATUS_EINVAL on invalid pointers, invalid buffer size, or
 *         invalid field policy.
 * @return FM_STATUS_ERANGE when `p_value` is not finite, cannot be represented
 *         by the implementation, or does not fit in the field.
 */
fm_status_t DISPLAY_FORMAT_Double(
    double p_value,
    const display_format_field_t *p_field,
    char *p_text,
    size_t p_text_size);

#endif /* DISPLAY_FORMAT_H */
