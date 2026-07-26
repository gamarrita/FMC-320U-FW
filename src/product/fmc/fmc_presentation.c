/**
 * @file    fmc_presentation.c
 * @brief   FMC startup and live TTL/RATE presentation implementation.
 */

#include "fmc_presentation.h"

#include <float.h>
#include <stddef.h>
#include <stdint.h>

#include "display_format.h"
#include "fmc_version.h"

#define FMC_PRESENTATION_TOP_COLUMNS       8U
#define FMC_PRESENTATION_BOTTOM_COLUMNS    7U

static bool fmc_presentation_snapshot_is_valid_(
    const fmc_presentation_snapshot_t *p_snapshot);
static bool fmc_presentation_rate_state_is_valid_(
    const fmc_runtime_rate_state_t *p_rate);
static fm_status_t fmc_presentation_present_state_(
    fmc_presentation_t *p_presentation,
    fmc_presentation_state_t p_state,
    const fmc_presentation_snapshot_t *p_snapshot);
static fm_status_t fmc_presentation_build_frame_(
    fmc_presentation_state_t p_state,
    const fmc_presentation_snapshot_t *p_snapshot,
    fmc_presentation_frame_t *p_frame);
static void fmc_presentation_clear_frame_(
    fmc_presentation_frame_t *p_frame);
static void fmc_presentation_build_version_(
    fmc_presentation_frame_t *p_frame);
static fm_status_t fmc_presentation_build_ttl_rate_(
    const fmc_presentation_snapshot_t *p_snapshot,
    fmc_presentation_frame_t *p_frame);
static fm_status_t fmc_presentation_format_nonnegative_(
    double p_value,
    uint8_t p_width,
    char *p_text,
    size_t p_text_size,
    bool *p_overflow);
static fm_status_t fmc_presentation_format_least_significant_(
    double p_value,
    uint8_t p_width,
    char *p_text,
    size_t p_text_size);
static uint64_t fmc_presentation_pow10_u64_(uint8_t p_digits);

fm_status_t FMC_PRESENTATION_Init(
    fmc_presentation_t *p_presentation,
    const fmc_presentation_snapshot_t *p_snapshot,
    fmc_presentation_sink_t p_sink,
    void *p_sink_context)
{
    if ((p_presentation == NULL) || (p_sink == NULL) ||
        !fmc_presentation_snapshot_is_valid_(p_snapshot))
    {
        return FM_STATUS_EINVAL;
    }

    p_presentation->state = FMC_PRESENTATION_STATE_NOT_STARTED;
    p_presentation->snapshot = *p_snapshot;
    p_presentation->sink = p_sink;
    p_presentation->sink_context = p_sink_context;

    return FM_STATUS_OK;
}

fm_status_t FMC_PRESENTATION_Start(
    fmc_presentation_t *p_presentation)
{
    if (p_presentation == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    if (p_presentation->state != FMC_PRESENTATION_STATE_NOT_STARTED)
    {
        return FM_STATUS_ESTATE;
    }

    return fmc_presentation_present_state_(
        p_presentation,
        FMC_PRESENTATION_STATE_ALL_SEGMENTS,
        &p_presentation->snapshot);
}

fm_status_t FMC_PRESENTATION_Advance(
    fmc_presentation_t *p_presentation,
    const fmc_presentation_snapshot_t *p_snapshot)
{
    fmc_presentation_state_t next_state;
    fm_status_t status;

    if ((p_presentation == NULL) ||
        !fmc_presentation_snapshot_is_valid_(p_snapshot))
    {
        return FM_STATUS_EINVAL;
    }

    switch (p_presentation->state)
    {
    case FMC_PRESENTATION_STATE_ALL_SEGMENTS:
        next_state = FMC_PRESENTATION_STATE_FIRMWARE_VERSION;
        break;

    case FMC_PRESENTATION_STATE_FIRMWARE_VERSION:
        next_state = FMC_PRESENTATION_STATE_TTL_RATE;
        break;

    case FMC_PRESENTATION_STATE_TTL_RATE:
        return FM_STATUS_OK;

    case FMC_PRESENTATION_STATE_NOT_STARTED:
    default:
        return FM_STATUS_ESTATE;
    }

    status = fmc_presentation_present_state_(p_presentation,
                                             next_state,
                                             p_snapshot);
    if (status == FM_STATUS_OK)
    {
        p_presentation->snapshot = *p_snapshot;
    }

    return status;
}

fm_status_t FMC_PRESENTATION_HandleInput(
    fmc_presentation_t *p_presentation,
    const fmc_input_event_t *p_input,
    const fmc_presentation_snapshot_t *p_snapshot)
{
    if ((p_presentation == NULL) || (p_input == NULL) ||
        !fmc_presentation_snapshot_is_valid_(p_snapshot))
    {
        return FM_STATUS_EINVAL;
    }

    if ((p_input->key == FMC_INPUT_KEY_ESC) &&
        (p_input->action == FMC_INPUT_ACTION_SHORT) &&
        ((p_presentation->state ==
          FMC_PRESENTATION_STATE_ALL_SEGMENTS) ||
         (p_presentation->state ==
          FMC_PRESENTATION_STATE_FIRMWARE_VERSION)))
    {
        return FMC_PRESENTATION_Advance(p_presentation, p_snapshot);
    }

    return FM_STATUS_OK;
}

fm_status_t FMC_PRESENTATION_Refresh(
    fmc_presentation_t *p_presentation,
    const fmc_presentation_snapshot_t *p_snapshot)
{
    fm_status_t status;

    if ((p_presentation == NULL) ||
        !fmc_presentation_snapshot_is_valid_(p_snapshot))
    {
        return FM_STATUS_EINVAL;
    }

    if (p_presentation->state != FMC_PRESENTATION_STATE_TTL_RATE)
    {
        return FM_STATUS_ESTATE;
    }

    status = fmc_presentation_present_state_(
        p_presentation,
        FMC_PRESENTATION_STATE_TTL_RATE,
        p_snapshot);
    if (status == FM_STATUS_OK)
    {
        p_presentation->snapshot = *p_snapshot;
    }

    return status;
}

fmc_presentation_state_t FMC_PRESENTATION_GetState(
    const fmc_presentation_t *p_presentation)
{
    if (p_presentation == NULL)
    {
        return FMC_PRESENTATION_STATE_NOT_STARTED;
    }

    return p_presentation->state;
}

static bool fmc_presentation_snapshot_is_valid_(
    const fmc_presentation_snapshot_t *p_snapshot)
{
    if (p_snapshot == NULL)
    {
        return false;
    }

    return (p_snapshot->ttl >= 0.0) &&
           (p_snapshot->ttl == p_snapshot->ttl) &&
           (p_snapshot->ttl <= DBL_MAX) &&
           fmc_presentation_rate_state_is_valid_(&p_snapshot->rate) &&
           (p_snapshot->volume_unit == FMC_MODEL_VOLUME_UNIT_L) &&
           ((p_snapshot->rate_time_base ==
             FMC_MODEL_TIME_BASE_SECOND) ||
            (p_snapshot->rate_time_base ==
             FMC_MODEL_TIME_BASE_MINUTE)) &&
           (p_snapshot->ttl_fractional_digits ==
            FMC_PRESENTATION_VALUE_FRACTIONAL_DIGITS) &&
           (p_snapshot->rate_fractional_digits ==
            FMC_PRESENTATION_VALUE_FRACTIONAL_DIGITS);
}

static bool fmc_presentation_rate_state_is_valid_(
    const fmc_runtime_rate_state_t *p_rate)
{
    bool value_is_valid;

    if (p_rate == NULL)
    {
        return false;
    }

    value_is_valid = (p_rate->value >= 0.0) &&
                     (p_rate->value == p_rate->value) &&
                     (p_rate->value <= DBL_MAX);

    switch (p_rate->quality)
    {
    case FREQUENCY_OBSERVATION_QUALITY_VALID:
        return p_rate->value_present && value_is_valid;

    case FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE:
        return !p_rate->value_present;

    case FREQUENCY_OBSERVATION_QUALITY_STALE:
    case FREQUENCY_OBSERVATION_QUALITY_INVALID:
        return !p_rate->value_present || value_is_valid;

    default:
        return false;
    }
}

static fm_status_t fmc_presentation_present_state_(
    fmc_presentation_t *p_presentation,
    fmc_presentation_state_t p_state,
    const fmc_presentation_snapshot_t *p_snapshot)
{
    fmc_presentation_frame_t frame;
    fm_status_t status;

    if ((p_presentation == NULL) || (p_presentation->sink == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    status = fmc_presentation_build_frame_(p_state, p_snapshot, &frame);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = p_presentation->sink(&frame,
                                  p_presentation->sink_context);
    if (status == FM_STATUS_OK)
    {
        p_presentation->state = p_state;
    }

    return status;
}

static fm_status_t fmc_presentation_build_frame_(
    fmc_presentation_state_t p_state,
    const fmc_presentation_snapshot_t *p_snapshot,
    fmc_presentation_frame_t *p_frame)
{
    if (p_frame == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    fmc_presentation_clear_frame_(p_frame);

    switch (p_state)
    {
    case FMC_PRESENTATION_STATE_ALL_SEGMENTS:
        p_frame->all_segments = true;
        return FM_STATUS_OK;

    case FMC_PRESENTATION_STATE_FIRMWARE_VERSION:
        fmc_presentation_build_version_(p_frame);
        return FM_STATUS_OK;

    case FMC_PRESENTATION_STATE_TTL_RATE:
        return fmc_presentation_build_ttl_rate_(p_snapshot, p_frame);

    case FMC_PRESENTATION_STATE_NOT_STARTED:
    default:
        return FM_STATUS_EINVAL;
    }
}

static void fmc_presentation_clear_frame_(
    fmc_presentation_frame_t *p_frame)
{
    uint8_t index;

    p_frame->all_segments = false;
    for (index = 0U; index < FMC_PRESENTATION_TOP_TEXT_SIZE; index++)
    {
        p_frame->top_text[index] = '\0';
    }
    for (index = 0U; index < FMC_PRESENTATION_BOTTOM_TEXT_SIZE; index++)
    {
        p_frame->bottom_text[index] = '\0';
    }
    for (index = 0U; index < FMC_PRESENTATION_ALPHA_TEXT_SIZE; index++)
    {
        p_frame->alpha_text[index] = '\0';
    }
    p_frame->indicator_ttl = false;
    p_frame->indicator_rate = false;
    p_frame->indicator_slash = false;
    p_frame->indicator_second = false;
    p_frame->indicator_minute = false;
    p_frame->ttl_overflow = false;
    p_frame->rate_overflow = false;
}

static void fmc_presentation_build_version_(
    fmc_presentation_frame_t *p_frame)
{
    p_frame->bottom_text[0] =
        (char) ('0' + (FMC_VERSION_MAJOR / 10));
    p_frame->bottom_text[1] =
        (char) ('0' + (FMC_VERSION_MAJOR % 10));
    p_frame->bottom_text[2] = '.';
    p_frame->bottom_text[3] =
        (char) ('0' + (FMC_VERSION_MINOR / 10));
    p_frame->bottom_text[4] =
        (char) ('0' + (FMC_VERSION_MINOR % 10));
    p_frame->bottom_text[5] = '.';
    p_frame->bottom_text[6] =
        (char) ('0' + (FMC_VERSION_PATCH / 10));
    p_frame->bottom_text[7] =
        (char) ('0' + (FMC_VERSION_PATCH % 10));
    p_frame->bottom_text[8] = '\0';
    p_frame->alpha_text[0] = 'B';
    p_frame->alpha_text[1] =
        (char) ('0' + FMC_VERSION_BETA);
    p_frame->alpha_text[2] = '\0';
}

static fm_status_t fmc_presentation_build_ttl_rate_(
    const fmc_presentation_snapshot_t *p_snapshot,
    fmc_presentation_frame_t *p_frame)
{
    fm_status_t status;

    if (!fmc_presentation_snapshot_is_valid_(p_snapshot))
    {
        return FM_STATUS_EINVAL;
    }

    status = fmc_presentation_format_nonnegative_(
        p_snapshot->ttl,
        FMC_PRESENTATION_TOP_COLUMNS,
        p_frame->top_text,
        sizeof(p_frame->top_text),
        &p_frame->ttl_overflow);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    if (p_snapshot->rate.quality ==
        FREQUENCY_OBSERVATION_QUALITY_VALID)
    {
        status = fmc_presentation_format_nonnegative_(
            p_snapshot->rate.value,
            FMC_PRESENTATION_BOTTOM_COLUMNS,
            p_frame->bottom_text,
            sizeof(p_frame->bottom_text),
            &p_frame->rate_overflow);
        if (status != FM_STATUS_OK)
        {
            return status;
        }
    }
    else
    {
        uint8_t index;

        for (index = 0U;
             index < FMC_PRESENTATION_BOTTOM_COLUMNS;
             index++)
        {
            p_frame->bottom_text[index] = '-';
        }
        p_frame->bottom_text[FMC_PRESENTATION_BOTTOM_COLUMNS] = '\0';
        p_frame->rate_overflow = false;
    }

    p_frame->alpha_text[0] = FMC_PRESENTATION_LITERS_LEGEND[0];
    p_frame->alpha_text[1] = FMC_PRESENTATION_LITERS_LEGEND[1];
    p_frame->alpha_text[2] = '\0';
    p_frame->indicator_ttl = true;
    p_frame->indicator_rate = true;
    p_frame->indicator_slash = true;
    p_frame->indicator_second =
        (p_snapshot->rate_time_base == FMC_MODEL_TIME_BASE_SECOND);
    p_frame->indicator_minute =
        (p_snapshot->rate_time_base == FMC_MODEL_TIME_BASE_MINUTE);

    return FM_STATUS_OK;
}

static fm_status_t fmc_presentation_format_nonnegative_(
    double p_value,
    uint8_t p_width,
    char *p_text,
    size_t p_text_size,
    bool *p_overflow)
{
    const display_format_field_t field =
    {
        .visible_width = p_width,
        .fractional_digits = FMC_PRESENTATION_VALUE_FRACTIONAL_DIGITS,
        .align = DISPLAY_FORMAT_ALIGN_RIGHT,
        .pad_char = ' ',
        .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
        .overflow_char = ' '
    };
    fm_status_t status;

    if ((p_text == NULL) || (p_overflow == NULL) ||
        (p_value < 0.0) || (p_value != p_value))
    {
        return FM_STATUS_EINVAL;
    }

    status = DISPLAY_FORMAT_Double(p_value,
                                   &field,
                                   p_text,
                                   p_text_size);
    if (status == FM_STATUS_OK)
    {
        *p_overflow = false;
        return FM_STATUS_OK;
    }

    if (status != FM_STATUS_ERANGE)
    {
        return status;
    }

    status = fmc_presentation_format_least_significant_(p_value,
                                                        p_width,
                                                        p_text,
                                                        p_text_size);
    if (status == FM_STATUS_OK)
    {
        *p_overflow = true;
    }

    return status;
}

static fm_status_t fmc_presentation_format_least_significant_(
    double p_value,
    uint8_t p_width,
    char *p_text,
    size_t p_text_size)
{
    char digits[FMC_PRESENTATION_TOP_COLUMNS];
    double scaled_double;
    uint64_t scaled_value;
    uint64_t modulus;
    uint8_t digit_index;
    uint8_t text_index;

    if ((p_text == NULL) || (p_width < 2U) ||
        (p_width > FMC_PRESENTATION_TOP_COLUMNS) ||
        (p_text_size < ((size_t) p_width + 2U)))
    {
        return FM_STATUS_EINVAL;
    }

    scaled_double = (p_value * 10.0) + 0.5;
    if ((scaled_double < 0.0) ||
        (scaled_double > (double) UINT64_MAX))
    {
        return FM_STATUS_ERANGE;
    }

    scaled_value = (uint64_t) scaled_double;
    modulus = fmc_presentation_pow10_u64_(p_width);
    scaled_value %= modulus;

    for (digit_index = p_width; digit_index > 0U; digit_index--)
    {
        digits[digit_index - 1U] =
            (char) ('0' + (scaled_value % 10U));
        scaled_value /= 10U;
    }

    text_index = 0U;
    for (digit_index = 0U; digit_index < p_width; digit_index++)
    {
        if (digit_index == (uint8_t) (p_width - 1U))
        {
            p_text[text_index] = '.';
            text_index++;
        }

        p_text[text_index] = digits[digit_index];
        text_index++;
    }
    p_text[text_index] = '\0';

    return FM_STATUS_OK;
}

static uint64_t fmc_presentation_pow10_u64_(uint8_t p_digits)
{
    uint64_t value = 1U;
    uint8_t index;

    for (index = 0U; index < p_digits; index++)
    {
        value *= 10U;
    }

    return value;
}
