/**
 * @file    fm_board_keyboard.c
 * @brief   Board keyboard mapping and EXTI bridge.
 */

#include "fm_board_keyboard.h"

#include <stddef.h>

#include "fm_debug.h"
#include "fm_port_gpio_exti.h"
#include "main.h"

typedef struct
{
    uint16_t gpio_pin;
    fm_board_keyboard_key_t key;
    const char *p_falling_msg;
    const char *p_rising_msg;
} fm_board_keyboard_map_t;

static void fm_board_keyboard_exti_callback_(uint16_t gpio_pin,
                                             fm_port_gpio_exti_edge_t edge);
static void fm_board_keyboard_on_exti_(uint16_t gpio_pin,
                                       fm_port_gpio_exti_edge_t edge);

static const fm_board_keyboard_map_t g_fm_board_keyboard_map[] =
{
    {
        KEY_DOWN_Pin,
        FM_BOARD_KEYBOARD_KEY_DOWN,
        "KEY_INPUT_BRINGUP:KEY=DOWN EDGE=FALLING",
        "KEY_INPUT_BRINGUP:KEY=DOWN EDGE=RISING"
    },
    {
        KEY_UP_Pin,
        FM_BOARD_KEYBOARD_KEY_UP,
        "KEY_INPUT_BRINGUP:KEY=UP EDGE=FALLING",
        "KEY_INPUT_BRINGUP:KEY=UP EDGE=RISING"
    },
    {
        KEY_ENTER_Pin,
        FM_BOARD_KEYBOARD_KEY_ENTER,
        "KEY_INPUT_BRINGUP:KEY=ENTER EDGE=FALLING",
        "KEY_INPUT_BRINGUP:KEY=ENTER EDGE=RISING"
    },
    {
        KEY_ESC_Pin,
        FM_BOARD_KEYBOARD_KEY_ESC,
        "KEY_INPUT_BRINGUP:KEY=ESC EDGE=FALLING",
        "KEY_INPUT_BRINGUP:KEY=ESC EDGE=RISING"
    }
};

void FM_BOARD_KeyboardInit(void)
{
    FM_PORT_GPIO_EXTI_SetCallback(fm_board_keyboard_exti_callback_);
    FM_PORT_GPIO_EXTI_Init();
}

bool FM_BOARD_KeyboardKeyFromGpioPin(uint16_t gpio_pin,
                                     fm_board_keyboard_key_t *p_key)
{
    uint8_t index;

    if (p_key == NULL)
    {
        return false;
    }

    for (index = 0U;
         index < (uint8_t) (sizeof(g_fm_board_keyboard_map) /
                            sizeof(g_fm_board_keyboard_map[0]));
         index++)
    {
        if (g_fm_board_keyboard_map[index].gpio_pin == gpio_pin)
        {
            *p_key = g_fm_board_keyboard_map[index].key;
            return true;
        }
    }

    return false;
}

/* IRQ path: translate the board pin and defer UART formatting to fm_debug. */
static void fm_board_keyboard_on_exti_(uint16_t gpio_pin,
                                       fm_port_gpio_exti_edge_t edge)
{
    uint8_t index;

    for (index = 0U;
         index < (uint8_t) (sizeof(g_fm_board_keyboard_map) /
                            sizeof(g_fm_board_keyboard_map[0]));
         index++)
    {
        if (g_fm_board_keyboard_map[index].gpio_pin == gpio_pin)
        {
            if (edge == FM_PORT_GPIO_EXTI_EDGE_FALLING)
            {
                (void) FM_DEBUG_LogConstISR(
                    g_fm_board_keyboard_map[index].p_falling_msg);
            }
            else
            {
                (void) FM_DEBUG_LogConstISR(
                    g_fm_board_keyboard_map[index].p_rising_msg);
            }

            return;
        }
    }

    (void) FM_DEBUG_Log2ISR((uint16_t) FM_DEBUG_EVT_USER,
                            (int32_t) gpio_pin,
                            (int32_t) edge);
}

static void fm_board_keyboard_exti_callback_(uint16_t gpio_pin,
                                             fm_port_gpio_exti_edge_t edge)
{
    fm_board_keyboard_on_exti_(gpio_pin, edge);
}
