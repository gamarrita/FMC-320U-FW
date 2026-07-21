/**
 * @file    fm_main.c
 * @brief   Reduced FMC product runtime wiring under ThreadX.
 */
#include "fm_main.h"

#include <stdbool.h>

#include "fm_board.h"
#include "fm_board_keyboard.h"
#include "fm_debug.h"
#include "fm_main_event.h"
#include "fm_main_input_adapter.h"
#include "fm_port_rtc.h"
#include "fmc_runtime.h"
#include "fm_status.h"
#include "tx_api.h"

#define FM_MAIN_EVENT_QUEUE_DEPTH              (8U)
#define FM_MAIN_PERIODIC_REFRESH_TICKS         ((ULONG) TX_TIMER_TICKS_PER_SECOND)

static TX_QUEUE fm_main_event_queue;
static TX_TIMER fm_main_periodic_refresh_timer;
static ULONG fm_main_event_queue_storage[
    FM_MAIN_EVENT_QUEUE_DEPTH * FM_MAIN_EVENT_QUEUE_MESSAGE_WORDS];
static volatile ULONG fm_main_event_queue_overflow_count;
static volatile ULONG fm_main_event_queue_send_error_count;

static void fm_main_keyboard_callback_(fm_board_keyboard_key_t key,
                                       fm_board_keyboard_edge_t edge);
static void fm_main_event_publish_(const fm_main_event_t *p_event);
static void fm_main_event_report_publish_errors_(void);
static void fm_main_counter_increment_(volatile ULONG *p_counter);
static void fm_main_event_handle_(fmc_runtime_t *p_runtime,
                                  const fm_main_event_t *p_event,
                                  bool *p_signal_led_on);
static void fm_main_keyboard_handle_event_(
    fmc_runtime_t *p_runtime,
    const fm_main_event_t *p_event);
static bool fm_main_keyboard_event_to_runtime_(
    const fm_main_event_t *p_event,
    fmc_runtime_event_t *p_runtime_event);
static void fm_main_periodic_refresh_timer_callback_(ULONG input);
static void fm_main_periodic_refresh_handle_(bool *p_signal_led_on);
static void fm_main_require_tx_success_(UINT status, const char *p_msg);
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

static void fm_main_event_handle_(fmc_runtime_t *p_runtime,
                                  const fm_main_event_t *p_event,
                                  bool *p_signal_led_on)
{
    if (p_event == NULL)
    {
        return;
    }

    switch ((fm_main_event_kind_t) p_event->kind)
    {
    case FM_MAIN_EVENT_KEYBOARD:
        fm_main_keyboard_handle_event_(p_runtime, p_event);
        break;

    case FM_MAIN_EVENT_PERIODIC_REFRESH:
        fm_main_periodic_refresh_handle_(p_signal_led_on);
        break;

    case FM_MAIN_EVENT_COUNT:
    default:
        FM_DEBUG_ReportError(FM_DEBUG_ERR_BACKEND);
        break;
    }
}

static void fm_main_keyboard_handle_event_(
    fmc_runtime_t *p_runtime,
    const fm_main_event_t *p_event)
{
    fmc_runtime_event_t runtime_event;
    fm_status_t status;

    if ((p_runtime == NULL) || (p_event == NULL))
    {
        return;
    }

    if (!fm_main_keyboard_event_to_runtime_(p_event, &runtime_event))
    {
        return;
    }

    status = FMC_RUNTIME_Dispatch(p_runtime, &runtime_event);
    fm_main_require_status_ok_(status, "FM_MAIN:RUNTIME_DISPATCH");

    (void) FM_DEBUG_UartStr("FM_MAIN:INPUT_SHORT\n");
}

static bool fm_main_keyboard_event_to_runtime_(
    const fm_main_event_t *p_event,
    fmc_runtime_event_t *p_runtime_event)
{
    fm_board_keyboard_key_t key;
    fm_board_keyboard_edge_t edge;

    if ((p_event == NULL) || (p_runtime_event == NULL))
    {
        return false;
    }

    if ((fm_main_event_kind_t) p_event->kind != FM_MAIN_EVENT_KEYBOARD)
    {
        return false;
    }

    key = (fm_board_keyboard_key_t) p_event->key;
    edge = (fm_board_keyboard_edge_t) p_event->edge;

    if (edge != FM_BOARD_KEYBOARD_EDGE_FALLING)
    {
        return false;
    }

    return FM_MAIN_INPUT_ADAPTER_ShortEventFromBoardKey(key, p_runtime_event);
}

static void fm_main_periodic_refresh_timer_callback_(ULONG input)
{
    fm_main_event_t event;

    (void) input;

    FM_MAIN_EVENT_MakePeriodicRefresh(&event);
    fm_main_event_publish_(&event);
}

static void fm_main_periodic_refresh_handle_(bool *p_signal_led_on)
{
    if (p_signal_led_on == NULL)
    {
        return;
    }

    if (*p_signal_led_on)
    {
        FM_BOARD_LedSignalOff();
        *p_signal_led_on = false;
    }
    else
    {
        FM_BOARD_LedSignalOn();
        *p_signal_led_on = true;
    }
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

    if (FM_MAIN_PERIODIC_REFRESH_TICKS == 0U)
    {
        FM_DEBUG_PanicMsg("FM_MAIN:REFRESH_TICKS_ZERO");
    }

    status = tx_timer_create(&fm_main_periodic_refresh_timer,
                             (CHAR *) "FM_MAIN_REFRESH",
                             fm_main_periodic_refresh_timer_callback_,
                             0U,
                             FM_MAIN_PERIODIC_REFRESH_TICKS,
                             FM_MAIN_PERIODIC_REFRESH_TICKS,
                             TX_AUTO_ACTIVATE);
    fm_main_require_tx_success_(status, "FM_MAIN:REFRESH_TIMER_CREATE");
}

void FM_MAIN_Main(void)
{
    fmc_runtime_t runtime;
    fm_main_event_t event;
    bool signal_led_on = false;
    UINT status;

    FM_MAIN_Init();
    FMC_RUNTIME_Init(&runtime);

    FM_BOARD_KeyboardSetCallback(fm_main_keyboard_callback_);
    FM_BOARD_KeyboardInit();

    (void) FM_DEBUG_UartStr("FM_MAIN:READY\n");

    for (;;)
    {
        status = tx_queue_receive(&fm_main_event_queue,
                                  &event,
                                  TX_WAIT_FOREVER);
        fm_main_require_tx_success_(status, "FM_MAIN:EVENT_QUEUE_RX");

        fm_main_event_report_publish_errors_();
        fm_main_event_handle_(&runtime, &event, &signal_led_on);
        FM_DEBUG_Flush();
    }
}
