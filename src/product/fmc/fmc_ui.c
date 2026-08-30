/**
 * @file    fmc_ui.c
 * @brief   FMC startup and Phase 8 user-menu UI implementation.
 */

#include "fmc_ui.h"

#include <float.h>
#include <stddef.h>
#include <stdint.h>

#include "display_format.h"
#include "fmc_version.h"

#define FMC_UI_TOP_COLUMNS 8U
#define FMC_UI_BOTTOM_COLUMNS 7U

/**
 * @brief Determine whether one RATE state can be rendered.
 *
 * A valid-quality observation requires a present, finite, nonnegative value.
 * Every accepted non-valid quality renders as dashes, so its retained value
 * and presence metadata are not inspected by this presentation boundary.
 *
 * @param[in] p_rate RATE state to validate.
 *
 * @return `true` when the RATE state has an accepted visible representation.
 * @return `false` for `NULL`, an unknown quality, or an unrenderable valid RATE.
 */
static bool fmc_ui_rate_state_is_valid_(const fmc_runtime_rate_state_t* p_rate);

/**
 * @brief Validate one semantic input event accepted by the UI contract.
 *
 * Checks key and action enum bounds and enforces that external buttons only
 * produce SHORT actions.
 *
 * @param[in] p_input Input event to validate.
 *
 * @return `true` when the event is accepted by the UI.
 * @return `false` otherwise, including when `p_input` is `NULL`.
 */
static bool fmc_ui_input_is_valid_(const fmc_input_event_t* p_input);

/**
 * @brief Determine whether a state belongs to the five-screen user menu.
 *
 * Startup and sentinel states are not user-menu states.
 *
 * @param[in] p_state State to classify.
 *
 * @return `true` for TTL/RATE through DATE_TIME; otherwise `false`.
 */
static bool fmc_ui_state_is_user_(fmc_ui_state_t p_state);

/**
 * @brief Build and present one state transactionally through the configured sink.
 *
 * The requested state becomes current only after frame composition and the
 * sink call both succeed. Snapshot ownership remains with the caller; this
 * helper does not update `p_ui->snapshot`.
 *
 * @param[in,out] p_ui UI instance whose sink is used and state may be committed.
 * @param[in] p_state State whose semantic frame must be presented.
 * @param[in] p_snapshot Snapshot used to compose live user-menu frames.
 *
 * @return `FM_STATUS_OK` when the sink accepts the frame and state is committed.
 * @return `FM_STATUS_EINVAL` for an invalid UI, sink, state, or required snapshot.
 * @return Other formatting or sink errors unchanged, without committing the state.
 */
static fm_status_t fmc_ui_present_state_(fmc_ui_t* p_ui, fmc_ui_state_t p_state,
                                         const fmc_ui_snapshot_t* p_snapshot);

/**
 * @brief Compose the complete semantic frame for one UI state.
 *
 * Starts from a cleared frame, then selects the all-segments, version,
 * measurement, or placeholder representation associated with `p_state`.
 * User-menu states require a valid snapshot and carry the supplied POINT state.
 *
 * @param[in] p_state State to represent.
 * @param[in] p_snapshot Snapshot used by user-menu states; ignored by startup views.
 * @param[in] p_point_on Logical POINT state to place in user-menu frames.
 * @param[out] p_frame Destination semantic frame.
 *
 * @return `FM_STATUS_OK` when the frame is composed.
 * @return `FM_STATUS_EINVAL` for a null destination, unsupported state, or
 *         invalid snapshot required by a user-menu state.
 * @return Other formatting errors unchanged.
 */
static fm_status_t fmc_ui_build_frame_(fmc_ui_state_t p_state, const fmc_ui_snapshot_t* p_snapshot,
                                       bool p_point_on, fmc_ui_frame_t* p_frame);

/**
 * @brief Reset a semantic frame to blank text and all indicators off.
 *
 * This establishes the common baseline used before composing every state.
 *
 * @param[out] p_frame Frame to clear; must not be `NULL`.
 */
static void fmc_ui_clear_frame_(fmc_ui_frame_t* p_frame);

/**
 * @brief Compose the firmware-version startup frame.
 *
 * Writes the numeric `major.minor.patch` value to the bottom row and the beta
 * marker to the alpha field. The caller must provide an already cleared frame.
 *
 * @param[in,out] p_frame Frame to populate; must not be `NULL`.
 */
static void fmc_ui_build_version_(fmc_ui_frame_t* p_frame);

/**
 * @brief Compose a live volume-and-rate measurement frame.
 *
 * Formats the selected volume on the top row and RATE on the bottom row. A
 * non-valid RATE quality is rendered as dashes. The helper also selects the
 * TTL or ACM legend and the configured rate time-base indicators.
 *
 * @param[in] p_volume Nonnegative volume value to display.
 * @param[in] p_show_ttl `true` for the TTL legend; `false` for the ACM legend.
 * @param[in] p_snapshot Snapshot supplying RATE and presentation settings.
 * @param[in,out] p_frame Cleared frame to populate.
 *
 * @return `FM_STATUS_OK` when the measurement frame is composed.
 * @return `FM_STATUS_EINVAL` when the snapshot is invalid.
 * @return Other numeric-formatting errors unchanged.
 */
static fm_status_t fmc_ui_build_measurement_(double p_volume, bool p_show_ttl,
                                             const fmc_ui_snapshot_t* p_snapshot,
                                             fmc_ui_frame_t* p_frame);

/**
 * @brief Write the two-character legend for one configured volume unit.
 *
 * Every current model unit has an explicit visible representation. No unit is
 * silently normalized to another unit.
 *
 * @param[in] p_unit Configured volume unit.
 * @param[out] p_alpha_text Destination alpha field.
 *
 * @return `FM_STATUS_OK` when the legend is written.
 * @return `FM_STATUS_EINVAL` for an invalid destination or unknown enum value.
 */
static fm_status_t fmc_ui_set_volume_legend_(fmc_model_volume_unit_t p_unit,
                                             char* p_alpha_text);

/**
 * @brief Select the standalone indicator for one configured RATE time base.
 *
 * @param[in] p_time_base Configured RATE time base.
 * @param[in,out] p_frame Frame whose S, M, H, or D indicator is selected.
 *
 * @return `FM_STATUS_OK` when exactly one time-base indicator is selected.
 * @return `FM_STATUS_EINVAL` for an invalid frame or unknown enum value.
 */
static fm_status_t fmc_ui_set_time_base_indicator_(fmc_model_time_base_t p_time_base,
                                                   fmc_ui_frame_t* p_frame);

/**
 * @brief Compose an inert user-menu placeholder frame.
 *
 * Writes the two-character screen mnemonic in the alpha field and `OFF` in
 * the bottom row. The caller must provide an already cleared frame and a
 * mnemonic containing at least two characters.
 *
 * @param[in] p_alpha Two-character screen mnemonic.
 * @param[in,out] p_frame Cleared frame to populate; must not be `NULL`.
 */
static void fmc_ui_build_placeholder_(const char* p_alpha, fmc_ui_frame_t* p_frame);

/**
 * @brief Format a nonnegative value using the Phase 8 fixed-point policy.
 *
 * First attempts the normal right-aligned formatter. If the value exceeds the
 * visible field, it falls back to the least-significant digits and reports the
 * intentional overflow representation through `p_overflow`.
 *
 * @param[in] p_value Nonnegative finite value to format.
 * @param[in] p_width Number of visible numeric digit columns, excluding the point.
 * @param[out] p_text Destination string.
 * @param[in] p_text_size Capacity of `p_text`, including the terminator.
 * @param[out] p_overflow Set when least-significant overflow rendering is used.
 *
 * @return `FM_STATUS_OK` when either normal or overflow text is produced.
 * @return `FM_STATUS_EINVAL` for invalid pointers or numeric input.
 * @return Other formatter or range errors unchanged.
 */
static fm_status_t fmc_ui_format_nonnegative_(double p_value, uint8_t p_width, char* p_text,
                                              size_t p_text_size, bool* p_overflow);

/**
 * @brief Render the least-significant fixed-point digits of an overflowing value.
 *
 * Rounds to one fractional digit, keeps the lowest `p_width` decimal digits,
 * and inserts the decimal point before the last digit. This makes overflow
 * visible through the separate semantic flag while preserving recent digits.
 *
 * @param[in] p_value Nonnegative value to reduce.
 * @param[in] p_width Number of decimal digits to retain, excluding the point.
 * @param[out] p_text Destination string.
 * @param[in] p_text_size Capacity of `p_text`, including point and terminator.
 *
 * @return `FM_STATUS_OK` when the reduced string is produced.
 * @return `FM_STATUS_EINVAL` for invalid pointers, width, or destination size.
 * @return `FM_STATUS_ERANGE` when the scaled value cannot fit in `uint64_t`.
 */
static fm_status_t fmc_ui_format_least_significant_(double p_value, uint8_t p_width, char* p_text,
                                                    size_t p_text_size);

/**
 * @brief Calculate an unsigned integer power of ten for decimal reduction.
 *
 * The caller limits `p_digits` to the UI field width, so the result fits in
 * `uint64_t`.
 *
 * @param[in] p_digits Decimal exponent.
 *
 * @return `10` raised to `p_digits`.
 */
static uint64_t fmc_ui_pow10_u64_(uint8_t p_digits);

fm_status_t FMC_UI_Init(fmc_ui_t* p_ui, const fmc_ui_snapshot_t* p_snapshot, fmc_ui_sink_t p_sink,
                        void* p_sink_context)
{
    if ((p_ui == NULL) || (p_snapshot == NULL) || (p_sink == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    p_ui->state = FMC_UI_STATE_NOT_STARTED;
    p_ui->snapshot = *p_snapshot;
    p_ui->point_on = false;
    p_ui->sink = p_sink;
    p_ui->sink_context = p_sink_context;

    return FM_STATUS_OK;
}

fm_status_t FMC_UI_Start(fmc_ui_t* p_ui)
{
    if (p_ui == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    if (p_ui->state != FMC_UI_STATE_NOT_STARTED)
    {
        return FM_STATUS_ESTATE;
    }

    return fmc_ui_present_state_(p_ui, FMC_UI_STATE_ALL_SEGMENTS, &p_ui->snapshot);
}

fm_status_t FMC_UI_Advance(fmc_ui_t* p_ui, const fmc_ui_snapshot_t* p_snapshot)
{
    fmc_ui_state_t next_state;
    fm_status_t status;

    if ((p_ui == NULL) || (p_snapshot == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    switch (p_ui->state)
    {
    case FMC_UI_STATE_ALL_SEGMENTS:
        next_state = FMC_UI_STATE_FIRMWARE_VERSION;
        break;

    case FMC_UI_STATE_FIRMWARE_VERSION:
        next_state = FMC_UI_STATE_TTL_RATE;
        break;

    case FMC_UI_STATE_TTL_RATE:
    case FMC_UI_STATE_ACM_RATE:
    case FMC_UI_STATE_PRINT:
    case FMC_UI_STATE_LOG_DOWNLOAD:
    case FMC_UI_STATE_DATE_TIME:
        return FM_STATUS_OK;

    case FMC_UI_STATE_NOT_STARTED:
    case FMC_UI_STATE_COUNT:
    default:
        return FM_STATUS_ESTATE;
    }

    status = fmc_ui_present_state_(p_ui, next_state, p_snapshot);
    if (status == FM_STATUS_OK)
    {
        p_ui->snapshot = *p_snapshot;
    }

    return status;
}

fm_status_t FMC_UI_HandleInput(fmc_ui_t* p_ui, const fmc_input_event_t* p_input,
                               const fmc_ui_snapshot_t* p_snapshot, fmc_ui_request_t* p_request)
{
    fmc_ui_state_t next_state;
    fm_status_t status;

    if (p_request == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    p_request->kind = FMC_UI_REQUEST_NONE;

    if ((p_ui == NULL) || !fmc_ui_input_is_valid_(p_input) || (p_snapshot == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    if ((p_ui->state == FMC_UI_STATE_ALL_SEGMENTS) ||
        (p_ui->state == FMC_UI_STATE_FIRMWARE_VERSION))
    {
        if ((p_input->key == FMC_INPUT_KEY_ESC) && (p_input->action == FMC_INPUT_ACTION_SHORT))
        {
            return FMC_UI_Advance(p_ui, p_snapshot);
        }

        return FM_STATUS_OK;
    }

    if (!fmc_ui_state_is_user_(p_ui->state))
    {
        return FM_STATUS_ESTATE;
    }

    if ((p_ui->state == FMC_UI_STATE_ACM_RATE) &&
        (((p_input->key == FMC_INPUT_KEY_ENTER) && (p_input->action == FMC_INPUT_ACTION_LONG)) ||
         ((p_input->key == FMC_INPUT_KEY_EXT_2) && (p_input->action == FMC_INPUT_ACTION_SHORT))))
    {
        p_request->kind = FMC_UI_REQUEST_RESET_ACM;
        return FM_STATUS_OK;
    }

    next_state = p_ui->state;
    if (p_input->action == FMC_INPUT_ACTION_SHORT)
    {
        switch (p_input->key)
        {
        case FMC_INPUT_KEY_DOWN:
            if (next_state < FMC_UI_STATE_DATE_TIME)
            {
                next_state = (fmc_ui_state_t)(next_state + 1U);
            }
            break;

        case FMC_INPUT_KEY_UP:
            if (next_state > FMC_UI_STATE_TTL_RATE)
            {
                next_state = (fmc_ui_state_t)(next_state - 1U);
            }
            break;

        case FMC_INPUT_KEY_EXT_1:
            if (next_state == FMC_UI_STATE_DATE_TIME)
            {
                next_state = FMC_UI_STATE_TTL_RATE;
            }
            else
            {
                next_state = (fmc_ui_state_t)(next_state + 1U);
            }
            break;

        case FMC_INPUT_KEY_ENTER:
        case FMC_INPUT_KEY_ESC:
        case FMC_INPUT_KEY_EXT_2:
        case FMC_INPUT_KEY_COUNT:
        default:
            break;
        }
    }

    if (next_state == p_ui->state)
    {
        return FM_STATUS_OK;
    }

    status = fmc_ui_present_state_(p_ui, next_state, p_snapshot);
    if (status == FM_STATUS_OK)
    {
        p_ui->snapshot = *p_snapshot;
    }

    return status;
}

fm_status_t FMC_UI_Refresh(fmc_ui_t* p_ui, const fmc_ui_snapshot_t* p_snapshot)
{
    fm_status_t status;

    if ((p_ui == NULL) || (p_snapshot == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    if (!fmc_ui_state_is_user_(p_ui->state))
    {
        return FM_STATUS_ESTATE;
    }

    status = fmc_ui_present_state_(p_ui, p_ui->state, p_snapshot);
    if (status == FM_STATUS_OK)
    {
        p_ui->snapshot = *p_snapshot;
    }

    return status;
}

fm_status_t FMC_UI_ObservePulseDelta(fmc_ui_t* p_ui, uint64_t p_pulse_delta)
{
    if (p_ui == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    if (fmc_ui_state_is_user_(p_ui->state))
    {
        if (p_pulse_delta == 0U)
        {
            p_ui->point_on = false;
        }
        else
        {
            p_ui->point_on = !p_ui->point_on;
        }

        return FM_STATUS_OK;
    }

    switch (p_ui->state)
    {
    case FMC_UI_STATE_NOT_STARTED:
    case FMC_UI_STATE_ALL_SEGMENTS:
    case FMC_UI_STATE_FIRMWARE_VERSION:
        return FM_STATUS_OK;

    case FMC_UI_STATE_COUNT:
    default:
        return FM_STATUS_ESTATE;
    }
}

fmc_ui_state_t FMC_UI_GetState(const fmc_ui_t* p_ui)
{
    if (p_ui == NULL)
    {
        return FMC_UI_STATE_NOT_STARTED;
    }

    return p_ui->state;
}

static bool fmc_ui_input_is_valid_(const fmc_input_event_t* p_input)
{
    if ((p_input == NULL) || (p_input->key < FMC_INPUT_KEY_DOWN) ||
        (p_input->key >= FMC_INPUT_KEY_COUNT) || (p_input->action < FMC_INPUT_ACTION_SHORT) ||
        (p_input->action >= FMC_INPUT_ACTION_COUNT))
    {
        return false;
    }

    if (((p_input->key == FMC_INPUT_KEY_EXT_1) || (p_input->key == FMC_INPUT_KEY_EXT_2)) &&
        (p_input->action != FMC_INPUT_ACTION_SHORT))
    {
        return false;
    }

    return true;
}

static bool fmc_ui_state_is_user_(fmc_ui_state_t p_state)
{
    return (p_state >= FMC_UI_STATE_TTL_RATE) && (p_state <= FMC_UI_STATE_DATE_TIME);
}

static bool fmc_ui_rate_state_is_valid_(const fmc_runtime_rate_state_t* p_rate)
{
    bool value_is_valid;

    if (p_rate == NULL)
    {
        return false;
    }

    value_is_valid =
        (p_rate->value >= 0.0) && (p_rate->value == p_rate->value) && (p_rate->value <= DBL_MAX);

    switch (p_rate->quality)
    {
    case FREQUENCY_OBSERVATION_QUALITY_VALID:
        return p_rate->value_present && value_is_valid;

    case FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE:
    case FREQUENCY_OBSERVATION_QUALITY_STALE:
    case FREQUENCY_OBSERVATION_QUALITY_INVALID:
        return true;

    default:
        return false;
    }
}

static fm_status_t fmc_ui_present_state_(fmc_ui_t* p_ui, fmc_ui_state_t p_state,
                                         const fmc_ui_snapshot_t* p_snapshot)
{
    fmc_ui_frame_t frame;
    fm_status_t status;

    if ((p_ui == NULL) || (p_ui->sink == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    status = fmc_ui_build_frame_(p_state, p_snapshot, p_ui->point_on, &frame);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = p_ui->sink(&frame, p_ui->sink_context);
    if (status == FM_STATUS_OK)
    {
        p_ui->state = p_state;
    }

    return status;
}

static fm_status_t fmc_ui_build_frame_(fmc_ui_state_t p_state, const fmc_ui_snapshot_t* p_snapshot,
                                       bool p_point_on, fmc_ui_frame_t* p_frame)
{
    if (p_frame == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    if (fmc_ui_state_is_user_(p_state) && (p_snapshot == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    fmc_ui_clear_frame_(p_frame);

    switch (p_state)
    {
    case FMC_UI_STATE_ALL_SEGMENTS:
        p_frame->all_segments = true;
        return FM_STATUS_OK;

    case FMC_UI_STATE_FIRMWARE_VERSION:
        fmc_ui_build_version_(p_frame);
        return FM_STATUS_OK;

    case FMC_UI_STATE_TTL_RATE:
        p_frame->indicator_point = p_point_on;
        return fmc_ui_build_measurement_(p_snapshot->ttl, true, p_snapshot, p_frame);

    case FMC_UI_STATE_ACM_RATE:
        p_frame->indicator_point = p_point_on;
        return fmc_ui_build_measurement_(p_snapshot->acm, false, p_snapshot, p_frame);

    case FMC_UI_STATE_PRINT:
        p_frame->indicator_point = p_point_on;
        fmc_ui_build_placeholder_("PR", p_frame);
        return FM_STATUS_OK;

    case FMC_UI_STATE_LOG_DOWNLOAD:
        p_frame->indicator_point = p_point_on;
        fmc_ui_build_placeholder_("LD", p_frame);
        return FM_STATUS_OK;

    case FMC_UI_STATE_DATE_TIME:
        p_frame->indicator_point = p_point_on;
        fmc_ui_build_placeholder_("DT", p_frame);
        return FM_STATUS_OK;

    case FMC_UI_STATE_NOT_STARTED:
    case FMC_UI_STATE_COUNT:
    default:
        return FM_STATUS_EINVAL;
    }
}

static void fmc_ui_clear_frame_(fmc_ui_frame_t* p_frame)
{
    uint8_t index;

    p_frame->all_segments = false;
    for (index = 0U; index < FMC_UI_TOP_TEXT_SIZE; index++)
    {
        p_frame->top_text[index] = '\0';
    }
    for (index = 0U; index < FMC_UI_BOTTOM_TEXT_SIZE; index++)
    {
        p_frame->bottom_text[index] = '\0';
    }
    for (index = 0U; index < FMC_UI_ALPHA_TEXT_SIZE; index++)
    {
        p_frame->alpha_text[index] = '\0';
    }
    p_frame->indicator_point = false;
    p_frame->indicator_acm_top = false;
    p_frame->indicator_acm_bottom = false;
    p_frame->indicator_ttl = false;
    p_frame->indicator_rate = false;
    p_frame->indicator_slash = false;
    p_frame->indicator_second = false;
    p_frame->indicator_minute = false;
    p_frame->indicator_hour = false;
    p_frame->indicator_day = false;
    p_frame->volume_overflow = false;
    p_frame->rate_overflow = false;
}

static void fmc_ui_build_version_(fmc_ui_frame_t* p_frame)
{
    p_frame->bottom_text[0] = (char)('0' + (FMC_VERSION_MAJOR / 10));
    p_frame->bottom_text[1] = (char)('0' + (FMC_VERSION_MAJOR % 10));
    p_frame->bottom_text[2] = '.';
    p_frame->bottom_text[3] = (char)('0' + (FMC_VERSION_MINOR / 10));
    p_frame->bottom_text[4] = (char)('0' + (FMC_VERSION_MINOR % 10));
    p_frame->bottom_text[5] = '.';
    p_frame->bottom_text[6] = (char)('0' + (FMC_VERSION_PATCH / 10));
    p_frame->bottom_text[7] = (char)('0' + (FMC_VERSION_PATCH % 10));
    p_frame->bottom_text[8] = '\0';
    p_frame->alpha_text[0] = 'B';
    p_frame->alpha_text[1] = (char)('0' + FMC_VERSION_BETA);
    p_frame->alpha_text[2] = '\0';
}

static fm_status_t fmc_ui_build_measurement_(double p_volume, bool p_show_ttl,
                                             const fmc_ui_snapshot_t* p_snapshot,
                                             fmc_ui_frame_t* p_frame)
{
    fm_status_t status;

    if ((p_snapshot == NULL) || (p_frame == NULL) ||
        !fmc_ui_rate_state_is_valid_(&p_snapshot->rate) ||
        (p_snapshot->volume_fractional_digits != FMC_UI_VALUE_FRACTIONAL_DIGITS) ||
        ((p_snapshot->rate.quality == FREQUENCY_OBSERVATION_QUALITY_VALID) &&
         (p_snapshot->rate_fractional_digits != FMC_UI_VALUE_FRACTIONAL_DIGITS)))
    {
        return FM_STATUS_EINVAL;
    }

    status = fmc_ui_set_volume_legend_(p_snapshot->volume_unit, p_frame->alpha_text);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = fmc_ui_set_time_base_indicator_(p_snapshot->rate_time_base, p_frame);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = fmc_ui_format_nonnegative_(p_volume, FMC_UI_TOP_COLUMNS, p_frame->top_text,
                                        sizeof(p_frame->top_text), &p_frame->volume_overflow);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    if (p_snapshot->rate.quality == FREQUENCY_OBSERVATION_QUALITY_VALID)
    {
        status = fmc_ui_format_nonnegative_(p_snapshot->rate.value, FMC_UI_BOTTOM_COLUMNS,
                                            p_frame->bottom_text, sizeof(p_frame->bottom_text),
                                            &p_frame->rate_overflow);
        if (status != FM_STATUS_OK)
        {
            return status;
        }
    }
    else
    {
        uint8_t index;

        for (index = 0U; index < FMC_UI_BOTTOM_COLUMNS; index++)
        {
            p_frame->bottom_text[index] = '-';
        }
        p_frame->bottom_text[FMC_UI_BOTTOM_COLUMNS] = '\0';
        p_frame->rate_overflow = false;
    }

    p_frame->indicator_ttl = p_show_ttl;
    p_frame->indicator_acm_top = !p_show_ttl;
    p_frame->indicator_rate = true;
    p_frame->indicator_slash = true;

    return FM_STATUS_OK;
}

static fm_status_t fmc_ui_set_volume_legend_(fmc_model_volume_unit_t p_unit,
                                             char* p_alpha_text)
{
    if (p_alpha_text == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    switch (p_unit)
    {
    case FMC_MODEL_VOLUME_UNIT_CUSTOM:
        p_alpha_text[0] = '-';
        p_alpha_text[1] = '-';
        break;

    case FMC_MODEL_VOLUME_UNIT_L:
        p_alpha_text[0] = 'L';
        p_alpha_text[1] = 't';
        break;

    case FMC_MODEL_VOLUME_UNIT_M3:
        p_alpha_text[0] = 'M';
        p_alpha_text[1] = '3';
        break;

    case FMC_MODEL_VOLUME_UNIT_GAL_US:
        p_alpha_text[0] = 'G';
        p_alpha_text[1] = 'L';
        break;

    case FMC_MODEL_VOLUME_UNIT_BBL_US:
        p_alpha_text[0] = 'B';
        p_alpha_text[1] = 'R';
        break;

    case FMC_MODEL_VOLUME_UNIT_KG:
        p_alpha_text[0] = 'K';
        p_alpha_text[1] = 'G';
        break;

    case FMC_MODEL_VOLUME_UNIT_EQUIV_M3:
        p_alpha_text[0] = 'M';
        p_alpha_text[1] = 'C';
        break;

    default:
        return FM_STATUS_EINVAL;
    }

    p_alpha_text[2] = '\0';
    return FM_STATUS_OK;
}

static fm_status_t fmc_ui_set_time_base_indicator_(fmc_model_time_base_t p_time_base,
                                                   fmc_ui_frame_t* p_frame)
{
    if (p_frame == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    p_frame->indicator_second = false;
    p_frame->indicator_minute = false;
    p_frame->indicator_hour = false;
    p_frame->indicator_day = false;

    switch (p_time_base)
    {
    case FMC_MODEL_TIME_BASE_SECOND:
        p_frame->indicator_second = true;
        break;

    case FMC_MODEL_TIME_BASE_MINUTE:
        p_frame->indicator_minute = true;
        break;

    case FMC_MODEL_TIME_BASE_HOUR:
        p_frame->indicator_hour = true;
        break;

    case FMC_MODEL_TIME_BASE_DAY:
        p_frame->indicator_day = true;
        break;

    default:
        return FM_STATUS_EINVAL;
    }

    return FM_STATUS_OK;
}

static void fmc_ui_build_placeholder_(const char* p_alpha, fmc_ui_frame_t* p_frame)
{
    uint8_t index;

    for (index = 0U; index < 4U; index++)
    {
        p_frame->bottom_text[index] = ' ';
    }
    p_frame->bottom_text[4] = 'O';
    p_frame->bottom_text[5] = 'F';
    p_frame->bottom_text[6] = 'F';
    p_frame->bottom_text[7] = '\0';
    p_frame->alpha_text[0] = p_alpha[0];
    p_frame->alpha_text[1] = p_alpha[1];
    p_frame->alpha_text[2] = '\0';
}

static fm_status_t fmc_ui_format_nonnegative_(double p_value, uint8_t p_width, char* p_text,
                                              size_t p_text_size, bool* p_overflow)
{
    const display_format_field_t field = {.visible_width = p_width,
                                          .fractional_digits = FMC_UI_VALUE_FRACTIONAL_DIGITS,
                                          .align = DISPLAY_FORMAT_ALIGN_RIGHT,
                                          .pad_char = ' ',
                                          .overflow_policy = DISPLAY_FORMAT_OVERFLOW_ERROR,
                                          .overflow_char = ' '};
    fm_status_t status;

    if ((p_text == NULL) || (p_overflow == NULL) || (p_value < 0.0) || (p_value != p_value) ||
        (p_value > DBL_MAX))
    {
        return FM_STATUS_EINVAL;
    }

    status = DISPLAY_FORMAT_Double(p_value, &field, p_text, p_text_size);
    if (status == FM_STATUS_OK)
    {
        *p_overflow = false;
        return FM_STATUS_OK;
    }

    if (status != FM_STATUS_ERANGE)
    {
        return status;
    }

    status = fmc_ui_format_least_significant_(p_value, p_width, p_text, p_text_size);
    if (status == FM_STATUS_OK)
    {
        *p_overflow = true;
    }

    return status;
}

static fm_status_t fmc_ui_format_least_significant_(double p_value, uint8_t p_width, char* p_text,
                                                    size_t p_text_size)
{
    char digits[FMC_UI_TOP_COLUMNS];
    double scaled_double;
    uint64_t scaled_value;
    uint64_t modulus;
    uint8_t digit_index;
    uint8_t text_index;

    if ((p_text == NULL) || (p_width < 2U) || (p_width > FMC_UI_TOP_COLUMNS) ||
        (p_text_size < ((size_t)p_width + 2U)))
    {
        return FM_STATUS_EINVAL;
    }

    scaled_double = (p_value * 10.0) + 0.5;
    if ((scaled_double < 0.0) || (scaled_double > (double)UINT64_MAX))
    {
        return FM_STATUS_ERANGE;
    }

    scaled_value = (uint64_t)scaled_double;
    modulus = fmc_ui_pow10_u64_(p_width);
    scaled_value %= modulus;

    for (digit_index = p_width; digit_index > 0U; digit_index--)
    {
        digits[digit_index - 1U] = (char)('0' + (scaled_value % 10U));
        scaled_value /= 10U;
    }

    text_index = 0U;
    for (digit_index = 0U; digit_index < p_width; digit_index++)
    {
        if (digit_index == (uint8_t)(p_width - 1U))
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

static uint64_t fmc_ui_pow10_u64_(uint8_t p_digits)
{
    uint64_t value = 1U;
    uint8_t index;

    for (index = 0U; index < p_digits; index++)
    {
        value *= 10U;
    }

    return value;
}
