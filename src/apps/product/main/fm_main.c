/**
 * @file    fm_main.c
 * @brief   Reduced FMC product runtime wiring under ThreadX.
 */
#include "fm_main.h"

#include <stdbool.h>

#include "fm_board.h"
#include "fm_board_keyboard.h"
#include "fm_debug.h"
#include "fm_main_input_adapter.h"
#include "fm_port_rtc.h"
#include "fm_port_time.h"
#include "fmc_runtime.h"
#include "fm_status.h"
#include "tx_api.h"

#define FM_MAIN_RUNTIME_STACK_SIZE_BYTES       (2048U)
#define FM_MAIN_RUNTIME_PRIORITY               (9U)
#define FM_MAIN_RUNTIME_PREEMPTION_THRESHOLD   (9U)
#define FM_MAIN_RUNTIME_TIME_SLICE             (TX_NO_TIME_SLICE)
#define FM_MAIN_KEYBOARD_QUEUE_DEPTH           (8U)
#define FM_MAIN_KEYBOARD_QUEUE_MESSAGE_WORDS   (3U)
#define FM_MAIN_KEYBOARD_EVENT_FLAG_OVERFLOW   (1UL << 0)
#define FM_MAIN_IDLE_FLUSH_MS                  (100U)

typedef struct
{
    ULONG key;
    ULONG edge;
    ULONG flags;
} fm_main_keyboard_event_t;

static TX_THREAD fm_main_runtime_thread;
static TX_QUEUE fm_main_keyboard_queue;
static ULONG fm_main_runtime_stack[
    FM_MAIN_RUNTIME_STACK_SIZE_BYTES / sizeof(ULONG)];
static ULONG fm_main_keyboard_queue_storage[
    FM_MAIN_KEYBOARD_QUEUE_DEPTH * FM_MAIN_KEYBOARD_QUEUE_MESSAGE_WORDS];
static fmc_runtime_t fm_main_runtime;

static void fm_main_runtime_thread_entry_(ULONG input);
static void fm_main_keyboard_callback_(fm_board_keyboard_key_t key,
                                       fm_board_keyboard_edge_t edge);
static void fm_main_keyboard_publish_isr_(fm_board_keyboard_key_t key,
                                          fm_board_keyboard_edge_t edge);
static void fm_main_keyboard_handle_event_(
    const fm_main_keyboard_event_t *p_keyboard_event);
static bool fm_main_keyboard_event_to_runtime_(
    const fm_main_keyboard_event_t *p_keyboard_event,
    fmc_runtime_event_t *p_runtime_event);
static void fm_main_require_tx_success_(UINT status, const char *p_msg);
static void fm_main_require_status_ok_(fm_status_t status, const char *p_msg);

static void fm_main_runtime_thread_entry_(ULONG input)
{
    fm_main_keyboard_event_t keyboard_event;
    UINT status;

    (void) input;

    FMC_RUNTIME_Init(&fm_main_runtime);

    for (;;)
    {
        status = tx_queue_receive(&fm_main_keyboard_queue,
                                  &keyboard_event,
                                  TX_WAIT_FOREVER);
        fm_main_require_tx_success_(status, "FM_MAIN:KEY_QUEUE_RX");

        fm_main_keyboard_handle_event_(&keyboard_event);
    }
}

static void fm_main_keyboard_callback_(fm_board_keyboard_key_t key,
                                       fm_board_keyboard_edge_t edge)
{
    fm_main_keyboard_publish_isr_(key, edge);
}

static void fm_main_keyboard_publish_isr_(fm_board_keyboard_key_t key,
                                          fm_board_keyboard_edge_t edge)
{
    fm_main_keyboard_event_t keyboard_event =
    {
        .key = (ULONG) key,
        .edge = (ULONG) edge,
        .flags = 0U
    };
    UINT status;

    status = tx_queue_send(&fm_main_keyboard_queue,
                           &keyboard_event,
                           TX_NO_WAIT);
    if (status == TX_SUCCESS)
    {
        return;
    }

    if (status == TX_QUEUE_FULL)
    {
        (void) FM_DEBUG_LogConstISR("FM_MAIN:KEY_QUEUE_OVERFLOW");

        keyboard_event.flags = FM_MAIN_KEYBOARD_EVENT_FLAG_OVERFLOW;
        if (tx_queue_flush(&fm_main_keyboard_queue) == TX_SUCCESS)
        {
            (void) tx_queue_send(&fm_main_keyboard_queue,
                                 &keyboard_event,
                                 TX_NO_WAIT);
        }

        return;
    }

    (void) FM_DEBUG_LogConstISR("FM_MAIN:KEY_QUEUE_SEND_ERROR");
}

static void fm_main_keyboard_handle_event_(
    const fm_main_keyboard_event_t *p_keyboard_event)
{
    fmc_runtime_event_t runtime_event;
    fm_status_t status;

    if ((p_keyboard_event->flags &
         FM_MAIN_KEYBOARD_EVENT_FLAG_OVERFLOW) != 0U)
    {
        FM_DEBUG_ReportError(FM_DEBUG_ERR_BUFFER_FULL);
    }

    if (!fm_main_keyboard_event_to_runtime_(p_keyboard_event, &runtime_event))
    {
        return;
    }

    status = FMC_RUNTIME_Dispatch(&fm_main_runtime, &runtime_event);
    fm_main_require_status_ok_(status, "FM_MAIN:RUNTIME_DISPATCH");

    (void) FM_DEBUG_UartStr("FM_MAIN:INPUT_SHORT\n");
}

static bool fm_main_keyboard_event_to_runtime_(
    const fm_main_keyboard_event_t *p_keyboard_event,
    fmc_runtime_event_t *p_runtime_event)
{
    fm_board_keyboard_key_t key;
    fm_board_keyboard_edge_t edge;

    if ((p_keyboard_event == NULL) || (p_runtime_event == NULL))
    {
        return false;
    }

    key = (fm_board_keyboard_key_t) p_keyboard_event->key;
    edge = (fm_board_keyboard_edge_t) p_keyboard_event->edge;

    if (edge != FM_BOARD_KEYBOARD_EDGE_FALLING)
    {
        return false;
    }

    return FM_MAIN_INPUT_ADAPTER_ShortEventFromBoardKey(key, p_runtime_event);
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

    status = tx_queue_create(&fm_main_keyboard_queue,
                             (CHAR *) "FM_MAIN_KEYBOARD",
                             FM_MAIN_KEYBOARD_QUEUE_MESSAGE_WORDS,
                             fm_main_keyboard_queue_storage,
                             sizeof(fm_main_keyboard_queue_storage));
    fm_main_require_tx_success_(status, "FM_MAIN:KEY_QUEUE_CREATE");

    status = tx_thread_create(&fm_main_runtime_thread,
                              (CHAR *) "FMC_RUNTIME",
                              fm_main_runtime_thread_entry_,
                              0U,
                              fm_main_runtime_stack,
                              sizeof(fm_main_runtime_stack),
                              FM_MAIN_RUNTIME_PRIORITY,
                              FM_MAIN_RUNTIME_PREEMPTION_THRESHOLD,
                              FM_MAIN_RUNTIME_TIME_SLICE,
                              TX_AUTO_START);
    fm_main_require_tx_success_(status, "FM_MAIN:RUNTIME_THREAD_CREATE");

    FM_BOARD_KeyboardSetCallback(fm_main_keyboard_callback_);
    FM_BOARD_KeyboardInit();

    (void) FM_DEBUG_UartStr("FM_MAIN:READY\n");
}

void FM_MAIN_Main(void)
{
    FM_MAIN_Init();

    for (;;)
    {
        FM_DEBUG_Flush();
        FM_PORT_TIME_SleepMs(FM_MAIN_IDLE_FLUSH_MS);
    }
}
