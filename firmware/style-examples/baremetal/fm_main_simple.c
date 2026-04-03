/*
 * @file    fm_main_simple.c
 * @brief   Implementación mínima de aplicación baremetal.
 */

#include "fm_main_simple.h"

/* =========================== Private Defines ============================= */
#define FM_MAIN_SIMPLE_FALSE   (0u)
#define FM_MAIN_SIMPLE_TRUE    (1u)

/* =========================== Private Types =============================== */
typedef struct
{
    uint32_t uptime_ms;
    uint32_t heartbeat_ms;
    bool     app_ready;
} fm_main_simple_state_t;

/* =========================== Private Data ================================ */
static fm_main_simple_state_t g_main_state;

/* =========================== Private Prototypes ========================== */
static void app_init_services_(void);
static void app_process_(void);
static void app_service_heartbeat_(void);
static void app_idle_delay_ms_(uint32_t delay_ms);

/* =========================== Private Bodies ============================== */
static void app_init_services_(void)
{
    g_main_state.uptime_ms = 0u;
    g_main_state.heartbeat_ms = 0u;
    g_main_state.app_ready = true;
}

static void app_process_(void)
{
    g_main_state.uptime_ms += FM_MAIN_SIMPLE_LOOP_PERIOD_MS;
    g_main_state.heartbeat_ms += FM_MAIN_SIMPLE_LOOP_PERIOD_MS;
}

static void app_service_heartbeat_(void)
{
    if (g_main_state.heartbeat_ms >= FM_MAIN_SIMPLE_HEARTBEAT_PERIOD_MS)
    {
        g_main_state.heartbeat_ms = 0u;
        /* Punto de hook para watchdog, log o toggle de diagnóstico. */
    }
}

static void app_idle_delay_ms_(uint32_t delay_ms)
{
    (void)delay_ms;
    /* En baremetal real: sleep, wait-for-interrupt o delay apropiado. */
}

/* =========================== Public Bodies =============================== */
void FM_MAIN_SIMPLE_Init(void)
{
    app_init_services_();
}

void FM_MAIN_SIMPLE_Main(void)
{
    for (;;)
    {
        app_process_();
        app_service_heartbeat_();
        app_idle_delay_ms_(FM_MAIN_SIMPLE_LOOP_PERIOD_MS);
    }
}

/*** end of file ***/