/**
 * @file fmx.h
 * @brief FMX public API and system events.
 *
 * Exposes shared types, event identifiers, and ThreadX related hooks used by
 * the FLOWMEET application layer.
 * @details Summarizes operational risks, REQ-FMX traceability, and ThreadX/HAL agreements for the module.
 */
#ifndef FMX_H_
#define FMX_H_

#ifdef __cplusplus
extern "C" {
#endif

// --- Includes ---
#include "main.h"
#include "app_threadx.h"
#include "tx_port.h"
#include "tx_api.h"

// --- Public Constants ---
// Cada evento usa un bit de fmx_events_t; admite hasta 32 eventos.
// Se mantiene el cast a ULONG para cumplir con REQ-FMX-QUEUE-001 y ThreadX.
/**
 * Event bitfield type for the FMX event queue.
 * Aligned with the ULONG width required by ThreadX.
 */
typedef ULONG fmx_events_t;

#define FMX_EVENT_EMPTY             ((fmx_events_t)0u)
#define FMX_EVENT_MENU_REFRESH      ((fmx_events_t)1u)
#define FMX_EVENT_KEY_DOWN          ((fmx_events_t)2u)
#define FMX_EVENT_KEY_UP            ((fmx_events_t)3u)
#define FMX_EVENT_KEY_ESC           ((fmx_events_t)4u)
#define FMX_EVENT_KEY_ENTER         ((fmx_events_t)5u)
#define FMX_EVENT_KEY_DOWN_LONG     ((fmx_events_t)6u)
#define FMX_EVENT_KEY_UP_LONG       ((fmx_events_t)7u)
#define FMX_EVENT_KEY_ESC_LONG      ((fmx_events_t)8u)
#define FMX_EVENT_KEY_ENTER_LONG    ((fmx_events_t)9u)
#define FMX_EVENT_KEY_EXT_1         ((fmx_events_t)10u)
#define FMX_EVENT_KEY_EXT_2         ((fmx_events_t)11u)
// Fuerza un wake-up cuando ThreadX alcanza el timeout maximo en stop.
#define FMX_EVENT_TIME_OUT          ((fmx_events_t)12u)
#define FMX_EVENT_END               ((fmx_events_t)13u)

// Stack sized to cover GUI and monitoring tasks.
#define FMX_STACK_SIZE              (1024u * 16u)
// Priority defined by REQ-FMX-THREAD-010.
#define FMX_THREAD_PRIORITY_10      (10u)
// Threshold preserves deterministic preemption.
#define FMX_THRESHOLD_10            (10u)
// No time slice to avoid menu jitter.
#define FMX_SLICE_0                 (0u)

// --- Public Types ---


/**
 * Low power control states.
 * Indicates when the UI can power down peripherals to save energy.
 */
typedef enum {
    FMX_LOW_POWER_DISABLE = 0u,
    FMX_LOW_POWER_ENABLE
} FMX_LowPowerControl_t;

/**
 * General status codes for FMX modules.
 * The consistent prefix simplifies failure traceability.
 */
typedef enum {
    FMX_STATUS_NULL = 0u,
    FMX_STATUS_OK,
    FMX_STATUS_ERROR,
    FMX_STATUS_BUSY,
    FMX_STATUS_TIMEOUT,
    FMX_STATUS_INVALIDA_PARAM,
    FMX_STATUS_OUT_OF_RANGE
} fmx_status_t;


/*
 *
 */
typedef enum {
	FMX_ACK_NONE = 0,
	FMX_ACK_POWER_ON,
	FMX_ACK_LOW_BATTERY,
	FMX_ACK_NEW_CONFIG,
	FMX_ACK_RATE_OFF,
	FMX_ACK_RATE_STARTED,
	FMX_ACK_RATE_ON,
	FMX_ACK_RATE_STOPED,
	FMX_ACK_RATE_CHANGE,
	FMX_ACK_TICKET,
}fmx_ack_t;





// --- Extern Variables ---
// PCB-wired flag enables the debug channel.
extern const uint32_t FMX_DEBUG_UART_1_ENABLE;
// Queue accepts deferred commands coming from ISRs.
extern TX_QUEUE fmx_deferred_cmd_queue;

// --- Public API ---
/**
 * Initializes the FMX module and its ThreadX resources.
 * @param memory_ptr Memory pool provided by the RTOS.
 * @return Status code returned by ThreadX.
 * @details Verifies each call according to REQ-FMX-INIT-001.
 */
UINT FMX_Init(VOID *memory_ptr);

/**
 * Turns on the LCD backlight and restarts its timer.
 * @details Uses ThreadX timers to control consumption.
 */
void FMX_LcdBackLightOn(void);

/**
 * Ensures a menu refresh event exists in the queue.
 * @details Maintains compliance with REQ-FMX-GUI-002.
 */
void FMX_RefreshEventTrue(void);

/**
 * Triggers the Bluetooth slave connection sequence.
 * @details Wakes the BT thread via semaphore without resorting to polling.
 */
void FMX_Trigger_BluetoothSlave(void);

#ifdef __cplusplus
}
#endif

#endif // FMX_H_
/*** END OF FILE ***/
