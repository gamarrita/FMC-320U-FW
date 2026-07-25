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
#include "fm_main_event.h"
#include "fm_main_input_adapter.h"
#include "fm_main_input_recognizer.h"
#include "fm_main_presentation_lcd.h"
#include "fm_port_pulse_counter.h"
#include "fm_port_rtc.h"
#include "fmc_presentation.h"
#include "fmc_runtime.h"
#include "fm_status.h"
#include "tx_api.h"

#define FM_MAIN_EVENT_QUEUE_DEPTH              (8U)
#define FM_MAIN_PERIODIC_REFRESH_TICKS         ((ULONG) TX_TIMER_TICKS_PER_SECOND)
#define FM_MAIN_KEY_HOLD_TICKS                 ((ULONG) (3UL * TX_TIMER_TICKS_PER_SECOND))
#define FM_MAIN_PRESENTATION_TICKS             ((ULONG) (3UL * TX_TIMER_TICKS_PER_SECOND))

typedef struct
{
    fmc_runtime_t runtime;
    fm_main_acquisition_t acquisition;
    fmc_presentation_t presentation;
    fm_main_input_recognizer_t input_recognizer;
    ULONG key_hold_start_ticks;
    ULONG presentation_start_ticks;
} fm_main_owner_state_t;

static TX_QUEUE fm_main_event_queue;
static TX_TIMER fm_main_periodic_refresh_timer;
static TX_TIMER fm_main_key_hold_timer;
static TX_TIMER fm_main_presentation_timer;
static ULONG fm_main_event_queue_storage[
    FM_MAIN_EVENT_QUEUE_DEPTH * FM_MAIN_EVENT_QUEUE_MESSAGE_WORDS];
static volatile ULONG fm_main_event_queue_overflow_count;
static volatile ULONG fm_main_event_queue_send_error_count;

/**
 * @brief Board keyboard callback that serializes a key edge into the owner queue.
 *
 * @param key Board-level key identity reported by the keyboard BSP.
 * @param edge Board-level edge reported by the keyboard BSP.
 *
 * @warning Runs in the producer context selected by the board keyboard module.
 *          It must not call the product runtime directly.
 */
static void fm_main_keyboard_callback_(fm_board_keyboard_key_t key,
                                       fm_board_keyboard_edge_t edge);

/**
 * @brief Publish one app-level event into the private owner queue.
 *
 * Queue overflow and unexpected send failures are recorded for later reporting
 * by the owner loop. This helper is bounded and never waits.
 *
 * @param p_event Event to copy into the owner queue.
 */
static void fm_main_event_publish_(const fm_main_event_t *p_event);

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
 * RISING starts a key hold and arms the long-press timer. FALLING emits SHORT
 * only when the hold did not already emit LONG.
 *
 * @param p_owner Owner-loop state.
 * @param p_event Keyboard event received from the owner queue.
 */
static void fm_main_keyboard_handle_event_(
    fm_main_owner_state_t *p_owner,
    const fm_main_event_t *p_event);

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
 * Executes requested timer changes first, then dispatches any generated runtime
 * input event.
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

/**
 * @brief Dispatch one semantic runtime input and log its action.
 *
 * @param p_runtime Runtime owned by `FM_MAIN_Main()`.
 * @param p_event Runtime input event.
 */
static void fm_main_runtime_dispatch_input_(
    fmc_runtime_t *p_runtime,
    const fmc_runtime_event_t *p_event);

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
 * @brief ThreadX callback for the one-shot Phase 6A presentation timer.
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
    fmc_presentation_state_t state,
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
 * Resamples debug jumpers, observes and dispatches the pulse counter once, and
 * emits optional totalization evidence. It also re-presents the current
 * provisional Phase 6A TTL/RATE snapshot while that stable view is active.
 */
static void fm_main_periodic_refresh_handle_(
    fm_main_owner_state_t *p_owner);

/**
 * @brief Emit optional ACM/TTL pulse-total evidence for one periodic cycle.
 *
 * Snapshot or formatting failures are diagnostic-only and never affect
 * acquisition or totalization.
 *
 * @param p_runtime Runtime whose canonical totals are reported.
 */
static void fm_main_totalization_trace_(const fmc_runtime_t *p_runtime);

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
                                       fm_board_keyboard_edge_t edge)
{
    fm_main_event_t event;

    FM_MAIN_EVENT_MakeKeyboard(&event, key, edge);
    fm_main_event_publish_(&event);
}

static void fm_main_event_publish_(const fm_main_event_t *p_event)
{
    fm_status_t status;

    status = FM_MAIN_EVENT_Publish(&fm_main_event_queue, p_event);
    if (status == FM_STATUS_OK)
    {
        return;
    }

    if (status == FM_STATUS_ERANGE)
    {
        fm_main_counter_increment_(&fm_main_event_queue_overflow_count);
        return;
    }

    fm_main_counter_increment_(&fm_main_event_queue_send_error_count);
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

    case FM_MAIN_EVENT_PERIODIC_REFRESH:
        fm_main_periodic_refresh_handle_(p_owner);
        break;

    case FM_MAIN_EVENT_KEY_HOLD_TIMEOUT:
        fm_main_key_hold_timeout_handle_(p_owner);
        break;

    case FM_MAIN_EVENT_PRESENTATION_TIMEOUT:
        fm_main_presentation_timeout_handle_(p_owner);
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

    status = FM_MAIN_INPUT_RECOGNIZER_HandleKeyboard(
        &p_owner->input_recognizer,
        (fm_board_keyboard_key_t) p_event->key,
        (fm_board_keyboard_edge_t) p_event->edge,
        &output);
    fm_main_input_recognizer_status_report_(status);
    fm_main_input_recognizer_output_apply_(p_owner, &output);
}

static void fm_main_key_hold_timer_callback_(ULONG input)
{
    fm_main_event_t event;

    (void) input;

    FM_MAIN_EVENT_MakeKeyHoldTimeout(&event);
    fm_main_event_publish_(&event);
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

    if (p_output->runtime_event_valid)
    {
        fmc_presentation_state_t previous_state;
        fmc_presentation_state_t current_state;
        fm_status_t status;

        previous_state = FMC_PRESENTATION_GetState(
            &p_owner->presentation);
        status = FMC_PRESENTATION_HandleInput(
            &p_owner->presentation,
            &p_output->runtime_event.data.input);
        fm_main_require_status_ok_(status, "FM_MAIN:PRESENTATION_INPUT");
        current_state = FMC_PRESENTATION_GetState(
            &p_owner->presentation);

        if (current_state != previous_state)
        {
            fm_main_presentation_timer_sync_(p_owner);
            fm_main_presentation_trace_(current_state, "ESC_SHORT");
            return;
        }

        fm_main_runtime_dispatch_input_(&p_owner->runtime,
                                        &p_output->runtime_event);
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

static void fm_main_runtime_dispatch_input_(
    fmc_runtime_t *p_runtime,
    const fmc_runtime_event_t *p_event)
{
    fm_status_t status;

    if ((p_runtime == NULL) || (p_event == NULL))
    {
        return;
    }

    status = FMC_RUNTIME_Dispatch(p_runtime, p_event);
    fm_main_require_status_ok_(status, "FM_MAIN:RUNTIME_DISPATCH");

    if (p_event->data.input.action == FMC_INPUT_ACTION_LONG)
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
    fm_main_event_publish_(&event);
}

static void fm_main_presentation_timeout_handle_(
    fm_main_owner_state_t *p_owner)
{
    fmc_presentation_state_t state;
    ULONG elapsed_ticks;
    fm_status_t status;

    if (p_owner == NULL)
    {
        return;
    }

    state = FMC_PRESENTATION_GetState(&p_owner->presentation);
    if ((state != FMC_PRESENTATION_STATE_ALL_SEGMENTS) &&
        (state != FMC_PRESENTATION_STATE_FIRMWARE_VERSION))
    {
        return;
    }

    elapsed_ticks = tx_time_get() - p_owner->presentation_start_ticks;
    if (elapsed_ticks < FM_MAIN_PRESENTATION_TICKS)
    {
        return;
    }

    status = FMC_PRESENTATION_Advance(&p_owner->presentation);
    fm_main_require_status_ok_(status, "FM_MAIN:PRESENTATION_TIMEOUT");
    fm_main_presentation_timer_sync_(p_owner);
    fm_main_presentation_trace_(
        FMC_PRESENTATION_GetState(&p_owner->presentation),
        "TIMEOUT");
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
    fmc_presentation_state_t state;

    if (p_owner == NULL)
    {
        return;
    }

    state = FMC_PRESENTATION_GetState(&p_owner->presentation);
    if ((state == FMC_PRESENTATION_STATE_ALL_SEGMENTS) ||
        (state == FMC_PRESENTATION_STATE_FIRMWARE_VERSION))
    {
        fm_main_presentation_timer_start_(p_owner);
        return;
    }

    fm_main_presentation_timer_cancel_();
}

static void fm_main_presentation_trace_(
    fmc_presentation_state_t state,
    const char *p_cause)
{
    const char *p_state;

    if (!FM_DEBUG_MsgIsEnabled() || (p_cause == NULL))
    {
        return;
    }

    switch (state)
    {
    case FMC_PRESENTATION_STATE_ALL_SEGMENTS:
        p_state = "ALL_SEGMENTS";
        break;

    case FMC_PRESENTATION_STATE_FIRMWARE_VERSION:
        p_state = "FIRMWARE_VERSION BOTTOM=00.01.00 ALPHA=B0";
        break;

    case FMC_PRESENTATION_STATE_TTL_RATE:
        p_state = "TTL_RATE TOP=1234.5 BOTTOM=12.3 UNIT="
                  FMC_PRESENTATION_LITERS_LEGEND "/MIN";
        break;

    case FMC_PRESENTATION_STATE_NOT_STARTED:
    default:
        p_state = "NOT_STARTED";
        break;
    }

    (void) FM_DEBUG_UartStr("FM_MAIN:PRESENTATION=");
    (void) FM_DEBUG_UartStr(p_state);
    (void) FM_DEBUG_UartStr(" CAUSE=");
    (void) FM_DEBUG_UartStr(p_cause);
    (void) FM_DEBUG_UartStr("\n");
}

static void fm_main_periodic_refresh_timer_callback_(ULONG input)
{
    fm_main_event_t event;

    (void) input;

    FM_MAIN_EVENT_MakePeriodicRefresh(&event);
    fm_main_event_publish_(&event);
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
    fmc_presentation_snapshot_t snapshot;
    fm_status_t status;
    uint16_t current_count;

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
        &p_owner->runtime);
    fm_main_require_status_ok_(status, "FM_MAIN:ACQUISITION");
    fm_main_totalization_trace_(&p_owner->runtime);

    if (FMC_PRESENTATION_GetState(&p_owner->presentation) ==
        FMC_PRESENTATION_STATE_TTL_RATE)
    {
        FMC_PRESENTATION_MakeDummySnapshot(&snapshot);
        status = FMC_PRESENTATION_Refresh(&p_owner->presentation,
                                          &snapshot);
        fm_main_require_status_ok_(status,
                                   "FM_MAIN:PRESENTATION_REFRESH");
        fm_main_presentation_trace_(
            FMC_PRESENTATION_GetState(&p_owner->presentation),
            "REFRESH");
    }

    FM_DEBUG_LedRun(FM_DEBUG_LED_OFF);
}

static void fm_main_totalization_trace_(const fmc_runtime_t *p_runtime)
{
    fmc_service_snapshot_t snapshot;
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

    FM_DEBUG_UartStr("FM_MAIN:TOTALIZATION ACM_PULSES=");
    fm_main_totalization_uint64_trace_(snapshot.model.acm.pulses);
    FM_DEBUG_UartStr(" TTL_PULSES=");
    fm_main_totalization_uint64_trace_(snapshot.model.ttl.pulses);
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
    UINT status;

    FM_BOARD_Init();
    FM_PORT_RTC_Init();
    FM_DEBUG_Init();

    fm_main_event_queue_overflow_count = 0U;
    fm_main_event_queue_send_error_count = 0U;

    status = tx_queue_create(&fm_main_event_queue,
                             (CHAR *) "FM_MAIN_EVENT",
                             FM_MAIN_EVENT_QUEUE_MESSAGE_WORDS,
                             fm_main_event_queue_storage,
                             sizeof(fm_main_event_queue_storage));
    fm_main_require_tx_success_(status, "FM_MAIN:EVENT_QUEUE_CREATE");

    if ((FM_MAIN_PERIODIC_REFRESH_TICKS == 0U) ||
        (FM_MAIN_KEY_HOLD_TICKS == 0U) ||
        (FM_MAIN_PRESENTATION_TICKS == 0U))
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
}

void FM_MAIN_Main(void)
{
    fm_main_owner_state_t owner;
    fmc_presentation_snapshot_t presentation_snapshot;
    fm_main_event_t event;
    fm_status_t project_status;
    UINT status;

    FM_MAIN_Init();
    FMC_RUNTIME_Init(&owner.runtime);
    FM_MAIN_ACQUISITION_Init(&owner.acquisition);
    if (!FM_PORT_PulseCounterStart())
    {
        FM_DEBUG_PanicMsg("FM_MAIN:PULSE_COUNTER_START");
    }

    FM_MAIN_INPUT_RECOGNIZER_Init(&owner.input_recognizer);
    owner.key_hold_start_ticks = 0U;
    owner.presentation_start_ticks = 0U;

    project_status = FM_MAIN_PRESENTATION_LCD_Init();
    fm_main_require_status_ok_(project_status,
                               "FM_MAIN:LCD_INIT");

    FMC_PRESENTATION_MakeDummySnapshot(&presentation_snapshot);
    project_status = FMC_PRESENTATION_Init(
        &owner.presentation,
        &presentation_snapshot,
        FM_MAIN_PRESENTATION_LCD_Present,
        NULL);
    fm_main_require_status_ok_(project_status,
                               "FM_MAIN:PRESENTATION_INIT");

    project_status = FMC_PRESENTATION_Start(&owner.presentation);
    fm_main_require_status_ok_(project_status,
                               "FM_MAIN:PRESENTATION_START");
    fm_main_presentation_timer_sync_(&owner);
    fm_main_presentation_trace_(
        FMC_PRESENTATION_GetState(&owner.presentation),
        "START");

    FM_BOARD_KeyboardSetCallback(fm_main_keyboard_callback_);
    FM_BOARD_KeyboardInit();

    fm_main_periodic_refresh_timer_start_();
    (void) FM_DEBUG_UartStr("FM_MAIN:READY\n");

    for (;;)
    {
        status = tx_queue_receive(&fm_main_event_queue,
                                  &event,
                                  TX_WAIT_FOREVER);
        fm_main_require_tx_success_(status, "FM_MAIN:EVENT_QUEUE_RX");

        fm_main_event_report_publish_errors_();
        fm_main_event_handle_(&owner, &event);
        FM_DEBUG_Flush();
    }
}
