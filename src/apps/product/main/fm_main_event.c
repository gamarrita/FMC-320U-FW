/**
 * @file    fm_main_event.c
 * @brief   Product main app-level event helpers.
 */
#include "fm_main_event.h"

#include <stddef.h>

void FM_MAIN_EVENT_MakeKeyboard(fm_main_event_t *p_event,
                                fm_board_keyboard_key_t key,
                                fm_board_keyboard_edge_t edge)
{
    if (p_event == NULL)
    {
        return;
    }

    p_event->kind = (ULONG) FM_MAIN_EVENT_KEYBOARD;
    p_event->key = (ULONG) key;
    p_event->edge = (ULONG) edge;
    p_event->flags = 0U;
}

void FM_MAIN_EVENT_MakePeriodicRefresh(fm_main_event_t *p_event)
{
    if (p_event == NULL)
    {
        return;
    }

    p_event->kind = (ULONG) FM_MAIN_EVENT_PERIODIC_REFRESH;
    p_event->key = 0U;
    p_event->edge = 0U;
    p_event->flags = 0U;
}

void FM_MAIN_EVENT_MakeKeyHoldTimeout(fm_main_event_t *p_event)
{
    if (p_event == NULL)
    {
        return;
    }

    p_event->kind = (ULONG) FM_MAIN_EVENT_KEY_HOLD_TIMEOUT;
    p_event->key = 0U;
    p_event->edge = 0U;
    p_event->flags = 0U;
}

fm_status_t FM_MAIN_EVENT_Publish(TX_QUEUE *p_queue,
                                  const fm_main_event_t *p_event)
{
    UINT status;

    if ((p_queue == NULL) || (p_event == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    status = tx_queue_send(p_queue, (VOID *) p_event, TX_NO_WAIT);
    if (status == TX_SUCCESS)
    {
        return FM_STATUS_OK;
    }

    if (status == TX_QUEUE_FULL)
    {
        return FM_STATUS_ERANGE;
    }

    return FM_STATUS_ESTATE;
}
