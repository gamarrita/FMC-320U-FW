/**
 * @file    fm_main.c
 * @brief   Reduced FMC product runtime wiring under ThreadX.
 */
#include "fm_main.h"

#include <stdbool.h>

#include "fm_board.h"
#include "fm_board_keyboard.h"
#include "fm_debug.h"
#include "fm_main_acquisition.h"
#include "fm_main_backlight.h"
#include "fm_main_event.h"
#include "fm_main_ext_button.h"
#include "fm_main_input_recognizer.h"
#include "fm_main_presentation_lcd.h"
#include "fm_main_ui.h"
#include "fm_port_frequency_time.h"
#include "fm_port_pulse_counter.h"
#include "fm_port_rtc.h"
#include "fmc_ui.h"
#include "fmc_runtime.h"
#include "fm_status.h"
#include "tx_api.h"

#define FM_MAIN_EVENT_QUEUE_DEPTH              (8U)
#define FM_MAIN_PERIODIC_REFRESH_TICKS         ((ULONG) TX_TIMER_TICKS_PER_SECOND)
#define FM_MAIN_KEY_HOLD_TICKS                 ((ULONG) (3UL * TX_TIMER_TICKS_PER_SECOND))
#define FM_MAIN_PRESENTATION_TICKS             ((ULONG) (3UL * TX_TIMER_TICKS_PER_SECOND))
#define FM_MAIN_EXT_BUTTON_RELEASE_TICKS        ((ULONG) (TX_TIMER_TICKS_PER_SECOND / 10UL))
#define FM_MAIN_BACKLIGHT_TICKS                 ((ULONG) (10UL * TX_TIMER_TICKS_PER_SECOND))
#define FM_MAIN_EXT_BUTTON_COUNT                (2U)

typedef struct
{
    fmc_runtime_t runtime;
    fm_main_acquisition_t acquisition;
    fmc_ui_t presentation;
    fm_main_input_recognizer_t input_recognizer;
    fm_main_ext_button_t ext_buttons[FM_MAIN_EXT_BUTTON_COUNT];
    ULONG ext_button_release_start_ticks[FM_MAIN_EXT_BUTTON_COUNT];
    ULONG key_hold_start_ticks;
    ULONG presentation_start_ticks;
} fm_main_owner_state_t;

static TX_QUEUE fm_main_event_queue;
static TX_TIMER fm_main_periodic_refresh_timer;
static TX_TIMER fm_main_key_hold_timer;
static TX_TIMER fm_main_presentation_timer;
static TX_TIMER fm_main_ext_button_timers[FM_MAIN_EXT_BUTTON_COUNT];
static TX_TIMER fm_main_backlight_timer;
static fm_main_backlight_t fm_main_backlight;
static ULONG fm_main_event_queue_storage[
    FM_MAIN_EVENT_QUEUE_DEPTH * FM_MAIN_EVENT_QUEUE_MESSAGE_WORDS];
static volatile ULONG fm_main_event_queue_overflow_count;
static volatile ULONG fm_main_event_queue_send_error_count;
static volatile bool fm_main_ext_button_press_enabled[
    FM_MAIN_EXT_BUTTON_COUNT];
static volatile bool fm_main_backlight_activity_pending;

/**
 * @brief Board keyboard callback that serializes a key edge into the owner queue.
 *
 * @param key Board-level key identity reported by the keyboard BSP.
 * @param transition Board-level pressed/released transition.
 *
 * @warning Runs in the producer context selected by the board keyboard module.
 *          It must not call the product runtime directly.
 */
static void fm_main_keyboard_callback_(fm_board_keyboard_key_t key,
                                       fm_board_keyboard_transition_t transition);

/**
 * @brief Publish one app-level event into the private owner queue.
 *
 * Queue overflow and unexpected send failures are recorded for later reporting
 * by the owner loop. This helper is bounded and never waits.
 *
 * @param p_event Event to copy into the owner queue.
 *
 * @return `true` when the event was queued.
 * @return `false` when publication failed and was recorded for owner reporting.
 */
static bool fm_main_event_publish_(const fm_main_event_t *p_event);

/**
 * @brief Coalesce one physical-button press into owner-loop backlight activity.
 *
 * This producer path is independent from semantic input acceptance and never
 * waits. Repeated raw press edges while an activity event is pending collapse
 * into the same request.
 */
static void fm_main_backlight_activity_publish_(void);

/** @brief Consume one coalesced physical-button backlight activity event. */
static void fm_main_backlight_activity_handle_(void);

/** @brief Commit and apply a fresh ten-second backlight interval. */
static void fm_main_backlight_activation_request_(void);

/** @brief Direct one-shot callback that turns off only the current interval. */
static void fm_main_backlight_timer_callback_(ULONG input);

/**
 * @brief Report producer-side queue publication errors from the owner context.
 *
 * Converts accumulated overflow and send-error counters into non-fatal debug
 * diagnostics. The counters may be incremented by ISR/timer producers.
 */
static void fm_main_event_report_publish_errors_(void);

/**
 * @brief Increment a shared event counter under a short interrupt lock.
 *
 * @param p_counter Counter shared between producer contexts and the owner loop.
 */
static void fm_main_counter_increment_(volatile ULONG *p_counter);

/**
 * @brief Dispatch one app-level event received by the owner loop.
 *
 * Keyboard and key-hold timeout events are processed by the owner-loop input
 * recognizer before dispatching semantic input to presentation or
 * `fmc_runtime`. Presentation timeout and periodic refresh events also remain
 * serialized in this owner.
 *
 * @param p_owner Owner-loop state.
 * @param p_event Event received from the owner queue.
 */
static void fm_main_event_handle_(fm_main_owner_state_t *p_owner,
                                  const fm_main_event_t *p_event);

/**
 * @brief Handle one keyboard event in the runtime owner context.
 *
 * PRESSED starts a key hold and arms the long-press timer. RELEASED emits SHORT
 * only when the hold did not already emit LONG. External buttons use their
 * independent stable-release recognizers instead.
 *
 * @param p_owner Owner-loop state.
 * @param p_event Keyboard event received from the owner queue.
 */
static void fm_main_keyboard_handle_event_(
    fm_main_owner_state_t *p_owner,
    const fm_main_event_t *p_event);

static void fm_main_ext_button_handle_event_(
    fm_main_owner_state_t *p_owner,
    const fm_main_event_t *p_event);

static void fm_main_ext_button_release_timer_callback_(ULONG input);

static void fm_main_ext_button_release_timeout_handle_(
    fm_main_owner_state_t *p_owner,
    fm_board_keyboard_key_t key);

static void fm_main_ext_button_output_apply_(
    fm_main_owner_state_t *p_owner,
    fm_board_keyboard_key_t key,
    const fm_main_ext_button_output_t *p_output);

static void fm_main_ext_button_timer_apply_(
    fm_main_owner_state_t *p_owner,
    fm_board_keyboard_key_t key,
    fm_main_ext_button_timer_action_t action);

static fm_main_ext_button_t *fm_main_ext_button_from_key_(
    fm_main_owner_state_t *p_owner,
    fm_board_keyboard_key_t key);

static TX_TIMER *fm_main_ext_button_timer_from_key_(
    fm_board_keyboard_key_t key);

static bool fm_main_ext_button_index_from_key_(
    fm_board_keyboard_key_t key,
    uint8_t *p_index);

static void fm_main_ext_buttons_init_(fm_main_owner_state_t *p_owner);

/**
 * @brief ThreadX key-hold timer callback for long-press recognition.
 *
 * Publishes `FM_MAIN_EVENT_KEY_HOLD_TIMEOUT` with no wait. It deliberately
 * avoids runtime dispatch and recognizer state changes.
 *
 * @param input ThreadX timer input value. Unused.
 */
static void fm_main_key_hold_timer_callback_(ULONG input);

/**
 * @brief Handle one key-hold timeout in the owner loop.
 *
 * The timeout is consumed only if it still belongs to an active hold that has
 * reached the configured 3 second threshold.
 *
 * @param p_owner Owner-loop state.
 */
static void fm_main_key_hold_timeout_handle_(fm_main_owner_state_t *p_owner);

/**
 * @brief Apply one recognizer output in the owner loop.
 *
 * Executes requested timer changes first, then routes any generated semantic
 * input directly to UI.
 *
 * @param p_owner Owner-loop state.
 * @param p_output Recognizer output to apply.
 */
static void fm_main_input_recognizer_output_apply_(
    fm_main_owner_state_t *p_owner,
    const fm_main_input_recognizer_output_t *p_output);

/**
 * @brief Report recognizer abnormal-state statuses as non-fatal diagnostics.
 *
 * @param status Recognizer status to report.
 */
static void fm_main_input_recognizer_status_report_(fm_status_t status);

static void fm_main_semantic_input_handle_(
    fm_main_owner_state_t *p_owner,
    const fmc_input_event_t *p_input);

/**
 * @brief Start or restart the one-shot long-press timer.
 *
 * @param p_owner Owner-loop state that receives the timer start tick.
 */
static void fm_main_key_hold_timer_start_(fm_main_owner_state_t *p_owner);

/**
 * @brief Cancel the one-shot long-press timer.
 */
static void fm_main_key_hold_timer_cancel_(void);

/**
 * @brief Check whether a timeout event is old enough for the current hold.
 *
 * @param p_owner Owner-loop state.
 *
 * @return `true` when the configured hold threshold has elapsed.
 * @return `false` for stale or early timeout delivery.
 */
static bool fm_main_key_hold_timeout_is_current_(
    const fm_main_owner_state_t *p_owner);

/**
 * @brief ThreadX callback for the one-shot startup presentation timer.
 *
 * @param input ThreadX timer input value. Unused.
 */
static void fm_main_presentation_timer_callback_(ULONG input);

/**
 * @brief Advance a temporary startup view after its current timeout.
 *
 * @param p_owner Owner-loop state.
 */
static void fm_main_presentation_timeout_handle_(
    fm_main_owner_state_t *p_owner);

/**
 * @brief Start a fresh nominal period for the current temporary view.
 *
 * @param p_owner Owner-loop state that records the period start tick.
 */
static void fm_main_presentation_timer_start_(
    fm_main_owner_state_t *p_owner);

/**
 * @brief Stop the presentation timer when entering stable TTL/RATE.
 */
static void fm_main_presentation_timer_cancel_(void);

/**
 * @brief Apply timer ownership after a successful presentation transition.
 *
 * @param p_owner Owner-loop state.
 */
static void fm_main_presentation_timer_sync_(
    fm_main_owner_state_t *p_owner);

/**
 * @brief Compose one presentation-only copy of the canonical runtime state.
 *
 * @param p_runtime Runtime owned by the serialized main loop.
 * @param p_snapshot Caller-owned presentation snapshot.
 *
 * @return Runtime snapshot/getter status unchanged.
 */
/**
 * @brief Acknowledge a pending runtime presentation update.
 *
 * Called only after a successful periodic presentation or successful atomic
 * entry to TTL/RATE.
 *
 * @param p_runtime Runtime owned by the serialized main loop.
 */
static void fm_main_presentation_update_acknowledge_(
    fmc_runtime_t *p_runtime);

/**
 * @brief Emit one human-audit trace after successful LCD presentation.
 *
 * Traces are gated by the existing debug-message jumper and never participate
 * in presentation state or timing decisions.
 *
 * @param state Successfully presented state.
 * @param p_cause Constant transition cause such as `START`, `TIMEOUT`,
 *        `ESC_SHORT`, or `REFRESH`.
 */
static void fm_main_presentation_trace_(
    fmc_ui_state_t state,
    const char *p_cause);

/**
 * @brief ThreadX periodic timer callback for the product main refresh source.
 *
 * Publishes `FM_MAIN_EVENT_PERIODIC_REFRESH` with no wait. It deliberately
 * avoids runtime dispatch, board LED access, and blocking work.
 *
 * @param input ThreadX timer input value. Unused.
 */
static void fm_main_periodic_refresh_timer_callback_(ULONG input);

/**
 * @brief Activate the initialized periodic owner-loop event source.
 */
static void fm_main_periodic_refresh_timer_start_(void);

/**
 * @brief Handle one periodic refresh event in the owner loop.
 *
 * Resamples debug jumpers, observes and dispatches independent totalization
 * and frequency samples, and emits optional compact live evidence. It also
 * presents a fresh canonical runtime snapshot while TTL/RATE is active.
 */
static void fm_main_periodic_refresh_handle_(
    fm_main_owner_state_t *p_owner);

/**
 * @brief Emit optional compact ACM/TTL/quality evidence for one cycle.
 *
 * Snapshot or formatting failures are diagnostic-only and never affect
 * acquisition or totalization.
 *
 * @param p_runtime Runtime whose canonical totals are reported.
 */
static void fm_main_live_trace_(const fmc_runtime_t *p_runtime);

/**
 * @brief Emit one `uint64_t` as decimal without formatted-I/O dependencies.
 *
 * @param value Value to emit through the gated debug UART.
 */
static void fm_main_totalization_uint64_trace_(uint64_t value);

/**
 * @brief Require a successful ThreadX status or stop in a debug panic.
 *
 * @param status ThreadX status to validate.
 * @param p_msg Panic message used when status is not `TX_SUCCESS`.
 */
static void fm_main_require_tx_success_(UINT status, const char *p_msg);

/**
 * @brief Require `FM_STATUS_OK` or stop in a debug panic.
 *
 * @param status Project status to validate.
 * @param p_msg Panic message used when status is not `FM_STATUS_OK`.
 */
static void fm_main_require_status_ok_(fm_status_t status, const char *p_msg);

static void fm_main_keyboard_callback_(fm_board_keyboard_key_t key,
                                       fm_board_keyboard_transition_t transition)
{
    fm_main_event_t event;
    uint8_t index;

    if (transition == FM_BOARD_KEYBOARD_TRANSITION_PRESSED)
    {
        fm_main_backlight_activity_publish_();
    }

    if (fm_main_ext_button_index_from_key_(key, &index))
    {
        if ((transition != FM_BOARD_KEYBOARD_TRANSITION_PRESSED) ||
            !fm_main_ext_button_press_enabled[index])
        {
            return;
        }

        fm_main_ext_button_press_enabled[index] = false;
        FM_MAIN_EVENT_MakeKeyboard(&event, key, transition);
        if (!fm_main_event_publish_(&event))
        {
            fm_main_ext_button_press_enabled[index] = true;
        }
        return;
    }

    FM_MAIN_EVENT_MakeKeyboard(&event, key, transition);
    (void) fm_main_event_publish_(&event);
}

static bool fm_main_event_publish_(const fm_main_event_t *p_event)
{
    fm_status_t status;

    status = FM_MAIN_EVENT_Publish(&fm_main_event_queue, p_event);
    if (status == FM_STATUS_OK)
    {
        return true;
    }

    if (status == FM_STATUS_ERANGE)
    {
        fm_main_counter_increment_(&fm_main_event_queue_overflow_count);
        return false;
    }

    fm_main_counter_increment_(&fm_main_event_queue_send_error_count);
    return false;
}

static void fm_main_backlight_activity_publish_(void)
{
    fm_main_event_t event;

    if (fm_main_backlight_activity_pending)
    {
        return;
    }

    fm_main_backlight_activity_pending = true;
    FM_MAIN_EVENT_MakeBacklightActivity(&event);
    (void) fm_main_event_publish_(&event);
}

static void fm_main_backlight_activity_handle_(void)
{
    bool activation_requested;
    UINT interrupt_posture;

    interrupt_posture = tx_interrupt_control(TX_INT_DISABLE);
    activation_requested = fm_main_backlight_activity_pending;
    if (activation_requested)
    {
        fm_main_backlight_activity_pending = false;
    }
    (void) tx_interrupt_control(interrupt_posture);

    if (activation_requested)
    {
        fm_main_backlight_activation_request_();
    }
}

static void fm_main_backlight_activation_request_(void)
{
    fm_main_backlight_output_t output;
    fm_status_t project_status;
    UINT interrupt_posture;
    UINT status;

    interrupt_posture = tx_interrupt_control(TX_INT_DISABLE);
    project_status = FM_MAIN_BACKLIGHT_RequestActivation(
        &fm_main_backlight,
        (uint32_t) tx_time_get(),
        &output);
    (void) tx_interrupt_control(interrupt_posture);
    fm_main_require_status_ok_(project_status,
                               "FM_MAIN:BACKLIGHT_REQUEST");

    if ((output.timer_action != FM_MAIN_BACKLIGHT_TIMER_RESTART) ||
        !output.turn_on || output.turn_off ||
        (output.timer_ticks == 0U))
    {
        FM_DEBUG_PanicMsg("FM_MAIN:BACKLIGHT_OUTPUT");
    }

    status = tx_timer_deactivate(&fm_main_backlight_timer);
    fm_main_require_tx_success_(status, "FM_MAIN:BACKLIGHT_TIMER_STOP");

    status = tx_timer_change(&fm_main_backlight_timer,
                             (ULONG) output.timer_ticks,
                             0U);
    fm_main_require_tx_success_(status, "FM_MAIN:BACKLIGHT_TIMER_CHANGE");

    status = tx_timer_activate(&fm_main_backlight_timer);
    fm_main_require_tx_success_(status, "FM_MAIN:BACKLIGHT_TIMER_START");

    FM_BOARD_BacklightOn();
}

static void fm_main_backlight_timer_callback_(ULONG input)
{
    fm_main_backlight_output_t output;
    fm_status_t status;
    UINT interrupt_posture;

    (void) input;

    interrupt_posture = tx_interrupt_control(TX_INT_DISABLE);
    status = FM_MAIN_BACKLIGHT_HandleTimeout(
        &fm_main_backlight,
        (uint32_t) tx_time_get(),
        &output);
    if ((status == FM_STATUS_OK) && output.turn_off)
    {
        FM_BOARD_BacklightOff();
    }
    (void) tx_interrupt_control(interrupt_posture);

    fm_main_require_status_ok_(status, "FM_MAIN:BACKLIGHT_TIMEOUT");
}

static void fm_main_event_report_publish_errors_(void)
{
    static ULONG reported_overflow_count = 0U;
    static ULONG reported_send_error_count = 0U;
    ULONG overflow_count = fm_main_event_queue_overflow_count;
    ULONG send_error_count = fm_main_event_queue_send_error_count;

    if (overflow_count != reported_overflow_count)
    {
        FM_DEBUG_ReportErrorWithParam(
            FM_DEBUG_ERR_BUFFER_FULL,
            (int32_t) (overflow_count - reported_overflow_count));
        reported_overflow_count = overflow_count;
    }

    if (send_error_count != reported_send_error_count)
    {
        FM_DEBUG_ReportErrorWithParam(
            FM_DEBUG_ERR_BACKEND,
            (int32_t) (send_error_count - reported_send_error_count));
        reported_send_error_count = send_error_count;
    }
}

static void fm_main_counter_increment_(volatile ULONG *p_counter)
{
    UINT interrupt_posture;

    if (p_counter == NULL)
    {
        return;
    }

    interrupt_posture = tx_interrupt_control(TX_INT_DISABLE);
    (*p_counter)++;
    (void) tx_interrupt_control(interrupt_posture);
}

static void fm_main_event_handle_(fm_main_owner_state_t *p_owner,
                                  const fm_main_event_t *p_event)
{
    if (p_event == NULL)
    {
        return;
    }

    switch ((fm_main_event_kind_t) p_event->kind)
    {
    case FM_MAIN_EVENT_KEYBOARD:
        fm_main_keyboard_handle_event_(p_owner, p_event);
        break;

    case FM_MAIN_EVENT_BACKLIGHT_ACTIVITY:
        fm_main_backlight_activity_handle_();
        break;

    case FM_MAIN_EVENT_PERIODIC_REFRESH:
        fm_main_periodic_refresh_handle_(p_owner);
        break;

    case FM_MAIN_EVENT_KEY_HOLD_TIMEOUT:
        fm_main_key_hold_timeout_handle_(p_owner);
        break;

    case FM_MAIN_EVENT_PRESENTATION_TIMEOUT:
        fm_main_presentation_timeout_handle_(p_owner);
        break;

    case FM_MAIN_EVENT_EXT_BUTTON_RELEASE_TIMEOUT:
        fm_main_ext_button_release_timeout_handle_(
            p_owner,
            (fm_board_keyboard_key_t) p_event->key);
        break;

    case FM_MAIN_EVENT_COUNT:
    default:
        FM_DEBUG_ReportError(FM_DEBUG_ERR_BACKEND);
        break;
    }
}

static void fm_main_keyboard_handle_event_(
    fm_main_owner_state_t *p_owner,
    const fm_main_event_t *p_event)
{
    fm_main_input_recognizer_output_t output = {0};
    fm_status_t status;

    if ((p_owner == NULL) || (p_event == NULL))
    {
        return;
    }

    if ((fm_main_event_kind_t) p_event->kind != FM_MAIN_EVENT_KEYBOARD)
    {
        return;
    }

    if (((fm_board_keyboard_key_t) p_event->key ==
         FM_BOARD_KEYBOARD_KEY_EXT_1) ||
        ((fm_board_keyboard_key_t) p_event->key ==
         FM_BOARD_KEYBOARD_KEY_EXT_2))
    {
        fm_main_ext_button_handle_event_(p_owner, p_event);
    }
    else
    {
        status = FM_MAIN_INPUT_RECOGNIZER_HandleKeyboard(
            &p_owner->input_recognizer,
            (fm_board_keyboard_key_t) p_event->key,
            (fm_board_keyboard_transition_t) p_event->transition,
            &output);
        fm_main_input_recognizer_status_report_(status);
        fm_main_input_recognizer_output_apply_(p_owner, &output);
    }
}

static void fm_main_ext_button_handle_event_(
    fm_main_owner_state_t *p_owner,
    const fm_main_event_t *p_event)
{
    fm_main_ext_button_output_t output;
    fm_main_ext_button_t *p_button;
    fm_board_keyboard_key_t key;
    fm_status_t status;

    if ((p_owner == NULL) || (p_event == NULL))
    {
        return;
    }

    key = (fm_board_keyboard_key_t) p_event->key;
    p_button = fm_main_ext_button_from_key_(p_owner, key);
    if (p_button == NULL)
    {
        FM_DEBUG_ReportError(FM_DEBUG_ERR_BACKEND);
        return;
    }

    if ((fm_board_keyboard_transition_t) p_event->transition !=
        FM_BOARD_KEYBOARD_TRANSITION_PRESSED)
    {
        return;
    }

    status = FM_MAIN_EXT_BUTTON_HandlePress(p_button, &output);
    fm_main_require_status_ok_(status, "FM_MAIN:EXT_BUTTON_EDGE");
    fm_main_ext_button_output_apply_(p_owner, key, &output);
}

static void fm_main_ext_button_release_timer_callback_(ULONG input)
{
    fm_main_event_t event;
    fm_board_keyboard_key_t key;

    if (input == 0U)
    {
        key = FM_BOARD_KEYBOARD_KEY_EXT_1;
    }
    else if (input == 1U)
    {
        key = FM_BOARD_KEYBOARD_KEY_EXT_2;
    }
    else
    {
        return;
    }

    FM_MAIN_EVENT_MakeExtButtonReleaseTimeout(&event, key);
    (void) fm_main_event_publish_(&event);
}

static void fm_main_ext_button_release_timeout_handle_(
    fm_main_owner_state_t *p_owner,
    fm_board_keyboard_key_t key)
{
    fm_main_ext_button_output_t output;
    fm_main_ext_button_t *p_button;
    bool pressed;
    fm_status_t status;
    UINT interrupt_posture;
    uint8_t index;

    if (p_owner == NULL)
    {
        return;
    }

    p_button = fm_main_ext_button_from_key_(p_owner, key);
    if ((p_button == NULL) ||
        !fm_main_ext_button_index_from_key_(key, &index) ||
        !FM_BOARD_KeyboardIsPressed(key, &pressed))
    {
        FM_DEBUG_PanicMsg("FM_MAIN:EXT_BUTTON_SAMPLE");
    }

    if ((tx_time_get() - p_owner->ext_button_release_start_ticks[index]) <
        FM_MAIN_EXT_BUTTON_RELEASE_TICKS)
    {
        return;
    }

    status = FM_MAIN_EXT_BUTTON_HandleSampleTimeout(
        p_button,
        pressed,
        &output);
    fm_main_require_status_ok_(status, "FM_MAIN:EXT_BUTTON_TIMEOUT");

    if (p_button->armed)
    {
        interrupt_posture = tx_interrupt_control(TX_INT_DISABLE);
        if (!FM_BOARD_KeyboardIsPressed(key, &pressed))
        {
            (void) tx_interrupt_control(interrupt_posture);
            FM_DEBUG_PanicMsg("FM_MAIN:EXT_BUTTON_REARM_SAMPLE");
        }

        fm_main_ext_button_press_enabled[index] = !pressed;
        (void) tx_interrupt_control(interrupt_posture);

        if (pressed)
        {
            status = FM_MAIN_EXT_BUTTON_HandlePress(p_button, &output);
            fm_main_require_status_ok_(status,
                                       "FM_MAIN:EXT_BUTTON_REARM_PRESS");
        }
    }

    fm_main_ext_button_output_apply_(p_owner, key, &output);
}

static void fm_main_ext_button_output_apply_(
    fm_main_owner_state_t *p_owner,
    fm_board_keyboard_key_t key,
    const fm_main_ext_button_output_t *p_output)
{
    if ((p_owner == NULL) || (p_output == NULL))
    {
        return;
    }

    fm_main_ext_button_timer_apply_(p_owner, key, p_output->timer_action);

    if (p_output->input_valid)
    {
        fm_main_semantic_input_handle_(p_owner, &p_output->input);
    }
}

static void fm_main_ext_button_timer_apply_(
    fm_main_owner_state_t *p_owner,
    fm_board_keyboard_key_t key,
    fm_main_ext_button_timer_action_t action)
{
    TX_TIMER *p_timer;
    UINT status;
    uint8_t index;

    if (action == FM_MAIN_EXT_BUTTON_TIMER_NONE)
    {
        return;
    }

    p_timer = fm_main_ext_button_timer_from_key_(key);
    if ((p_owner == NULL) || (p_timer == NULL) ||
        !fm_main_ext_button_index_from_key_(key, &index))
    {
        FM_DEBUG_PanicMsg("FM_MAIN:EXT_BUTTON_TIMER_KEY");
    }

    status = tx_timer_deactivate(p_timer);
    fm_main_require_tx_success_(status, "FM_MAIN:EXT_BUTTON_TIMER_STOP");

    status = tx_timer_change(
        p_timer,
        FM_MAIN_EXT_BUTTON_RELEASE_TICKS,
        0U);
    fm_main_require_tx_success_(status, "FM_MAIN:EXT_BUTTON_TIMER_CHANGE");

    p_owner->ext_button_release_start_ticks[index] = tx_time_get();

    status = tx_timer_activate(p_timer);
    fm_main_require_tx_success_(status, "FM_MAIN:EXT_BUTTON_TIMER_START");
}

static fm_main_ext_button_t *fm_main_ext_button_from_key_(
    fm_main_owner_state_t *p_owner,
    fm_board_keyboard_key_t key)
{
    if (p_owner == NULL)
    {
        return NULL;
    }

    if (key == FM_BOARD_KEYBOARD_KEY_EXT_1)
    {
        return &p_owner->ext_buttons[0];
    }

    if (key == FM_BOARD_KEYBOARD_KEY_EXT_2)
    {
        return &p_owner->ext_buttons[1];
    }

    return NULL;
}

static TX_TIMER *fm_main_ext_button_timer_from_key_(
    fm_board_keyboard_key_t key)
{
    if (key == FM_BOARD_KEYBOARD_KEY_EXT_1)
    {
        return &fm_main_ext_button_timers[0];
    }

    if (key == FM_BOARD_KEYBOARD_KEY_EXT_2)
    {
        return &fm_main_ext_button_timers[1];
    }

    return NULL;
}

static bool fm_main_ext_button_index_from_key_(
    fm_board_keyboard_key_t key,
    uint8_t *p_index)
{
    if (p_index == NULL)
    {
        return false;
    }

    if (key == FM_BOARD_KEYBOARD_KEY_EXT_1)
    {
        *p_index = 0U;
        return true;
    }

    if (key == FM_BOARD_KEYBOARD_KEY_EXT_2)
    {
        *p_index = 1U;
        return true;
    }

    return false;
}

static void fm_main_ext_buttons_init_(fm_main_owner_state_t *p_owner)
{
    static const fm_board_keyboard_key_t board_keys[] =
    {
        FM_BOARD_KEYBOARD_KEY_EXT_1,
        FM_BOARD_KEYBOARD_KEY_EXT_2
    };
    static const fmc_input_key_t input_keys[] =
    {
        FMC_INPUT_KEY_EXT_1,
        FMC_INPUT_KEY_EXT_2
    };
    fm_main_ext_button_output_t output;
    bool pressed;
    fm_status_t status;
    uint8_t index;

    if (p_owner == NULL)
    {
        return;
    }

    for (index = 0U; index < FM_MAIN_EXT_BUTTON_COUNT; index++)
    {
        p_owner->ext_button_release_start_ticks[index] = 0U;
        fm_main_ext_button_press_enabled[index] = false;
        if (!FM_BOARD_KeyboardIsPressed(board_keys[index], &pressed))
        {
            FM_DEBUG_PanicMsg("FM_MAIN:EXT_BUTTON_INIT_SAMPLE");
        }

        status = FM_MAIN_EXT_BUTTON_Init(
            &p_owner->ext_buttons[index],
            input_keys[index],
            pressed,
            &output);
        fm_main_require_status_ok_(status, "FM_MAIN:EXT_BUTTON_INIT");
        fm_main_ext_button_output_apply_(p_owner, board_keys[index], &output);
    }
}

static void fm_main_key_hold_timer_callback_(ULONG input)
{
    fm_main_event_t event;

    (void) input;

    FM_MAIN_EVENT_MakeKeyHoldTimeout(&event);
    (void) fm_main_event_publish_(&event);
}

static void fm_main_key_hold_timeout_handle_(fm_main_owner_state_t *p_owner)
{
    fm_main_input_recognizer_output_t output = {0};
    fm_status_t status;

    if (p_owner == NULL)
    {
        return;
    }

    if (!fm_main_key_hold_timeout_is_current_(p_owner))
    {
        FM_DEBUG_ReportError(FM_DEBUG_ERR_TIMEOUT);
        return;
    }

    status = FM_MAIN_INPUT_RECOGNIZER_HandleHoldTimeout(
        &p_owner->input_recognizer,
        &output);
    fm_main_input_recognizer_status_report_(status);
    fm_main_input_recognizer_output_apply_(p_owner, &output);
}

static void fm_main_input_recognizer_output_apply_(
    fm_main_owner_state_t *p_owner,
    const fm_main_input_recognizer_output_t *p_output)
{
    if ((p_owner == NULL) || (p_output == NULL))
    {
        return;
    }

    switch (p_output->timer_action)
    {
    case FM_MAIN_INPUT_RECOGNIZER_TIMER_START:
        fm_main_key_hold_timer_start_(p_owner);
        break;

    case FM_MAIN_INPUT_RECOGNIZER_TIMER_CANCEL:
        fm_main_key_hold_timer_cancel_();
        break;

    case FM_MAIN_INPUT_RECOGNIZER_TIMER_NONE:
    default:
        break;
    }

    if (p_output->input_valid)
    {
        fm_main_semantic_input_handle_(p_owner, &p_output->input);
    }
}

static void fm_main_input_recognizer_status_report_(fm_status_t status)
{
    if (status == FM_STATUS_OK)
    {
        return;
    }

    if (status == FM_STATUS_ESTATE)
    {
        FM_DEBUG_ReportError(FM_DEBUG_ERR_BACKEND);
        return;
    }

    fm_main_require_status_ok_(status, "FM_MAIN:INPUT_RECOGNIZER");
}

static void fm_main_semantic_input_handle_(
    fm_main_owner_state_t *p_owner,
    const fmc_input_event_t *p_input)
{
    fm_main_ui_result_t result;
    fm_status_t status;

    if ((p_owner == NULL) || (p_input == NULL))
    {
        return;
    }

    status = FM_MAIN_UI_HandleInput(
        &p_owner->runtime,
        &p_owner->presentation,
        p_input,
        &result);
    fm_main_require_status_ok_(status, "FM_MAIN:UI_INPUT");

    if (result.current_state != result.previous_state)
    {
        fm_main_presentation_timer_sync_(p_owner);
        fm_main_presentation_trace_(result.current_state, "INPUT");
    }
    else if (result.acm_reset_executed)
    {
        fm_main_presentation_trace_(result.current_state, "RESET_ACM");
    }

    if (p_input->action == FMC_INPUT_ACTION_LONG)
    {
        (void) FM_DEBUG_UartStr("FM_MAIN:INPUT_LONG\n");
    }
    else
    {
        (void) FM_DEBUG_UartStr("FM_MAIN:INPUT_SHORT\n");
    }
}

static void fm_main_key_hold_timer_start_(fm_main_owner_state_t *p_owner)
{
    UINT status;

    if (p_owner == NULL)
    {
        return;
    }

    status = tx_timer_deactivate(&fm_main_key_hold_timer);
    fm_main_require_tx_success_(status, "FM_MAIN:KEY_HOLD_TIMER_STOP");

    status = tx_timer_change(&fm_main_key_hold_timer,
                             FM_MAIN_KEY_HOLD_TICKS,
                             0U);
    fm_main_require_tx_success_(status, "FM_MAIN:KEY_HOLD_TIMER_CHANGE");

    p_owner->key_hold_start_ticks = tx_time_get();

    status = tx_timer_activate(&fm_main_key_hold_timer);
    fm_main_require_tx_success_(status, "FM_MAIN:KEY_HOLD_TIMER_START");
}

static void fm_main_key_hold_timer_cancel_(void)
{
    UINT status;

    status = tx_timer_deactivate(&fm_main_key_hold_timer);
    fm_main_require_tx_success_(status, "FM_MAIN:KEY_HOLD_TIMER_CANCEL");
}

static bool fm_main_key_hold_timeout_is_current_(
    const fm_main_owner_state_t *p_owner)
{
    ULONG elapsed_ticks;

    if (p_owner == NULL)
    {
        return false;
    }

    elapsed_ticks = tx_time_get() - p_owner->key_hold_start_ticks;

    return elapsed_ticks >= FM_MAIN_KEY_HOLD_TICKS;
}

static void fm_main_presentation_timer_callback_(ULONG input)
{
    fm_main_event_t event;

    (void) input;

    FM_MAIN_EVENT_MakePresentationTimeout(&event);
    (void) fm_main_event_publish_(&event);
}

static void fm_main_presentation_timeout_handle_(
    fm_main_owner_state_t *p_owner)
{
    fmc_ui_snapshot_t snapshot;
    fmc_ui_state_t state;
    ULONG elapsed_ticks;
    fm_status_t status;

    if (p_owner == NULL)
    {
        return;
    }

    state = FMC_UI_GetState(&p_owner->presentation);
    if ((state != FMC_UI_STATE_ALL_SEGMENTS) &&
        (state != FMC_UI_STATE_FIRMWARE_VERSION))
    {
        return;
    }

    elapsed_ticks = tx_time_get() - p_owner->presentation_start_ticks;
    if (elapsed_ticks < FM_MAIN_PRESENTATION_TICKS)
    {
        return;
    }

    status = FM_MAIN_UI_MakeSnapshot(&p_owner->runtime, &snapshot);
    fm_main_require_status_ok_(status,
                               "FM_MAIN:PRESENTATION_SNAPSHOT");
    status = FMC_UI_Advance(&p_owner->presentation,
                                      &snapshot);
    fm_main_require_status_ok_(status, "FM_MAIN:PRESENTATION_TIMEOUT");
    fm_main_presentation_timer_sync_(p_owner);
    fm_main_presentation_trace_(
        FMC_UI_GetState(&p_owner->presentation),
        "TIMEOUT");
    if (FMC_UI_GetState(&p_owner->presentation) ==
        FMC_UI_STATE_TTL_RATE)
    {
        fm_main_presentation_update_acknowledge_(&p_owner->runtime);
    }
}

static void fm_main_presentation_timer_start_(
    fm_main_owner_state_t *p_owner)
{
    UINT status;

    if (p_owner == NULL)
    {
        return;
    }

    status = tx_timer_deactivate(&fm_main_presentation_timer);
    fm_main_require_tx_success_(status,
                                "FM_MAIN:PRESENTATION_TIMER_STOP");

    status = tx_timer_change(&fm_main_presentation_timer,
                             FM_MAIN_PRESENTATION_TICKS,
                             0U);
    fm_main_require_tx_success_(status,
                                "FM_MAIN:PRESENTATION_TIMER_CHANGE");

    p_owner->presentation_start_ticks = tx_time_get();

    status = tx_timer_activate(&fm_main_presentation_timer);
    fm_main_require_tx_success_(status,
                                "FM_MAIN:PRESENTATION_TIMER_START");
}

static void fm_main_presentation_timer_cancel_(void)
{
    UINT status;

    status = tx_timer_deactivate(&fm_main_presentation_timer);
    fm_main_require_tx_success_(status,
                                "FM_MAIN:PRESENTATION_TIMER_CANCEL");
}

static void fm_main_presentation_timer_sync_(
    fm_main_owner_state_t *p_owner)
{
    fmc_ui_state_t state;

    if (p_owner == NULL)
    {
        return;
    }

    state = FMC_UI_GetState(&p_owner->presentation);
    if ((state == FMC_UI_STATE_ALL_SEGMENTS) ||
        (state == FMC_UI_STATE_FIRMWARE_VERSION))
    {
        fm_main_presentation_timer_start_(p_owner);
        return;
    }

    fm_main_presentation_timer_cancel_();
}

static void fm_main_presentation_update_acknowledge_(fmc_runtime_t* p_runtime)
{
    fm_status_t status;

    status = FMC_RUNTIME_ClearPresentationUpdatePending(p_runtime);
    fm_main_require_status_ok_(status, "FM_MAIN:PRESENTATION_ACK");
}

static void fm_main_presentation_trace_(fmc_ui_state_t state, const char* p_cause)
{
    const char* p_state;

    if (!FM_DEBUG_MsgIsEnabled() || (p_cause == NULL))
    {
        return;
    }

    switch (state)
    {
    case FMC_UI_STATE_ALL_SEGMENTS:
        p_state = "ALL";
        break;

    case FMC_UI_STATE_FIRMWARE_VERSION:
        p_state = "VER";
        break;

    case FMC_UI_STATE_TTL_RATE:
        p_state = "TTL";
        break;

    case FMC_UI_STATE_ACM_RATE:
        p_state = "ACM";
        break;

    case FMC_UI_STATE_PRINT:
        p_state = "PR";
        break;

    case FMC_UI_STATE_LOG_DOWNLOAD:
        p_state = "LD";
        break;

    case FMC_UI_STATE_DATE_TIME:
        p_state = "DT";
        break;

    case FMC_UI_STATE_NOT_STARTED:
    case FMC_UI_STATE_COUNT:
    default:
        p_state = "NONE";
        break;
    }

    (void) FM_DEBUG_UartStr("FM:P=");
    (void) FM_DEBUG_UartStr(p_state);
    (void) FM_DEBUG_UartStr(" C=");
    (void) FM_DEBUG_UartStr(p_cause);
    (void) FM_DEBUG_UartStr("\n");
}

static void fm_main_periodic_refresh_timer_callback_(ULONG input)
{
    fm_main_event_t event;

    (void) input;

    FM_MAIN_EVENT_MakePeriodicRefresh(&event);
    (void) fm_main_event_publish_(&event);
}

static void fm_main_periodic_refresh_timer_start_(void)
{
    UINT status;

    status = tx_timer_activate(&fm_main_periodic_refresh_timer);
    fm_main_require_tx_success_(status, "FM_MAIN:REFRESH_TIMER_START");
}

static void fm_main_periodic_refresh_handle_(
    fm_main_owner_state_t *p_owner)
{
    frequency_observation_sample_t frequency_sample;
    fm_status_t status;
    uint16_t current_count;
    uint64_t pulse_delta;

    if (p_owner == NULL)
    {
        return;
    }

    FM_DEBUG_RefreshJumpers();
    FM_DEBUG_LedRun(FM_DEBUG_LED_ON);

    current_count = FM_PORT_PulseCounterReadStable();
    status = FM_MAIN_ACQUISITION_ProcessObservation(
        &p_owner->acquisition,
        current_count,
        &p_owner->runtime,
        &pulse_delta);
    fm_main_require_status_ok_(status, "FM_MAIN:ACQUISITION");

    status = FMC_UI_ObservePulseDelta(
        &p_owner->presentation,
        pulse_delta);
    fm_main_require_status_ok_(status, "FM_MAIN:POINT_OBSERVATION");

    /*
     * Frequency owns an independent counter baseline. A pulse may arrive
     * between this read and the totalization read above; it then belongs to
     * the current frequency window and the next totalization delta.
     */
    frequency_sample.pulse_count =
        FM_PORT_PulseCounterReadStable();
    status = FM_PORT_FrequencyTimeRead(
        &frequency_sample.timestamp_us);
    fm_main_require_status_ok_(status, "FM_MAIN:FREQUENCY_TIME");
    status = FM_MAIN_ACQUISITION_ProcessFrequencyObservation(
        &p_owner->acquisition,
        &frequency_sample,
        &p_owner->runtime);
    fm_main_require_status_ok_(status, "FM_MAIN:FREQUENCY");

    fm_main_live_trace_(&p_owner->runtime);

    if ((FMC_UI_GetState(&p_owner->presentation) >=
         FMC_UI_STATE_TTL_RATE) &&
        (FMC_UI_GetState(&p_owner->presentation) <=
         FMC_UI_STATE_DATE_TIME))
    {
        status = FM_MAIN_UI_Refresh(
            &p_owner->runtime,
            &p_owner->presentation);
        fm_main_require_status_ok_(status,
                                   "FM_MAIN:PRESENTATION_REFRESH");
    }

    FM_DEBUG_LedRun(FM_DEBUG_LED_OFF);
}

static void fm_main_live_trace_(const fmc_runtime_t *p_runtime)
{
    fmc_service_snapshot_t snapshot;
    fmc_runtime_rate_state_t rate;
    char quality;
    fm_status_t status;

    if (!FM_DEBUG_MsgIsEnabled() || (p_runtime == NULL))
    {
        return;
    }

    status = FMC_RUNTIME_GetSnapshot(p_runtime, &snapshot);
    if (status != FM_STATUS_OK)
    {
        return;
    }

    status = FMC_RUNTIME_GetRateState(p_runtime, &rate);
    if (status != FM_STATUS_OK)
    {
        return;
    }

    switch (rate.quality)
    {
    case FREQUENCY_OBSERVATION_QUALITY_VALID:
        quality = 'V';
        break;
    case FREQUENCY_OBSERVATION_QUALITY_UNAVAILABLE:
        quality = 'U';
        break;
    case FREQUENCY_OBSERVATION_QUALITY_STALE:
        quality = 'S';
        break;
    case FREQUENCY_OBSERVATION_QUALITY_INVALID:
    default:
        quality = 'I';
        break;
    }

    FM_DEBUG_UartStr("FM:LIVE A=");
    fm_main_totalization_uint64_trace_(snapshot.model.acm.pulses);
    FM_DEBUG_UartStr(" T=");
    fm_main_totalization_uint64_trace_(snapshot.model.ttl.pulses);
    FM_DEBUG_UartStr(" Q=");
    FM_DEBUG_UartMsg(&quality, 1U);
    FM_DEBUG_UartStr("\n");
}

static void fm_main_totalization_uint64_trace_(uint64_t value)
{
    char digits[20];
    uint32_t length = 0U;

    do
    {
        digits[(sizeof(digits) - 1U) - length] =
            (char) ('0' + (value % 10U));
        value /= 10U;
        length++;
    } while ((value != 0U) && (length < sizeof(digits)));

    FM_DEBUG_UartMsg(&digits[sizeof(digits) - length], length);
}

static void fm_main_require_tx_success_(UINT status, const char *p_msg)
{
    if (status != TX_SUCCESS)
    {
        FM_DEBUG_PanicMsg(p_msg);
    }
}

static void fm_main_require_status_ok_(fm_status_t status, const char *p_msg)
{
    if (status != FM_STATUS_OK)
    {
        FM_DEBUG_PanicMsg(p_msg);
    }
}

void FM_MAIN_Init(void)
{
    fm_status_t project_status;
    UINT status;

    FM_BOARD_Init();
    FM_PORT_RTC_Init();
    FM_DEBUG_Init();

    fm_main_event_queue_overflow_count = 0U;
    fm_main_event_queue_send_error_count = 0U;
    fm_main_ext_button_press_enabled[0] = false;
    fm_main_ext_button_press_enabled[1] = false;
    fm_main_backlight_activity_pending = false;

    project_status = FM_MAIN_BACKLIGHT_Init(
        &fm_main_backlight,
        (uint32_t) FM_MAIN_BACKLIGHT_TICKS);
    fm_main_require_status_ok_(project_status, "FM_MAIN:BACKLIGHT_INIT");

    status = tx_queue_create(&fm_main_event_queue,
                             (CHAR *) "FM_MAIN_EVENT",
                             FM_MAIN_EVENT_QUEUE_MESSAGE_WORDS,
                             fm_main_event_queue_storage,
                             sizeof(fm_main_event_queue_storage));
    fm_main_require_tx_success_(status, "FM_MAIN:EVENT_QUEUE_CREATE");

    if ((FM_MAIN_PERIODIC_REFRESH_TICKS == 0U) ||
        (FM_MAIN_KEY_HOLD_TICKS == 0U) ||
        (FM_MAIN_PRESENTATION_TICKS == 0U) ||
        (FM_MAIN_EXT_BUTTON_RELEASE_TICKS == 0U) ||
        (FM_MAIN_BACKLIGHT_TICKS == 0U))
    {
        FM_DEBUG_PanicMsg("FM_MAIN:TIMER_TICKS_ZERO");
    }

    status = tx_timer_create(&fm_main_periodic_refresh_timer,
                             (CHAR *) "FM_MAIN_REFRESH",
                             fm_main_periodic_refresh_timer_callback_,
                             0U,
                             FM_MAIN_PERIODIC_REFRESH_TICKS,
                             FM_MAIN_PERIODIC_REFRESH_TICKS,
                             TX_NO_ACTIVATE);
    fm_main_require_tx_success_(status, "FM_MAIN:REFRESH_TIMER_CREATE");

    status = tx_timer_create(&fm_main_key_hold_timer,
                             (CHAR *) "FM_MAIN_KEY_HOLD",
                             fm_main_key_hold_timer_callback_,
                             0U,
                             FM_MAIN_KEY_HOLD_TICKS,
                             0U,
                             TX_NO_ACTIVATE);
    fm_main_require_tx_success_(status, "FM_MAIN:KEY_HOLD_TIMER_CREATE");

    status = tx_timer_create(&fm_main_presentation_timer,
                             (CHAR *) "FM_MAIN_PRESENTATION",
                             fm_main_presentation_timer_callback_,
                             0U,
                             FM_MAIN_PRESENTATION_TICKS,
                             0U,
                             TX_NO_ACTIVATE);
    fm_main_require_tx_success_(
        status,
        "FM_MAIN:PRESENTATION_TIMER_CREATE");

    status = tx_timer_create(&fm_main_ext_button_timers[0],
                             (CHAR *) "FM_MAIN_EXT_1_RELEASE",
                             fm_main_ext_button_release_timer_callback_,
                             0U,
                             FM_MAIN_EXT_BUTTON_RELEASE_TICKS,
                             0U,
                             TX_NO_ACTIVATE);
    fm_main_require_tx_success_(status, "FM_MAIN:EXT_1_TIMER_CREATE");

    status = tx_timer_create(&fm_main_ext_button_timers[1],
                             (CHAR *) "FM_MAIN_EXT_2_RELEASE",
                             fm_main_ext_button_release_timer_callback_,
                             1U,
                             FM_MAIN_EXT_BUTTON_RELEASE_TICKS,
                             0U,
                             TX_NO_ACTIVATE);
    fm_main_require_tx_success_(status, "FM_MAIN:EXT_2_TIMER_CREATE");

    status = tx_timer_create(&fm_main_backlight_timer,
                             (CHAR *) "FM_MAIN_BACKLIGHT",
                             fm_main_backlight_timer_callback_,
                             0U,
                             FM_MAIN_BACKLIGHT_TICKS,
                             0U,
                             TX_NO_ACTIVATE);
    fm_main_require_tx_success_(status,
                                "FM_MAIN:BACKLIGHT_TIMER_CREATE");
}

void FM_MAIN_Main(void)
{
    fm_main_owner_state_t owner;
    frequency_observation_sample_t frequency_baseline;
    fmc_ui_snapshot_t presentation_snapshot;
    fm_main_event_t event;
    fm_status_t project_status;
    UINT status;

    FM_MAIN_Init();
    FMC_RUNTIME_Init(&owner.runtime);
    FM_MAIN_ACQUISITION_Init(&owner.acquisition);
    project_status = FM_PORT_FrequencyTimeStart();
    fm_main_require_status_ok_(project_status,
                               "FM_MAIN:FREQUENCY_TIME_START");
    if (!FM_PORT_PulseCounterStart())
    {
        FM_DEBUG_PanicMsg("FM_MAIN:PULSE_COUNTER_START");
    }

    /*
     * Establish the frequency baseline immediately without waiting for a
     * nonzero counter. This keeps zero-frequency startup valid and leaves the
     * pulse-delta observer's accepted zero baseline independent.
     */
    frequency_baseline.pulse_count =
        FM_PORT_PulseCounterReadStable();
    project_status = FM_PORT_FrequencyTimeRead(
        &frequency_baseline.timestamp_us);
    fm_main_require_status_ok_(project_status,
                               "FM_MAIN:FREQUENCY_TIME_BASELINE");
    project_status =
        FM_MAIN_ACQUISITION_ProcessFrequencyObservation(
            &owner.acquisition,
            &frequency_baseline,
            &owner.runtime);
    fm_main_require_status_ok_(project_status,
                               "FM_MAIN:FREQUENCY_BASELINE");

    FM_MAIN_INPUT_RECOGNIZER_Init(&owner.input_recognizer);
    owner.key_hold_start_ticks = 0U;
    owner.presentation_start_ticks = 0U;

    project_status = FM_MAIN_PRESENTATION_LCD_Init();
    fm_main_require_status_ok_(project_status,
                               "FM_MAIN:LCD_INIT");

    project_status = FM_MAIN_UI_MakeSnapshot(
        &owner.runtime,
        &presentation_snapshot);
    fm_main_require_status_ok_(project_status,
                               "FM_MAIN:PRESENTATION_SNAPSHOT");
    project_status = FMC_UI_Init(
        &owner.presentation,
        &presentation_snapshot,
        FM_MAIN_PRESENTATION_LCD_Present,
        NULL);
    fm_main_require_status_ok_(project_status,
                               "FM_MAIN:PRESENTATION_INIT");

    project_status = FMC_UI_Start(&owner.presentation);
    fm_main_require_status_ok_(project_status,
                               "FM_MAIN:PRESENTATION_START");
    fm_main_backlight_activation_request_();
    fm_main_presentation_timer_sync_(&owner);
    fm_main_presentation_trace_(
        FMC_UI_GetState(&owner.presentation),
        "START");

    FM_BOARD_KeyboardInit();
    fm_main_ext_buttons_init_(&owner);
    FM_BOARD_KeyboardSetCallback(fm_main_keyboard_callback_);

    fm_main_periodic_refresh_timer_start_();
    (void) FM_DEBUG_UartStr("FM_MAIN:READY\n");

    for (;;)
    {
        status = tx_queue_receive(&fm_main_event_queue,
                                  &event,
                                  TX_WAIT_FOREVER);
        fm_main_require_tx_success_(status, "FM_MAIN:EVENT_QUEUE_RX");

        /* Recover a coalesced activity request even if its wake event was full. */
        fm_main_backlight_activity_handle_();
        fm_main_event_report_publish_errors_();
        fm_main_event_handle_(&owner, &event);
        FM_DEBUG_Flush();
    }
}
