/**
 * @file    fm_debug.c
 * @brief   Debug services: ISR-safe event capture and foreground flush.
 *
 * Model:
 * - ISR / time-critical code enqueues structured events into a fixed ring.
 * - Foreground formats and transmits events over UART.
 * - Drop-on-full keeps ISR execution bounded.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fm_debug.h"
#include "fm_board.h"

/* Private Defines */
#define MSG_BUFFER_LENGTH        (96U)
#define UART_TIMEOUT_MS          (10U)
#define FM_DEBUG_EVT_CAPACITY    (8U)
#define FM_DEBUG_EVT_MASK        (FM_DEBUG_EVT_CAPACITY - 1U)
#define FM_DEBUG_FLUSH_TEXT_MAX  (128U)

#define FM_DEBUG_FLAG_HAS_PARAM1   (1U << 0)
#define FM_DEBUG_FLAG_CONST_TEXT   (1U << 1)

/* Private Types */

/* Single-producer (ISR), single-consumer (foreground) ring entry. */
typedef struct
{
    uint32_t ts_cycles;
    uint16_t code;
    uint16_t flags;
    int32_t  param0;
    int32_t  param1;
    const char *p_text; /* Must point to persistent memory when used. */
} ring_entry_t;

/* Private Data */

/* Cached enable flags updated from board-level configuration. */
static volatile bool msg_enable = false;
static volatile bool leds_enable = false;

/* DWT availability for timestamping. */
static bool dwt_ready = false;

/* Temporary buffer for blocking UART helpers (foreground only). */
static char msg_buffer[MSG_BUFFER_LENGTH];

/* Event ring: ISR producer, foreground consumer. Drop on full. */
static ring_entry_t evt_ring[FM_DEBUG_EVT_CAPACITY];
static volatile uint32_t evt_head = 0U;
static volatile uint32_t evt_tail = 0U;
static volatile uint32_t evt_dropped = 0U;
static volatile uint32_t evt_high_water = 0U;

/* Temporary formatting buffer used during flush (foreground only). */
static char flush_buffer[FM_DEBUG_FLUSH_TEXT_MAX];

/* Error tracking */
static volatile uint32_t err_counts[FM_DEBUG_ERR_COUNT] = { 0U };
static volatile uint32_t err_mask = 0U;
static volatile fm_debug_error_t last_error = FM_DEBUG_ERR_NONE;
static volatile int32_t err_param[FM_DEBUG_ERR_COUNT] = { 0 };
static const char *err_str[FM_DEBUG_ERR_COUNT] =
{
    "NONE",
    "OVERRUN",
    "TIMEOUT",
    "BACKEND",
    "BUFFER_FULL"
};

/* Private Prototypes */
static uint32_t timestamp_cycles(void);
static bool enqueue_event(uint16_t code, uint16_t flags, int32_t param0, int32_t param1, const char *p_text);
static void fm_debug_uart_transmit_(const char *p_msg, uint32_t len);
static void fm_debug_init_status_banner_(void);
static void fm_debug_panic_send_(const char *p_msg);
static void fm_debug_panic_loop_(bool flush_events);

/* Private Bodies */

/* Returns 0 when DWT timestamps are not available. */
static uint32_t timestamp_cycles(void)
{
    if (!dwt_ready)
    {
        return 0U;
    }

    return FM_BOARD_DwtGetCycles();
}

/* ISR-safe enqueue. Drop on full to keep latency bounded. */
static bool enqueue_event(uint16_t code, uint16_t flags, int32_t param0, int32_t param1, const char *p_text)
{
    uint32_t head;
    uint32_t tail;
    uint32_t queued;
    ring_entry_t *p_evt;

    if (!msg_enable)
    {
        return true;
    }

    head = evt_head;
    tail = evt_tail;
    queued = head - tail;

    if (queued >= FM_DEBUG_EVT_CAPACITY)
    {
        evt_dropped++;
        return false;
    }

    p_evt = &evt_ring[head & FM_DEBUG_EVT_MASK];

    p_evt->ts_cycles = timestamp_cycles();
    p_evt->code = code;
    p_evt->flags = flags;
    p_evt->param0 = param0;
    p_evt->param1 = param1;
    p_evt->p_text = p_text;

    evt_head = head + 1U;

    if ((queued + 1U) > evt_high_water)
    {
        evt_high_water = queued + 1U;
    }

    return true;
}

/* Central message-policy gate for every debug UART transmit path. */
static void fm_debug_uart_transmit_(const char *p_msg, uint32_t len)
{
    if ((!msg_enable) || (p_msg == NULL) || (len == 0U))
    {
        return;
    }

    (void) FM_BOARD_DebugUartTransmit((const uint8_t *) p_msg,
                                      len,
                                      UART_TIMEOUT_MS);
}

static void fm_debug_init_status_banner_(void)
{
    int len;
    const char *p_msg_state;
    const char *p_led_state;

    if (!msg_enable)
    {
        return;
    }

    p_msg_state = msg_enable ? "ENABLED" : "DISABLED";
    p_led_state = leds_enable ? "ENABLED" : "DISABLED";

    len = snprintf(msg_buffer,
                   MSG_BUFFER_LENGTH,
                   "DEBUG_INIT:MSG=%s LED=%s\n",
                   p_msg_state,
                   p_led_state);

    if (len <= 0)
    {
        return;
    }

    if ((uint32_t) len >= MSG_BUFFER_LENGTH)
    {
        len = (int) (MSG_BUFFER_LENGTH - 1U);
        msg_buffer[len] = '\0';
    }

    fm_debug_uart_transmit_(msg_buffer, (uint32_t) len);
}

/* Best-effort panic message send. Silent policy remains authoritative. */
static void fm_debug_panic_send_(const char *p_msg)
{
    uint32_t len;

    if ((!msg_enable) || (p_msg == NULL))
    {
        return;
    }

    len = (uint32_t) strlen(p_msg);

    if (len > (MSG_BUFFER_LENGTH - 2U))
    {
        len = MSG_BUFFER_LENGTH - 2U;
    }

    memcpy(msg_buffer, p_msg, len);

    if ((len == 0U) || (msg_buffer[len - 1U] != '\n'))
    {
        msg_buffer[len] = '\n';
        len++;
    }

    msg_buffer[len] = '\0';

    fm_debug_uart_transmit_(msg_buffer, len);
}

/* Non-returning loop for panic paths. */
static void fm_debug_panic_loop_(bool flush_events)
{
    for (;;)
    {
        if (flush_events)
        {
            FM_DEBUG_Flush();
        }
    }
}

/* Public Bodies */

void FM_DEBUG_Init(void)
{
    dwt_ready = FM_BOARD_DwtInit();
    evt_head = 0U;
    evt_tail = 0U;
    evt_dropped = 0U;
    evt_high_water = 0U;

    FM_DEBUG_ClearErrors();
    FM_DEBUG_RefreshJumpers();
    fm_debug_init_status_banner_();
}

void FM_DEBUG_PanicMsg(const char *p_msg)
{
    fm_debug_panic_send_(p_msg);
    FM_BOARD_LedErrorOn();
    fm_debug_panic_loop_(true);
}

void FM_DEBUG_PanicFault(const char *p_msg)
{
    fm_debug_panic_send_(p_msg);
    FM_BOARD_LedErrorOn();
    fm_debug_panic_loop_(false);
}

bool FM_DEBUG_IsEnabled(void)
{
    return (msg_enable || leds_enable);
}

void FM_DEBUG_RefreshJumpers(void)
{
    bool previous_msg_enable = msg_enable;
    bool next_msg_enable = FM_BOARD_DebugMsgEnabled();

    msg_enable = next_msg_enable;
    if (previous_msg_enable && !next_msg_enable)
    {
        /*
         * Publish the disabled policy before advancing the foreground
         * consumer. The ISR producer then suppresses new events, while every
         * event published under the previous policy is discarded here.
         */
        evt_tail = evt_head;
    }

    leds_enable = FM_BOARD_DebugLedsEnabled();
}

bool FM_DEBUG_MsgIsEnabled(void)
{
    return msg_enable;
}

bool FM_DEBUG_LedsAreEnabled(void)
{
    return leds_enable;
}

void FM_DEBUG_ReportError(fm_debug_error_t err)
{
    FM_DEBUG_ReportErrorWithParam(err, 0);
}

uint32_t FM_DEBUG_ErrorCount(fm_debug_error_t err)
{
    if ((err <= FM_DEBUG_ERR_NONE) || (err >= FM_DEBUG_ERR_COUNT))
    {
        return 0U;
    }

    return err_counts[err];
}

fm_debug_error_t FM_DEBUG_LastError(void)
{
    return last_error;
}

uint32_t FM_DEBUG_ErrorMask(void)
{
    return err_mask;
}

void FM_DEBUG_ClearErrors(void)
{
    uint32_t i;

    for (i = 0U; i < (uint32_t) FM_DEBUG_ERR_COUNT; i++)
    {
        err_counts[i] = 0U;
        err_param[i] = 0;
    }

    err_mask = 0U;
    last_error = FM_DEBUG_ERR_NONE;

    FM_BOARD_LedErrorOff();
}

/* LED control is gated internally by FM_DEBUG_LedsAreEnabled(). */
void FM_DEBUG_LedError(fm_debug_led_state_t state)
{
    if (!FM_DEBUG_LedsAreEnabled())
    {
        return;
    }

    if (state == FM_DEBUG_LED_ON)
    {
        FM_BOARD_LedErrorOn();
    }
    else
    {
        FM_BOARD_LedErrorOff();
    }
}

void FM_DEBUG_LedRun(fm_debug_led_state_t state)
{
    if (!FM_DEBUG_LedsAreEnabled())
    {
        return;
    }

    if (state == FM_DEBUG_LED_ON)
    {
        FM_BOARD_LedRunOn();
    }
    else
    {
        FM_BOARD_LedRunOff();
    }
}

void FM_DEBUG_LedSignal(fm_debug_led_state_t state)
{
    if (!FM_DEBUG_LedsAreEnabled())
    {
        return;
    }

    if (state == FM_DEBUG_LED_ON)
    {
        FM_BOARD_LedSignalOn();
    }
    else
    {
        FM_BOARD_LedSignalOff();
    }
}

void FM_DEBUG_UartMsg(const char *p_msg, uint32_t len)
{
    if ((!msg_enable) || (p_msg == NULL) || (len == 0U))
    {
        return;
    }

    if (len >= MSG_BUFFER_LENGTH)
    {
        len = MSG_BUFFER_LENGTH;
    }

    fm_debug_uart_transmit_(p_msg, len);
}

void FM_DEBUG_UartStr(const char *p_msg)
{
    uint32_t len;

    if ((!msg_enable) || (p_msg == NULL))
    {
        return;
    }

    len = (uint32_t) strlen(p_msg);

    if (len == 0U)
    {
        return;
    }

    FM_DEBUG_UartMsg(p_msg, len);
}

void FM_DEBUG_UartUint32(uint32_t num)
{
    int len;

    if (!msg_enable)
    {
        return;
    }

    len = snprintf(msg_buffer, MSG_BUFFER_LENGTH, "%lu\n", (unsigned long) num);

    if (len > 0)
    {
        FM_DEBUG_UartMsg(msg_buffer, (uint32_t) len);
    }
}

void FM_DEBUG_ReportErrorWithParam(fm_debug_error_t err, int32_t param)
{
    int len;

    if ((err <= FM_DEBUG_ERR_NONE) || (err >= FM_DEBUG_ERR_COUNT))
    {
        return;
    }

    err_counts[err]++;
    last_error = err;
    err_mask |= (1UL << (uint32_t) err);
    err_param[err] = param;

    if (msg_enable)
    {
        len = snprintf(flush_buffer,
                       FM_DEBUG_FLUSH_TEXT_MAX,
                       "[%lu] ERROR=%s CODE=%u PARAM=%ld\n",
                       (unsigned long) timestamp_cycles(),
                       err_str[err],
                       (unsigned int) err,
                       (long) param);
        if (len > 0)
        {
            if ((uint32_t) len > FM_DEBUG_FLUSH_TEXT_MAX)
            {
                len = (int) FM_DEBUG_FLUSH_TEXT_MAX;
            }

            fm_debug_uart_transmit_(flush_buffer, (uint32_t) len);
        }
    }

    FM_DEBUG_LedError(FM_DEBUG_LED_ON);
}

int32_t FM_DEBUG_ErrorParam(fm_debug_error_t err)
{
    if ((err <= FM_DEBUG_ERR_NONE) || (err >= FM_DEBUG_ERR_COUNT))
    {
        return 0;
    }

    return err_param[err];
}

const char *FM_DEBUG_ErrorString(fm_debug_error_t err)
{
    if ((err < FM_DEBUG_ERR_NONE) || (err >= FM_DEBUG_ERR_COUNT))
    {
        return "UNKNOWN";
    }

    return err_str[err];
}

void FM_DEBUG_UartInt32(int32_t num)
{
    int len;

    if (!msg_enable)
    {
        return;
    }

    len = snprintf(msg_buffer, MSG_BUFFER_LENGTH, "%ld\n", (long) num);

    if (len > 0)
    {
        FM_DEBUG_UartMsg(msg_buffer, (uint32_t) len);
    }
}

void FM_DEBUG_UartFloat(float num)
{
    int len;

    if (!msg_enable)
    {
        return;
    }

    len = snprintf(msg_buffer, MSG_BUFFER_LENGTH, "%0.2f\n", (double) num);

    if (len > 0)
    {
        FM_DEBUG_UartMsg(msg_buffer, (uint32_t) len);
    }
}

uint32_t FM_DEBUG_TimestampCycles(void)
{
    return timestamp_cycles();
}

bool FM_DEBUG_LogISR(uint16_t code, int32_t param0)
{
    return enqueue_event(code, 0U, param0, 0, NULL);
}

bool FM_DEBUG_Log2ISR(uint16_t code, int32_t param0, int32_t param1)
{
    return enqueue_event(code, FM_DEBUG_FLAG_HAS_PARAM1, param0, param1, NULL);
}

bool FM_DEBUG_LogConstISR(const char *p_msg)
{
    if (p_msg == NULL)
    {
        return false;
    }

    return enqueue_event((uint16_t) FM_DEBUG_EVT_TEXT, FM_DEBUG_FLAG_CONST_TEXT, 0, 0, p_msg);
}

uint32_t FM_DEBUG_DroppedCount(void)
{
    return evt_dropped;
}

uint32_t FM_DEBUG_QueuedCount(void)
{
    return (evt_head - evt_tail);
}

uint32_t FM_DEBUG_HighWatermark(void)
{
    return evt_high_water;
}

void FM_DEBUG_Flush(void)
{
    if (!msg_enable)
    {
        return;
    }

    while (evt_tail != evt_head)
    {
        ring_entry_t evt = evt_ring[evt_tail & FM_DEBUG_EVT_MASK];
        evt_tail++;

        if ((evt.flags & FM_DEBUG_FLAG_CONST_TEXT) != 0U)
        {
            const char *p_text = (evt.p_text != NULL) ? evt.p_text : "";
            int len = snprintf(flush_buffer, FM_DEBUG_FLUSH_TEXT_MAX,
                    "[%lu] TXT=%s\n",
                    (unsigned long) evt.ts_cycles,
                    p_text);

            if (len > 0)
            {
                if ((uint32_t) len > FM_DEBUG_FLUSH_TEXT_MAX)
                {
                    len = (int) FM_DEBUG_FLUSH_TEXT_MAX;
                }

                fm_debug_uart_transmit_(flush_buffer, (uint32_t) len);
            }
            continue;
        }

        int len;

        if ((evt.flags & FM_DEBUG_FLAG_HAS_PARAM1) != 0U)
        {
            len = snprintf(flush_buffer, FM_DEBUG_FLUSH_TEXT_MAX,
                    "[%lu] EVT=%u P0=%ld P1=%ld\n",
                    (unsigned long) evt.ts_cycles,
                    (unsigned int) evt.code,
                    (long) evt.param0,
                    (long) evt.param1);
        }
        else
        {
            len = snprintf(flush_buffer, FM_DEBUG_FLUSH_TEXT_MAX,
                    "[%lu] EVT=%u P0=%ld\n",
                    (unsigned long) evt.ts_cycles,
                    (unsigned int) evt.code,
                    (long) evt.param0);
        }

        if (len > 0)
        {
            if ((uint32_t) len > FM_DEBUG_FLUSH_TEXT_MAX)
            {
                len = (int) FM_DEBUG_FLUSH_TEXT_MAX;
            }

            fm_debug_uart_transmit_(flush_buffer, (uint32_t) len);
        }
    }
}
