/**
 * @file    fm_main_event.h
 * @brief   App-level event payload for the product main owner loop.
 *
 * These events serialize producer contexts into `FM_MAIN_Main()`. They are not
 * product-domain runtime events: ThreadX, queues, BSP keyboard edges, key-hold
 * timeouts, and the provisional periodic refresh source remain in the app
 * composition layer.
 */
#ifndef FM_MAIN_EVENT_H
#define FM_MAIN_EVENT_H

#include "fm_board_keyboard.h"
#include "fm_status.h"
#include "tx_api.h"

#define FM_MAIN_EVENT_QUEUE_MESSAGE_WORDS   (4U)

typedef enum
{
    FM_MAIN_EVENT_KEYBOARD = 0,
    FM_MAIN_EVENT_PERIODIC_REFRESH,
    FM_MAIN_EVENT_KEY_HOLD_TIMEOUT,
    FM_MAIN_EVENT_PRESENTATION_TIMEOUT,
    FM_MAIN_EVENT_COUNT
} fm_main_event_kind_t;

typedef struct
{
    ULONG kind;
    ULONG key;
    ULONG edge;
    ULONG flags;
} fm_main_event_t;

/**
 * @brief Populate an app-level keyboard event.
 *
 * @param p_event Caller-owned event destination.
 * @param key Board keyboard key identity.
 * @param edge Board keyboard edge observed for that key.
 */
void FM_MAIN_EVENT_MakeKeyboard(fm_main_event_t *p_event,
                                fm_board_keyboard_key_t key,
                                fm_board_keyboard_edge_t edge);

/**
 * @brief Populate an app-level periodic refresh event.
 *
 * Product main consumes this event to acquire and refresh live TTL/RATE.
 * Later slices may also update measurements without changing the
 * product-domain runtime contract.
 *
 * @param p_event Caller-owned event destination.
 */
void FM_MAIN_EVENT_MakePeriodicRefresh(fm_main_event_t *p_event);

/**
 * @brief Populate an app-level key-hold timeout event.
 *
 * This event is produced by the ThreadX one-shot key-hold timer and consumed
 * only by the `FM_MAIN_Main()` owner loop. It is not a product-domain event.
 *
 * @param p_event Caller-owned event destination.
 */
void FM_MAIN_EVENT_MakeKeyHoldTimeout(fm_main_event_t *p_event);

/**
 * @brief Populate an app-level presentation timeout event.
 *
 * This event is produced by the startup one-shot presentation timer and
 * consumed only by the product owner loop.
 *
 * @param p_event Caller-owned event destination.
 */
void FM_MAIN_EVENT_MakePresentationTimeout(fm_main_event_t *p_event);

/**
 * @brief Publish one app-level event to a ThreadX queue without waiting.
 *
 * @param p_queue Queue owned by the product main app.
 * @param p_event Event to copy into the queue.
 *
 * @return `FM_STATUS_OK` when the event was queued.
 * @return `FM_STATUS_EINVAL` when any pointer is `NULL`.
 * @return `FM_STATUS_ERANGE` when the queue is full.
 * @return `FM_STATUS_ESTATE` for other ThreadX queue errors.
 */
fm_status_t FM_MAIN_EVENT_Publish(TX_QUEUE *p_queue,
                                  const fm_main_event_t *p_event);

#endif /* FM_MAIN_EVENT_H */
