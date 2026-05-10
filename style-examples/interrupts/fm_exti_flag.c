/*
 * @file    fm_exti_flag.c
 * @brief   Minimal interrupt event example.
 */

#include "fm_exti_flag.h"

/* =========================== Private Data ================================ */
static volatile bool g_exti_flag;
static volatile uint32_t g_exti_count;

/* =========================== Private Prototypes ========================== */
static void fm_exti_flag_mark_pending_(void);

/* =========================== Private Bodies ============================== */
static void fm_exti_flag_mark_pending_(void)
{
    g_exti_flag = true;

    if (g_exti_count < 0xFFFFFFFFu)
    {
        g_exti_count++;
    }
}

/* =========================== Public Bodies =============================== */
void FM_EXTI_FLAG_Init(void)
{
    g_exti_flag = false;
    g_exti_count = 0u;
}

void FM_EXTI_FLAG_OnInterrupt(void)
{
    fm_exti_flag_mark_pending_();
}

bool FM_EXTI_FLAG_GetAndClear(void)
{
    bool pending_flag;

    pending_flag = g_exti_flag;
    g_exti_flag = false;

    return pending_flag;
}

uint32_t FM_EXTI_FLAG_GetCount(void)
{
    return g_exti_count;
}

/* =========================== Interrupts ================================== */
/*
void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin)
{
    (void)gpio_pin;
    FM_EXTI_FLAG_OnInterrupt();
}
*/

/*** end of file ***/