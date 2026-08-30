/**
 * @file    fmc_ui.h
 * @brief   RTOS-neutral FMC startup and user-menu UI contract.
 *
 * This module owns startup plus the five Phase 8 user states, semantic frame
 * composition, bounded/cyclic navigation, logical POINT state, and the
 * bounded UI request vocabulary. It does not access LCD hardware, own timers,
 * calculate measurements, call runtime services, or execute requests.
 */

#ifndef FMC_UI_H
#define FMC_UI_H

#include <stdbool.h>
#include <stdint.h>

#include "fm_status.h"
#include "fmc_input.h"
#include "fmc_model.h"
#include "fmc_runtime.h"

#define FMC_UI_TOP_TEXT_SIZE 10U
#define FMC_UI_BOTTOM_TEXT_SIZE 10U
#define FMC_UI_ALPHA_TEXT_SIZE 3U
#define FMC_UI_VALUE_FRACTIONAL_DIGITS 1U

typedef enum
{
    FMC_UI_STATE_NOT_STARTED = 0,
    FMC_UI_STATE_ALL_SEGMENTS,
    FMC_UI_STATE_FIRMWARE_VERSION,
    FMC_UI_STATE_TTL_RATE,
    FMC_UI_STATE_ACM_RATE,
    FMC_UI_STATE_PRINT,
    FMC_UI_STATE_LOG_DOWNLOAD,
    FMC_UI_STATE_DATE_TIME,
    FMC_UI_STATE_COUNT
} fmc_ui_state_t;

typedef enum
{
    FMC_UI_REQUEST_NONE = 0,
    FMC_UI_REQUEST_RESET_ACM,
    FMC_UI_REQUEST_COUNT
} fmc_ui_request_kind_t;

/**
 * @brief One owner-executed request produced by semantic UI input.
 *
 * Phase 8 requests contain exactly one request kind and no payload. The caller
 * owns authorization consequences, runtime execution, and any fresh snapshot
 * or presentation following a successful operation.
 */
typedef struct
{
    fmc_ui_request_kind_t kind;
} fmc_ui_request_t;

/**
 * @brief Coherent values supplied to the FMC UI.
 *
 * ACM, TTL, and RATE are accepted product values. The UI does not derive them
 * or own the RATE observation window. RATE value, presence, and quality remain
 * together in the runtime-owned state copied into this snapshot.
 *
 * Configuration values are accepted before they enter this snapshot. The UI
 * renders every volume unit and RATE time base defined by `fmc_model.h`; it
 * does not apply configuration policy. Phase 8 retains one fractional digit.
 * `UNAVAILABLE`, `STALE`, and `INVALID` RATE are rendered through the common
 * nonnumeric product representation.
 */
typedef struct
{
    double acm;
    double ttl;
    fmc_runtime_rate_state_t rate;
    fmc_model_volume_unit_t volume_unit;
    fmc_model_time_base_t rate_time_base;
    uint8_t volume_fractional_digits;
    uint8_t rate_fractional_digits;
} fmc_ui_snapshot_t;

/**
 * @brief Semantic display frame independent from physical LCD mapping.
 *
 * When `all_segments` is true, all other fields are ignored and the sink must
 * activate every software-controllable physical LCD segment. ACM indicators
 * remain positional because the glass has independent upper and lower
 * elements. Phase 8 ACM/RATE uses only `indicator_acm_top`.
 */
typedef struct
{
    bool all_segments;
    char top_text[FMC_UI_TOP_TEXT_SIZE];
    char bottom_text[FMC_UI_BOTTOM_TEXT_SIZE];
    char alpha_text[FMC_UI_ALPHA_TEXT_SIZE];
    bool indicator_point;
    bool indicator_acm_top;
    bool indicator_acm_bottom;
    bool indicator_ttl;
    bool indicator_rate;
    bool indicator_slash;
    bool indicator_second;
    bool indicator_minute;
    bool indicator_hour;
    bool indicator_day;
    bool volume_overflow;
    bool rate_overflow;
} fmc_ui_frame_t;

typedef fm_status_t (*fmc_ui_sink_t)(const fmc_ui_frame_t* p_frame, void* p_context);

typedef struct
{
    fmc_ui_state_t state;
    fmc_ui_snapshot_t snapshot;
    bool point_on;
    fmc_ui_sink_t sink;
    void* sink_context;
} fmc_ui_t;

/**
 * @brief Initialize a UI instance without showing a frame.
 *
 * POINT starts off and observations remain ignored until the first successful
 * entry to TTL/RATE.
 *
 * @return `FM_STATUS_OK` on success.
 * @return `FM_STATUS_EINVAL` for invalid pointers.
 */
fm_status_t FMC_UI_Init(fmc_ui_t* p_ui, const fmc_ui_snapshot_t* p_snapshot, fmc_ui_sink_t p_sink,
                        void* p_sink_context);

/**
 * @brief Present the initial all-segments state.
 *
 * State changes only after the sink accepts the frame.
 *
 * @return `FM_STATUS_OK` when the frame was presented.
 * @return `FM_STATUS_EINVAL` when `p_ui` is `NULL`.
 * @return `FM_STATUS_ESTATE` when already started.
 * @return Other sink or formatting errors unchanged.
 */
fm_status_t FMC_UI_Start(fmc_ui_t* p_ui);

/**
 * @brief Advance one startup state using the latest coherent snapshot.
 *
 * All-segments advances to firmware version, firmware version advances to
 * TTL/RATE, and every user-menu state remains stable. The stored snapshot is
 * replaced only after successful frame delivery.
 *
 * @return `FM_STATUS_OK` on success or when already in the user menu.
 * @return `FM_STATUS_EINVAL` for invalid pointers or measurement content that
 *         must be rendered during the transition.
 * @return `FM_STATUS_ESTATE` before startup or for corrupted state.
 * @return Other sink or formatting errors unchanged.
 */
fm_status_t FMC_UI_Advance(fmc_ui_t* p_ui, const fmc_ui_snapshot_t* p_snapshot);

/**
 * @brief Apply one semantic input and return an owner-executed request.
 *
 * SHORT ESC advances either temporary startup view. In the user menu, SHORT
 * DOWN and UP navigate without wrapping, EXT_1 SHORT advances with wrapping,
 * and LONG ENTER or EXT_2 SHORT requests ACM reset only on ACM/RATE. Entry to
 * another state presents its fresh frame immediately. All accepted unassigned
 * actions are no-ops.
 *
 * When `p_request` is non-NULL, its kind is initialized to
 * `FMC_UI_REQUEST_NONE` before other arguments are validated. A failed
 * presentation does not change state or stored snapshot.
 *
 * @return `FM_STATUS_OK` for an applied consequence or accepted no-op.
 * @return `FM_STATUS_EINVAL` for invalid pointers, key, action, an external-
 *         button LONG combination, or measurement content required by a
 *         presented destination state.
 * @return `FM_STATUS_ESTATE` before startup or for corrupted state.
 * @return Other sink or formatting errors unchanged.
 */
fm_status_t FMC_UI_HandleInput(fmc_ui_t* p_ui, const fmc_input_event_t* p_input,
                               const fmc_ui_snapshot_t* p_snapshot, fmc_ui_request_t* p_request);

/**
 * @brief Present the active user-menu state from a fresh coherent snapshot.
 *
 * The stored snapshot is replaced only after successful frame delivery.
 * Startup states are not periodically refreshed.
 *
 * @return `FM_STATUS_OK` when the frame was presented.
 * @return `FM_STATUS_EINVAL` for invalid pointers or measurement content
 *         required by the active screen.
 * @return `FM_STATUS_ESTATE` outside the five user-menu states.
 * @return Other sink or formatting errors unchanged.
 */
fm_status_t FMC_UI_Refresh(fmc_ui_t* p_ui, const fmc_ui_snapshot_t* p_snapshot);

/**
 * @brief Update logical user-menu POINT state from one pulse observation.
 *
 * In the user menu, zero clears POINT and a nonzero observation toggles it
 * exactly once. Startup observations are accepted no-ops. This operation does
 * not present a frame; the caller applies the resulting state through the
 * cycle's single `FMC_UI_Refresh()` call.
 *
 * @return `FM_STATUS_OK` on success or for a startup no-op.
 * @return `FM_STATUS_EINVAL` when `p_ui` is `NULL`.
 * @return `FM_STATUS_ESTATE` for corrupted state.
 */
fm_status_t FMC_UI_ObservePulseDelta(fmc_ui_t* p_ui, uint64_t p_pulse_delta);

/**
 * @brief Return the current successfully presented state.
 *
 * @return Current state, or `FMC_UI_STATE_NOT_STARTED` for `NULL`.
 */
fmc_ui_state_t FMC_UI_GetState(const fmc_ui_t* p_ui);

#endif /* FMC_UI_H */
