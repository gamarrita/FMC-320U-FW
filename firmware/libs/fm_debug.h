/**
 * @file    fm_debug.h
 * @brief   Public debug API for event logging, LEDs, errors, and UART helpers.
 */
#ifndef FM_DEBUG_H
#define FM_DEBUG_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    FM_DEBUG_LED_OFF = 0,
    FM_DEBUG_LED_ON  = 1
} fm_debug_led_state_t;

typedef enum
{
    FM_DEBUG_ERR_NONE = 0,
    FM_DEBUG_ERR_OVERRUN,
    FM_DEBUG_ERR_TIMEOUT,
    FM_DEBUG_ERR_BACKEND,
    FM_DEBUG_ERR_BUFFER_FULL,
    FM_DEBUG_ERR_COUNT
} fm_debug_error_t;

typedef enum
{
    FM_DEBUG_EVT_NONE = 0,
    FM_DEBUG_EVT_ERROR,
    FM_DEBUG_EVT_MARK,
    FM_DEBUG_EVT_TEXT,
    FM_DEBUG_EVT_IRQ_LATE,
    FM_DEBUG_EVT_USER = 0x0100
} fm_debug_event_t;

typedef struct
{
    uint32_t ts_cycles;
    uint16_t code;
    uint16_t flags;
    int32_t  param0;
    int32_t  param1;
} fm_debug_entry_t;

/**
 * @brief Initialize the debug subsystem.
 *
 * @note Call once after board peripherals are ready.
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

/** @brief Refresh cached message and LED enables from board configuration. */
void FM_DEBUG_RefreshJumpers(void);

/** @brief Return true if debug messages or LEDs are enabled. */
bool FM_DEBUG_IsEnabled(void);

/** @brief Return true if UART debug messages are enabled. */
bool FM_DEBUG_MsgIsEnabled(void);

/** @brief Return true if debug LEDs are enabled. */
bool FM_DEBUG_LedsAreEnabled(void);

/** @brief Drive the error LED when LED debug output is enabled. */
void FM_DEBUG_LedError(fm_debug_led_state_t state);

/** @brief Drive the run LED when LED debug output is enabled. */
void FM_DEBUG_LedRun(fm_debug_led_state_t state);

/** @brief Drive the signal LED when LED debug output is enabled. */
void FM_DEBUG_LedSignal(fm_debug_led_state_t state);

/** @brief Record an error without an extra parameter. */
void FM_DEBUG_ReportError(fm_debug_error_t err);

/** @brief Record an error with an extra signed parameter. */
void FM_DEBUG_ReportErrorWithParam(fm_debug_error_t err, int32_t param);

/** @brief Return how many times the given error was reported. */
uint32_t FM_DEBUG_ErrorCount(fm_debug_error_t err);

/** @brief Return the most recently reported error. */
fm_debug_error_t FM_DEBUG_LastError(void);

/** @brief Return the bitmask of errors seen since the last clear. */
uint32_t FM_DEBUG_ErrorMask(void);

/** @brief Return the last parameter recorded for the given error. */
int32_t FM_DEBUG_ErrorParam(fm_debug_error_t err);

/** @brief Return a human-readable string for an error code. */
const char *FM_DEBUG_ErrorString(fm_debug_error_t err);

/** @brief Clear all error counters, parameters, mask, and the error LED. */
void FM_DEBUG_ClearErrors(void);

/** @brief Read the current timestamp in CPU cycles, or 0 if unavailable. */
uint32_t FM_DEBUG_TimestampCycles(void);

/**
 * @brief Log one parameter from ISR or other time-critical code.
 *
 * @note Event transmission is deferred until FM_DEBUG_Flush().
 */
bool FM_DEBUG_LogISR(uint16_t code, int32_t param0);

/**
 * @brief Log two parameters from ISR or other time-critical code.
 *
 * @note Event transmission is deferred until FM_DEBUG_Flush().
 */
bool FM_DEBUG_Log2ISR(uint16_t code, int32_t param0, int32_t param1);

/**
 * @brief Log a constant string pointer from ISR without formatting.
 *
 * @warning p_msg must point to persistent memory.
 */
bool FM_DEBUG_LogConstISR(const char *p_msg);

/** @brief Return how many events were dropped because the ring buffer was full. */
uint32_t FM_DEBUG_DroppedCount(void);

/** @brief Return the current number of queued events pending flush. */
uint32_t FM_DEBUG_QueuedCount(void);

/** @brief Return the peak queued depth observed since init. */
uint32_t FM_DEBUG_HighWatermark(void);

/**
 * @brief Flush queued events over UART.
 *
 * @warning Foreground only. Not IRQ-safe.
 */
void FM_DEBUG_Flush(void);

/**
 * @brief Send a raw message buffer over the debug UART.
 *
 * @warning Foreground only. Not IRQ-safe.
 */
bool FM_DEBUG_UartMsg(const char *p_msg, uint32_t len);

/**
 * @brief Send an unsigned 32-bit value over the debug UART.
 *
 * @warning Foreground only. Not IRQ-safe.
 */
bool FM_DEBUG_UartUint32(uint32_t num);

/**
 * @brief Send a signed 32-bit value over the debug UART.
 *
 * @warning Foreground only. Not IRQ-safe.
 */
bool FM_DEBUG_UartInt32(int32_t num);

/**
 * @brief Send a float over the debug UART with two decimals.
 *
 * @warning Foreground only. Not IRQ-safe.
 */
bool FM_DEBUG_UartFloat(float num);

#endif /* FM_DEBUG_H */
