/**
 * @file    display_format.c
 * @brief   Reusable bounded display-field formatting implementation.
 */
#include "display_format.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Private macros */
#define DISPLAY_FORMAT_DIGITS_BUFFER_SIZE    32U

/* Private function declarations */
static fm_status_t display_format_validate_field_(
    const display_format_field_t *p_field,
    char *p_text,
    size_t p_text_size);
static fm_status_t display_format_pow10_(uint8_t p_digits, int64_t *p_value);
static fm_status_t display_format_scale_value_(int64_t p_scaled_value,
                                               uint8_t p_scale_digits,
                                               uint8_t p_fractional_digits,
                                               int64_t *p_value);
static fm_status_t display_format_emit_scaled_(int64_t p_value,
                                               bool p_negative,
                                               const display_format_field_t *p_field,
                                               char *p_text,
                                               size_t p_text_size);
static fm_status_t display_format_emit_overflow_(
    const display_format_field_t *p_field,
    char *p_text,
    size_t p_text_size);
static uint8_t display_format_u64_to_reversed_digits_(uint64_t p_value,
                                                      char *p_digits);
static void display_format_reverse_copy_(const char *p_reversed,
                                         uint8_t p_count,
                                         char *p_text,
                                         size_t *p_index);

/* Public function definitions */
fm_status_t DISPLAY_FORMAT_Unsigned(uint32_t p_value,
                                    const display_format_field_t *p_field,
                                    char *p_text,
                                    size_t p_text_size)
{
    if ((p_field != NULL) && (p_field->fractional_digits != 0U))
    {
        return FM_STATUS_EINVAL;
    }

    return DISPLAY_FORMAT_Scaled((int64_t) p_value,
                                 0U,
                                 p_field,
                                 p_text,
                                 p_text_size);
}

fm_status_t DISPLAY_FORMAT_Signed(int32_t p_value,
                                  const display_format_field_t *p_field,
                                  char *p_text,
                                  size_t p_text_size)
{
    if ((p_field != NULL) && (p_field->fractional_digits != 0U))
    {
        return FM_STATUS_EINVAL;
    }

    return DISPLAY_FORMAT_Scaled((int64_t) p_value,
                                 0U,
                                 p_field,
                                 p_text,
                                 p_text_size);
}

fm_status_t DISPLAY_FORMAT_Scaled(int64_t p_scaled_value,
                                  uint8_t p_scale_digits,
                                  const display_format_field_t *p_field,
                                  char *p_text,
                                  size_t p_text_size)
{
    int64_t scaled_value;
    bool negative;

    if (display_format_validate_field_(p_field, p_text, p_text_size) !=
        FM_STATUS_OK)
    {
        return FM_STATUS_EINVAL;
    }

    if (display_format_scale_value_(p_scaled_value,
                                    p_scale_digits,
                                    p_field->fractional_digits,
                                    &scaled_value) != FM_STATUS_OK)
    {
        return FM_STATUS_ERANGE;
    }

    negative = scaled_value < 0;

    return display_format_emit_scaled_(scaled_value,
                                       negative,
                                       p_field,
                                       p_text,
                                       p_text_size);
}

fm_status_t DISPLAY_FORMAT_Double(double p_value,
                                  const display_format_field_t *p_field,
                                  char *p_text,
                                  size_t p_text_size)
{
    double multiplier = 1.0;
    double scaled_double;
    int64_t scaled_value;
    uint8_t index;

    if (display_format_validate_field_(p_field, p_text, p_text_size) !=
        FM_STATUS_OK)
    {
        return FM_STATUS_EINVAL;
    }

    if (p_value != p_value)
    {
        return FM_STATUS_ERANGE;
    }

    for (index = 0U; index < p_field->fractional_digits; index++)
    {
        multiplier *= 10.0;
    }

    scaled_double = p_value * multiplier;
    if (scaled_double >= 0.0)
    {
        scaled_double += 0.5;
    }
    else
    {
        scaled_double -= 0.5;
    }

    if ((scaled_double > (double) INT64_MAX) ||
        (scaled_double < (double) INT64_MIN))
    {
        return FM_STATUS_ERANGE;
    }

    scaled_value = (int64_t) scaled_double;

    return display_format_emit_scaled_(scaled_value,
                                       scaled_value < 0,
                                       p_field,
                                       p_text,
                                       p_text_size);
}

/* Private function definitions */
static fm_status_t display_format_validate_field_(
    const display_format_field_t *p_field,
    char *p_text,
    size_t p_text_size)
{
    if ((p_field == NULL) || (p_text == NULL) || (p_text_size == 0U))
    {
        return FM_STATUS_EINVAL;
    }

    if ((p_field->visible_width == 0U) ||
        (p_field->visible_width > DISPLAY_FORMAT_VISIBLE_WIDTH_MAX) ||
        (p_field->fractional_digits > DISPLAY_FORMAT_FRACTIONAL_DIGITS_MAX) ||
        ((p_field->align != DISPLAY_FORMAT_ALIGN_RIGHT) &&
         (p_field->align != DISPLAY_FORMAT_ALIGN_LEFT)) ||
        ((p_field->overflow_policy != DISPLAY_FORMAT_OVERFLOW_ERROR) &&
         (p_field->overflow_policy != DISPLAY_FORMAT_OVERFLOW_FILL)))
    {
        return FM_STATUS_EINVAL;
    }

    return FM_STATUS_OK;
}

static fm_status_t display_format_pow10_(uint8_t p_digits, int64_t *p_value)
{
    int64_t value = 1;
    uint8_t index;

    if (p_value == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    for (index = 0U; index < p_digits; index++)
    {
        if (value > (INT64_MAX / 10))
        {
            return FM_STATUS_ERANGE;
        }

        value *= 10;
    }

    *p_value = value;

    return FM_STATUS_OK;
}

static fm_status_t display_format_scale_value_(int64_t p_scaled_value,
                                               uint8_t p_scale_digits,
                                               uint8_t p_fractional_digits,
                                               int64_t *p_value)
{
    uint8_t digit_delta;
    int64_t factor;

    if (p_value == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    if (p_scale_digits > p_fractional_digits)
    {
        digit_delta = (uint8_t) (p_scale_digits - p_fractional_digits);

        if (display_format_pow10_(digit_delta, &factor) != FM_STATUS_OK)
        {
            return FM_STATUS_ERANGE;
        }

        if (p_scaled_value >= 0)
        {
            if (p_scaled_value > (INT64_MAX - (factor / 2)))
            {
                return FM_STATUS_ERANGE;
            }

            *p_value = (p_scaled_value + (factor / 2)) / factor;
        }
        else
        {
            if (p_scaled_value < (INT64_MIN + (factor / 2)))
            {
                return FM_STATUS_ERANGE;
            }

            *p_value = (p_scaled_value - (factor / 2)) / factor;
        }
    }
    else
    {
        digit_delta = (uint8_t) (p_fractional_digits - p_scale_digits);

        if (display_format_pow10_(digit_delta, &factor) != FM_STATUS_OK)
        {
            return FM_STATUS_ERANGE;
        }

        if ((p_scaled_value > (INT64_MAX / factor)) ||
            (p_scaled_value < (INT64_MIN / factor)))
        {
            return FM_STATUS_ERANGE;
        }

        *p_value = p_scaled_value * factor;
    }

    return FM_STATUS_OK;
}

static fm_status_t display_format_emit_scaled_(int64_t p_value,
                                               bool p_negative,
                                               const display_format_field_t *p_field,
                                               char *p_text,
                                               size_t p_text_size)
{
    char reversed_digits[DISPLAY_FORMAT_DIGITS_BUFFER_SIZE];
    uint64_t magnitude;
    uint64_t integer_part;
    uint64_t fractional_part;
    int64_t fractional_factor;
    uint8_t integer_digits;
    uint8_t visible_count;
    uint8_t pad_count;
    size_t text_index = 0U;
    uint8_t index;

    if (display_format_pow10_(p_field->fractional_digits,
                              &fractional_factor) != FM_STATUS_OK)
    {
        return FM_STATUS_ERANGE;
    }

    if (p_negative)
    {
        magnitude = (uint64_t) (-(p_value + 1)) + 1U;
    }
    else
    {
        magnitude = (uint64_t) p_value;
    }

    integer_part = magnitude / (uint64_t) fractional_factor;
    fractional_part = magnitude % (uint64_t) fractional_factor;
    integer_digits = display_format_u64_to_reversed_digits_(integer_part,
                                                            reversed_digits);

    visible_count = integer_digits + p_field->fractional_digits;
    if (p_negative)
    {
        visible_count++;
    }

    if (visible_count > p_field->visible_width)
    {
        return display_format_emit_overflow_(p_field, p_text, p_text_size);
    }

    pad_count = (uint8_t) (p_field->visible_width - visible_count);

    if (p_text_size < ((size_t) p_field->visible_width +
                       ((p_field->fractional_digits > 0U) ? 2U : 1U)))
    {
        return FM_STATUS_EINVAL;
    }

    if ((p_field->align == DISPLAY_FORMAT_ALIGN_RIGHT) &&
        !((p_field->pad_char == '0') && p_negative))
    {
        for (index = 0U; index < pad_count; index++)
        {
            p_text[text_index] = p_field->pad_char;
            text_index++;
        }
    }

    if (p_negative)
    {
        p_text[text_index] = '-';
        text_index++;
    }

    if ((p_field->align == DISPLAY_FORMAT_ALIGN_RIGHT) &&
        (p_field->pad_char == '0') && p_negative)
    {
        for (index = 0U; index < pad_count; index++)
        {
            p_text[text_index] = p_field->pad_char;
            text_index++;
        }
    }

    display_format_reverse_copy_(reversed_digits,
                                 integer_digits,
                                 p_text,
                                 &text_index);

    if (p_field->fractional_digits > 0U)
    {
        p_text[text_index] = '.';
        text_index++;

        for (index = p_field->fractional_digits; index > 0U; index--)
        {
            p_text[text_index] =
                (char) ('0' +
                        ((fractional_part /
                          (uint64_t) (fractional_factor / 10)) % 10U));
            text_index++;
            fractional_factor /= 10;
        }
    }

    if (p_field->align == DISPLAY_FORMAT_ALIGN_LEFT)
    {
        for (index = 0U; index < pad_count; index++)
        {
            p_text[text_index] = p_field->pad_char;
            text_index++;
        }
    }

    p_text[text_index] = '\0';

    return FM_STATUS_OK;
}

static fm_status_t display_format_emit_overflow_(
    const display_format_field_t *p_field,
    char *p_text,
    size_t p_text_size)
{
    uint8_t index;

    if (p_field->overflow_policy != DISPLAY_FORMAT_OVERFLOW_FILL)
    {
        return FM_STATUS_ERANGE;
    }

    if (p_text_size < ((size_t) p_field->visible_width + 1U))
    {
        return FM_STATUS_EINVAL;
    }

    for (index = 0U; index < p_field->visible_width; index++)
    {
        p_text[index] = p_field->overflow_char;
    }

    p_text[p_field->visible_width] = '\0';

    return FM_STATUS_ERANGE;
}

static uint8_t display_format_u64_to_reversed_digits_(uint64_t p_value,
                                                      char *p_digits)
{
    uint8_t count = 0U;

    do
    {
        p_digits[count] = (char) ('0' + (p_value % 10U));
        p_value /= 10U;
        count++;
    } while ((p_value > 0U) && (count < DISPLAY_FORMAT_DIGITS_BUFFER_SIZE));

    return count;
}

static void display_format_reverse_copy_(const char *p_reversed,
                                         uint8_t p_count,
                                         char *p_text,
                                         size_t *p_index)
{
    uint8_t index;

    for (index = p_count; index > 0U; index--)
    {
        p_text[*p_index] = p_reversed[index - 1U];
        (*p_index)++;
    }
}
