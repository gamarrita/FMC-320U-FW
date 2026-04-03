/*
 * @file    fm_app_hello_world.c
 * @brief   Minimal hello world application flow.
 */

/* Includes */
#include "fm_app_hello_world.h"

#include <stdbool.h>
#include <stdint.h>

#include "fm_port_platform.h"

/* Private Types */
typedef struct
{
    uint32_t next_toggle_ms;
    bool     led_on;
    bool     initialized_flag;
} fm_app_hello_world_state_t;

/* Private Data */
static fm_app_hello_world_state_t g_fm_app_hello_world_state;

/* Private Prototypes */
static bool fm_app_hello_world_deadline_reached_(uint32_t now_ms,
                                                 uint32_t deadline_ms);
static void fm_app_hello_world_schedule_next_toggle_(uint32_t now_ms);
static void fm_app_hello_world_toggle_led_(void);

/* Private Bodies */
static bool fm_app_hello_world_deadline_reached_(uint32_t now_ms,
                                                 uint32_t deadline_ms)
{
    return ((int32_t)(now_ms - deadline_ms) >= 0);
}

static void fm_app_hello_world_schedule_next_toggle_(uint32_t now_ms)
{
    g_fm_app_hello_world_state.next_toggle_ms =
        now_ms + FM_APP_HELLO_WORLD_TOGGLE_PERIOD_MS;
}

static void fm_app_hello_world_toggle_led_(void)
{
    g_fm_app_hello_world_state.led_on = !g_fm_app_hello_world_state.led_on;
    FM_PORT_PLATFORM_SetStatusLed(g_fm_app_hello_world_state.led_on);
}

/* Public Bodies */
void FM_APP_HELLO_WORLD_Init(void)
{
    uint32_t now_ms;

    now_ms = FM_PORT_PLATFORM_GetTickMs();

    g_fm_app_hello_world_state.led_on = false;
    g_fm_app_hello_world_state.initialized_flag = true;

    FM_PORT_PLATFORM_SetStatusLed(false);
    fm_app_hello_world_schedule_next_toggle_(now_ms);
}

void FM_APP_HELLO_WORLD_Process(void)
{
    uint32_t now_ms;

    if (!g_fm_app_hello_world_state.initialized_flag)
    {
        return;
    }

    now_ms = FM_PORT_PLATFORM_GetTickMs();

    if (!fm_app_hello_world_deadline_reached_(
            now_ms,
            g_fm_app_hello_world_state.next_toggle_ms))
    {
        return;
    }

    fm_app_hello_world_toggle_led_();
    fm_app_hello_world_schedule_next_toggle_(now_ms);
}

/*** end of file ***/
