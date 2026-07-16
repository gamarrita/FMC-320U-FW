/**
 * @file    fm_debug_panic_bringup.c
 * @brief   Bring-up app for calibrating panic and fault debug paths.
 *
 * @details
 *  - This app is intentionally simple and deterministic.
 *  - One local switch selects which failure path to trigger.
 *  - The selected case should compile cleanly, then fail at runtime in a way
 *    that is easy to recognize from UART output and debugger state.
 */

#include "fm_debug_panic_bringup.h"

#include <stdint.h>

#include "main.h"
#include "stm32u5xx_it.h"
#include "fm_board.h"
#include "fm_debug.h"
#include "fm_port_time.h"

#define DEBUG_PANIC_BRINGUP_PRE_PANIC_DELAY_MS   100U

typedef enum
{
    FM_DEBUG_PANIC_BRINGUP_CASE_PANIC_MSG = 0,
    FM_DEBUG_PANIC_BRINGUP_CASE_ERROR_HANDLER,
    FM_DEBUG_PANIC_BRINGUP_CASE_HARDFAULT
} fm_debug_panic_bringup_case_t;

/*
 * Choose one scenario at a time.
 *
 * This stays as a local compile-time switch on purpose:
 * - It keeps the example easy to inspect.
 * - It avoids adding runtime UI or extra build plumbing.
 * - It gives humans and agents one known failure source per build.
 */
#define FM_DEBUG_PANIC_BRINGUP_CASE  FM_DEBUG_PANIC_BRINGUP_CASE_ERROR_HANDLER

/* Private Prototypes */
static void fm_debug_panic_bringup_run_case_(void);
static void fm_debug_panic_bringup_run_panic_msg_(void);
static void fm_debug_panic_bringup_run_error_handler_(void);
static void fm_debug_panic_bringup_run_hardfault_(void);

/* Private Bodies */
/*
 * The small dispatcher keeps the public entry point easy to read.
 * The switch also makes the selected scenario explicit in code review.
 */
static void fm_debug_panic_bringup_run_case_(void)
{
    switch (FM_DEBUG_PANIC_BRINGUP_CASE)
    {
    case FM_DEBUG_PANIC_BRINGUP_CASE_ERROR_HANDLER:
        fm_debug_panic_bringup_run_error_handler_();
        break;

    case FM_DEBUG_PANIC_BRINGUP_CASE_HARDFAULT:
        fm_debug_panic_bringup_run_hardfault_();
        break;

    case FM_DEBUG_PANIC_BRINGUP_CASE_PANIC_MSG:
    default:
        fm_debug_panic_bringup_run_panic_msg_();
        break;
    }
}

/*
 * This is the most direct project-owned path:
 * app -> FM_DEBUG_PanicMsg().
 *
 * Use this case to verify the "normal fatal error" stop point without going
 * through Error_Handler() or a CPU fault handler.
 */
static void fm_debug_panic_bringup_run_panic_msg_(void)
{
    (void) FM_DEBUG_UartStr("DEBUG_PANIC_BRINGUP: case=PANIC_MSG\n");
    FM_PORT_TIME_SleepMs(DEBUG_PANIC_BRINGUP_PRE_PANIC_DELAY_MS);
    FM_DEBUG_PanicMsg("DEBUG_PANIC_BRINGUP:PANIC_MSG");
}

/*
 * This path demonstrates how project code can still use the conventional
 * Error_Handler() entry and end up in the shared panic infrastructure.
 *
 * The UART context should let a debugger user distinguish this case from the
 * direct panic call above.
 */
static void fm_debug_panic_bringup_run_error_handler_(void)
{
    (void) FM_DEBUG_UartStr("DEBUG_PANIC_BRINGUP: case=ERROR_HANDLER\n");
    FM_PORT_TIME_SleepMs(DEBUG_PANIC_BRINGUP_PRE_PANIC_DELAY_MS);
    Error_Handler();
}

/*
 * This path demonstrates a fault-style stop.
 *
 * Calling HardFault_Handler() directly is intentional here:
 * - it keeps the example deterministic,
 * - it avoids relying on undefined behavior to provoke a real fault,
 * - and it still exercises the same fault-specific panic entry used by the
 *   interrupt file.
 */
static void fm_debug_panic_bringup_run_hardfault_(void)
{
    (void) FM_DEBUG_UartStr("DEBUG_PANIC_BRINGUP: case=HARDFAULT\n");
    FM_PORT_TIME_SleepMs(DEBUG_PANIC_BRINGUP_PRE_PANIC_DELAY_MS);
    HardFault_Handler();
}

/* Public Bodies */
/**
 * @brief Initialize the shared board baseline and trigger one bring-up case.
 *
 * @details
 *  - This app is meant as a calibration point for both humans and agents.
 *  - It sends a short bring-up header first so the UART log shows which
 *    scenario was selected before the fatal stop happens.
 *  - The selected case then transitions into one of the shared panic paths.
 */
void FM_DebugPanicBringup_Run(void)
{
    FM_BOARD_Init();
    FM_DEBUG_Init();

    /*
     * Emit one stable banner first so a human on the terminal or an agent
     * parsing UART output can immediately identify the app that was built.
     */
    (void) FM_DEBUG_UartStr("DEBUG_PANIC_BRINGUP: bringup start\n");
    fm_debug_panic_bringup_run_case_();
}
