/**
 * @file    fm_port_gpio_exti.c
 * @brief   GPIO EXTI adapter for HAL callbacks owned by authored code.
 */

#include "fm_port_gpio_exti.h"

#include <stddef.h>

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
    GPIO_InitTypeDef gpio_init = { 0 };

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    gpio_init.Pin = KEY_DOWN_Pin |
                    KEY_UP_Pin |
                    KEY_ENTER_Pin |
                    KEY_ESC_Pin;
    gpio_init.Mode = GPIO_MODE_IT_RISING_FALLING;
    gpio_init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(KEY_DOWN_GPIO_Port, &gpio_init);

    gpio_init.Pin = EXT_BUTTON_1_Pin | EXT_BUTTON_2_Pin;
    gpio_init.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(EXT_BUTTON_1_GPIO_Port, &gpio_init);

    HAL_NVIC_SetPriority(EXT_BUTTON_1_EXTI_IRQn, 5U, 0U);
    HAL_NVIC_EnableIRQ(EXT_BUTTON_1_EXTI_IRQn);

    HAL_NVIC_SetPriority(EXT_BUTTON_2_EXTI_IRQn, 5U, 0U);
    HAL_NVIC_EnableIRQ(EXT_BUTTON_2_EXTI_IRQn);

    HAL_NVIC_SetPriority(KEY_DOWN_EXTI_IRQn, 5U, 0U);
    HAL_NVIC_EnableIRQ(KEY_DOWN_EXTI_IRQn);

    HAL_NVIC_SetPriority(KEY_UP_EXTI_IRQn, 5U, 0U);
    HAL_NVIC_EnableIRQ(KEY_UP_EXTI_IRQn);

    HAL_NVIC_SetPriority(KEY_ENTER_EXTI_IRQn, 5U, 0U);
    HAL_NVIC_EnableIRQ(KEY_ENTER_EXTI_IRQn);

    HAL_NVIC_SetPriority(KEY_ESC_EXTI_IRQn, 5U, 0U);
    HAL_NVIC_EnableIRQ(KEY_ESC_EXTI_IRQn);
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    fm_port_gpio_exti_dispatch_(GPIO_Pin, FM_PORT_GPIO_EXTI_EDGE_RISING);
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    fm_port_gpio_exti_dispatch_(GPIO_Pin, FM_PORT_GPIO_EXTI_EDGE_FALLING);
}
