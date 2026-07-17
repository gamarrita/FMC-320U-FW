/**
 * @file    fm_port_gpio_exti.c
 * @brief   GPIO EXTI adapter for HAL callbacks owned by authored code.
 */

#include "fm_port_gpio_exti.h"

#include <stddef.h>

#include "gpio.h"
#include "main.h"

static volatile fm_port_gpio_exti_callback_t g_fm_port_gpio_exti_callback = NULL;

/* HAL enters here from IRQ context; keep dispatch short and non-blocking. */
static void fm_port_gpio_exti_dispatch_(uint16_t gpio_pin,
                                        fm_port_gpio_exti_edge_t edge)
{
    fm_port_gpio_exti_callback_t p_callback = g_fm_port_gpio_exti_callback;

    if (p_callback != NULL)
    {
        p_callback(gpio_pin, edge);
    }
}

void FM_PORT_GPIO_EXTI_SetCallback(fm_port_gpio_exti_callback_t p_callback)
{
    g_fm_port_gpio_exti_callback = p_callback;
}

void FM_PORT_GPIO_EXTI_Init(void)
{
    MX_GPIO_Init();
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    fm_port_gpio_exti_dispatch_(GPIO_Pin, FM_PORT_GPIO_EXTI_EDGE_RISING);
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    fm_port_gpio_exti_dispatch_(GPIO_Pin, FM_PORT_GPIO_EXTI_EDGE_FALLING);
}
