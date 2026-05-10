/*
 * @file    fm_task_simple.c
 * @brief   Simple periodic task example.
 */

#include "fm_task_simple.h"

/* =========================== Private Data ================================ */
static uint32_t g_task_cycle_count;

/* =========================== Private Prototypes ========================== */
static void fm_task_simple_service_(void);

/* =========================== Private Bodies ============================== */
static void fm_task_simple_service_(void)
{
    g_task_cycle_count++;
    /* Hook for periodic lightweight work. */
}

/* =========================== Public Bodies =============================== */
void FM_TASK_SIMPLE_Init(void)
{
    g_task_cycle_count = 0u;
}

void FM_TASK_SIMPLE_Entry(ULONG thread_input)
{
    (void)thread_input;

    FM_TASK_SIMPLE_Init();

    for (;;)
    {
        fm_task_simple_service_();
        tx_thread_sleep(FM_TASK_SIMPLE_PERIOD_TICKS);
    }
}

/*** end of file ***/