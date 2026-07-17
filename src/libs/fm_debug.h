/**
 * @file    fm_debug.h
 * @brief   Public debug API for event logging, LEDs, errors, and UART helpers.
 *
 * This module owns debug enable sampling, debug LED gating, error counters, a
 * fixed-size ISR-safe event ring, and foreground UART flushing through the
 * board debug channel.
 *
 * Call `FM_DEBUG_Init()` after board initialization and before using debug
 * services. ISR logging functions are bounded and never transmit directly.
 * UART helpers and `FM_DEBUG_Flush()` are foreground-only blocking paths.
 */
#ifndef FM_DEBUG_H
#define FM_DEBUG_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    /** LED off request. */
    FM_DEBUG_LED_OFF = 0,
    /** LED on request. */
    FM_DEBUG_LED_ON  = 1
} fm_debug_led_state_t;

typedef enum
{
    /** No error. Not counted by report/query APIs. */
    FM_DEBUG_ERR_NONE = 0,
    /** Overrun condition. */
    FM_DEBUG_ERR_OVERRUN,
    /** Timeout condition. */
    FM_DEBUG_ERR_TIMEOUT,
    /** Backend or transport failure. */
    FM_DEBUG_ERR_BACKEND,
    /** Fixed debug event buffer was full. */
    FM_DEBUG_ERR_BUFFER_FULL,
    /** Number of error codes; not a reportable error. */
    FM_DEBUG_ERR_COUNT
} fm_debug_error_t;

typedef enum
{
    /** No event. */
    FM_DEBUG_EVT_NONE = 0,
    /** Error event. */
    FM_DEBUG_EVT_ERROR,
    /** Generic marker event. */
    FM_DEBUG_EVT_MARK,
    /** Persistent text pointer event. */
    FM_DEBUG_EVT_TEXT,
    /** ISR latency marker. */
    FM_DEBUG_EVT_IRQ_LATE,
    /** First value reserved for user/application event codes. */
    FM_DEBUG_EVT_USER = 0x0100
} fm_debug_event_t;

typedef struct
{
    /** Timestamp captured in CPU cycles, or 0 when DWT is unavailable. */
    uint32_t ts_cycles;
    /** Event code from `fm_debug_event_t` or caller-owned user event range. */
    uint16_t code;
    /** Event flags reserved for debug implementation use. */
    uint16_t flags;
    /** First event parameter. */
    int32_t  param0;
    /** Second event parameter when present. */
    int32_t  param1;
} fm_debug_entry_t;

/**
 * @brief Initialize the debug subsystem.
 *
 * @note Call once after board peripherals are ready.
 * @note Emits one best-effort UART status banner describing whether message
 *       and LED debug outputs are currently enabled by jumper policy.
 */
void FM_DEBUG_Init(void);

/**
 * @brief Enter a non-returning panic loop for fatal project errors.
 *
 * @param p_msg Optional constant context string. May be NULL.
 *
 * @note Attempts a best-effort UART send and continues flushing queued events.
 */
void FM_DEBUG_PanicMsg(const char *p_msg);

/**
 * @brief Enter a non-returning panic loop from a fault-handler context.
 *
 * @param p_msg Optional constant context string. May be NULL.
 *
 * @note Keeps the fault path conservative by not flushing queued events.
 */
void FM_DEBUG_PanicFault(const char *p_msg);

/**
 * @brief Refresh cached message and LED enables from board configuration.
 *
 * Foreground use only. May sample board GPIO policy.
 */
void FM_DEBUG_RefreshJumpers(void);

/**
 * @brief Return true if debug messages or LEDs are enabled.
 *
 * @return `true` when either debug output class is enabled.
 * @return `false` otherwise.
 */
bool FM_DEBUG_IsEnabled(void);

/**
 * @brief Return true if UART debug messages are enabled.
 *
 * @return Cached message enable state from the last jumper refresh.
 */
bool FM_DEBUG_MsgIsEnabled(void);

/**
 * @brief Return true if debug LEDs are enabled.
 *
 * @return Cached LED enable state from the last jumper refresh.
 */
bool FM_DEBUG_LedsAreEnabled(void);

/**
 * @brief Drive the error LED when LED debug output is enabled.
 *
 * Has no effect while debug LEDs are disabled.
 *
 * @param state Requested LED state.
 */
void FM_DEBUG_LedError(fm_debug_led_state_t state);

/**
 * @brief Drive the run LED when LED debug output is enabled.
 *
 * Has no effect while debug LEDs are disabled.
 *
 * @param state Requested LED state.
 */
void FM_DEBUG_LedRun(fm_debug_led_state_t state);

/**
 * @brief Drive the signal LED when LED debug output is enabled.
 *
 * Has no effect while debug LEDs are disabled.
 *
 * @param state Requested LED state.
 */
void FM_DEBUG_LedSignal(fm_debug_led_state_t state);

/**
 * @brief Record an error without an extra parameter.
 *
 * Foreground use is expected. The current implementation may also update the
 * debug error LED through the board layer.
 *
 * @param err Error code to record. `FM_DEBUG_ERR_NONE` and invalid values are
 *            ignored.
 */
void FM_DEBUG_ReportError(fm_debug_error_t err);

/**
 * @brief Record an error with an extra signed parameter.
 *
 * Invalid error codes are ignored. This updates counters and queues one
 * bounded event for later flush.
 *
 * @param err Error code to record. `FM_DEBUG_ERR_NONE` and invalid values are
 *            ignored.
 * @param param Signed context value stored with the error.
 */
void FM_DEBUG_ReportErrorWithParam(fm_debug_error_t err, int32_t param);

/**
 * @brief Return how many times the given error was reported.
 *
 * @param err Error code to query.
 *
 * @return 0 for `FM_DEBUG_ERR_NONE` or invalid error codes.
 */
uint32_t FM_DEBUG_ErrorCount(fm_debug_error_t err);

/**
 * @brief Return the most recently reported error.
 *
 * @return Last valid error reported, or `FM_DEBUG_ERR_NONE` after clear/init.
 */
fm_debug_error_t FM_DEBUG_LastError(void);

/**
 * @brief Return the bitmask of errors seen since the last clear.
 *
 * Bit `1 << err` is set when that valid error has been reported.
 */
uint32_t FM_DEBUG_ErrorMask(void);

/**
 * @brief Return the last parameter recorded for the given error.
 *
 * @param err Error code to query.
 *
 * @return Last parameter for a valid error code.
 * @return 0 for `FM_DEBUG_ERR_NONE` or invalid error codes.
 */
int32_t FM_DEBUG_ErrorParam(fm_debug_error_t err);

/**
 * @brief Return a static human-readable string for an error code.
 *
 * @param err Error code to describe.
 *
 * @return Static string. Returns `"UNKNOWN"` for invalid error codes.
 */
const char *FM_DEBUG_ErrorString(fm_debug_error_t err);

/** @brief Clear all error counters, parameters, mask, and the error LED. */
void FM_DEBUG_ClearErrors(void);

/**
 * @brief Read the current debug timestamp in CPU cycles.
 *
 * @return Current DWT cycle count.
 * @return 0 when DWT timestamps are unavailable.
 */
uint32_t FM_DEBUG_TimestampCycles(void);

/**
 * @brief Log one parameter from ISR or other time-critical code.
 *
 * @param code Event code to store.
 * @param param0 First event parameter.
 *
 * @note Event transmission is deferred until FM_DEBUG_Flush().
 *
 * @return `true` when the event was queued.
 * @return `false` when the ring buffer was full.
 */
bool FM_DEBUG_LogISR(uint16_t code, int32_t param0);

/**
 * @brief Log two parameters from ISR or other time-critical code.
 *
 * @param code Event code to store.
 * @param param0 First event parameter.
 * @param param1 Second event parameter.
 *
 * @note Event transmission is deferred until FM_DEBUG_Flush().
 *
 * @return `true` when the event was queued.
 * @return `false` when the ring buffer was full.
 */
bool FM_DEBUG_Log2ISR(uint16_t code, int32_t param0, int32_t param1);

/**
 * @brief Log a constant string pointer from ISR without formatting.
 *
 * @warning p_msg must point to persistent memory.
 *
 * @return `true` when the text event was queued.
 * @return `false` when `p_msg` is `NULL` or the ring buffer was full.
 */
bool FM_DEBUG_LogConstISR(const char *p_msg);

/**
 * @brief Return how many events were dropped because the ring buffer was full.
 *
 * @return Saturating behavior is not provided; this is the raw drop counter.
 */
uint32_t FM_DEBUG_DroppedCount(void);

/**
 * @brief Return the current number of queued events pending flush.
 *
 * Intended for foreground diagnostics.
 */
uint32_t FM_DEBUG_QueuedCount(void);

/**
 * @brief Return the peak queued depth observed since init.
 *
 * @return Maximum queue depth seen in the fixed debug event ring.
 */
uint32_t FM_DEBUG_HighWatermark(void);

/**
 * @brief Flush queued events over UART.
 *
 * @warning Foreground only. Not IRQ-safe.
 * @warning Blocking UART transmit path when debug messages are enabled.
 */
void FM_DEBUG_Flush(void);

/**
 * @brief Send a raw message buffer over the debug UART.
 *
 * Sends at most the internal debug message buffer length. Returns `false`
 * without transmitting when messages are disabled.
 *
 * @param p_msg Buffer to transmit; not retained after return.
 * @param len Number of bytes to transmit.
 *
 * @return `true` when the request was accepted for transmit.
 * @return `false` when `p_msg` is `NULL`, `len` is zero, or messages are
 *         disabled.
 *
 * @warning Foreground only. Not IRQ-safe.
 */
bool FM_DEBUG_UartMsg(const char *p_msg, uint32_t len);

/**
 * @brief Send a null-terminated string over the debug UART.
 *
 * @param p_msg Null-terminated string; not retained after return.
 *
 * @return `true` when the request was accepted for transmit.
 * @return `false` when `p_msg` is `NULL`, empty, or messages are disabled.
 *
 * @warning Foreground only. Not IRQ-safe.
 */
bool FM_DEBUG_UartStr(const char *p_msg);

/**
 * @brief Send an unsigned 32-bit value over the debug UART.
 *
 * @return `true` when the request was accepted for transmit.
 * @return `false` when messages are disabled.
 *
 * @warning Foreground only. Not IRQ-safe.
 */
bool FM_DEBUG_UartUint32(uint32_t num);

/**
 * @brief Send a signed 32-bit value over the debug UART.
 *
 * @return `true` when the request was accepted for transmit.
 * @return `false` when messages are disabled.
 *
 * @warning Foreground only. Not IRQ-safe.
 */
bool FM_DEBUG_UartInt32(int32_t num);

/**
 * @brief Send a float over the debug UART with two decimals.
 *
 * @return `true` when the request was accepted for transmit.
 * @return `false` when messages are disabled.
 *
 * @warning Foreground only. Not IRQ-safe.
 */
bool FM_DEBUG_UartFloat(float num);

#endif /* FM_DEBUG_H */
