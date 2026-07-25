/**
 * @file    fm_keyboard_input_bringup.c
 * @brief   Human-observed bring-up for keyboard GPIO EXTI short presses.
 *
 * Human validation contract:
 * - enable debug UART messages before reset
 * - open the ST-LINK VCP at 115200 8N1
 * - wait for `KEY_INPUT_BRINGUP:READY KEYS=DOWN,UP,ENTER,ESC EDGE=FALLING`
 * - press and release each listed key once
 * - expect one `KEY_INPUT_BRINGUP:KEY=<name> EDGE=FALLING` line per release
 *
 * Error communication:
 * - UART output is optional observability controlled by jumper policy
 * - disabled UART output does not change bring-up control flow
 *
 * Report failures with the UART log tail, pressed key, and red LED state.
 */

#include "fm_keyboard_input_bringup.h"

#include "fm_board.h"
#include "fm_board_keyboard.h"
#include "fm_debug.h"
#include "fm_port_time.h"

#define FM_KEYBOARD_INPUT_BRINGUP_IDLE_DELAY_MS   100U
#define FM_KEYBOARD_INPUT_BRINGUP_RUN_BLINK_MS    500U

void FM_KeyboardInputBringup_Run(void)
{
    uint32_t elapsed_ms = 0U;
    fm_debug_led_state_t run_led = FM_DEBUG_LED_OFF;

    FM_BOARD_Init();
    FM_DEBUG_Init();
    FM_BOARD_KeyboardInit();

    FM_DEBUG_UartStr("KEY_INPUT_BRINGUP:START\n");

    FM_DEBUG_UartStr(
            "KEY_INPUT_BRINGUP:READY KEYS=DOWN,UP,ENTER,ESC EDGE=FALLING\n");

    for (;;)
    {
        FM_DEBUG_Flush();

        elapsed_ms += FM_KEYBOARD_INPUT_BRINGUP_IDLE_DELAY_MS;
        if (elapsed_ms >= FM_KEYBOARD_INPUT_BRINGUP_RUN_BLINK_MS)
        {
            elapsed_ms = 0U;
            run_led = (run_led == FM_DEBUG_LED_OFF) ?
                      FM_DEBUG_LED_ON :
                      FM_DEBUG_LED_OFF;
            FM_DEBUG_LedRun(run_led);
        }

        FM_PORT_TIME_SleepMs(FM_KEYBOARD_INPUT_BRINGUP_IDLE_DELAY_MS);
    }
}
