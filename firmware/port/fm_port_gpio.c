/**
 * @file    fm_port_gpio.c
 * @brief   Platform GPIO helpers for LEDs and debug jumpers.
 *
 * @details
 *  - This module is part of port because it performs the MCU-specific GPIO
 *    setup and HAL access required by the current target.
 *  - Unlike a pure generic GPIO adapter, it also carries some board-level
 *    signal naming for LEDs and debug jumpers.
 *  - That deviation is intentional for now: these signals map directly to
 *    fixed board wiring, and preserving those names keeps the relationship
 *    between the board and the MCU pins easier to follow.
 */

#include <fm_port_gpio.h>
#include "main.h"

/* Private Prototypes */
static void fm_port_gpio_enable_clock_(GPIO_TypeDef *port);
static bool fm_port_gpio_read_jumper_(GPIO_TypeDef *port, uint16_t pin);

/* Private Bodies */
static void fm_port_gpio_enable_clock_(GPIO_TypeDef *port)
{
    if (port == GPIOA)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else if (port == GPIOB)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else if (port == GPIOC)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else if (port == GPIOD)
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
    else if (port == GPIOE)
    {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
    else if (port == GPIOF)
    {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    }
    else if (port == GPIOG)
    {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    }
    else if (port == GPIOH)
    {
        __HAL_RCC_GPIOH_CLK_ENABLE();
    }
}

/* Temporarily enable pull-up to sample a jumper, then return the pin to analog to keep leakage low. */
static bool fm_port_gpio_read_jumper_(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    bool enabled;

    fm_port_gpio_enable_clock_(port);

    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &GPIO_InitStruct);

    enabled = (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET);

    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(port, &GPIO_InitStruct);

    return enabled;
}

/* Public Bodies */

/* Manual GPIO bring-up: LEDs as push-pull outputs low, jumpers left analog until sampled. */
void FM_PORT_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    fm_port_gpio_enable_clock_(LED_ERROR_GPIO_Port);
    fm_port_gpio_enable_clock_(LED_ACTIVE_GPIO_Port);
    fm_port_gpio_enable_clock_(LED_SIGNAL_GPIO_Port);
    fm_port_gpio_enable_clock_(DBG_MSG_EN_GPIO_Port);
    fm_port_gpio_enable_clock_(DBG_LED_EN_GPIO_Port);

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = LED_ERROR_Pin;
    HAL_GPIO_Init(LED_ERROR_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = LED_ACTIVE_Pin;
    HAL_GPIO_Init(LED_ACTIVE_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_ACTIVE_GPIO_Port, LED_ACTIVE_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = LED_SIGNAL_Pin;
    HAL_GPIO_Init(LED_SIGNAL_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_SIGNAL_GPIO_Port, LED_SIGNAL_Pin, GPIO_PIN_RESET);

    /* Leave jumpers in lowest-power state when not sampling. */
    GPIO_InitStruct.Pin = DBG_MSG_EN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DBG_MSG_EN_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DBG_LED_EN_Pin;
    HAL_GPIO_Init(DBG_LED_EN_GPIO_Port, &GPIO_InitStruct);
}

/* Sample LED enable jumper (true when floating/high). */
bool FM_PORT_GPIO_IsDbgLedEnabled(void)
{
    return fm_port_gpio_read_jumper_(DBG_LED_EN_GPIO_Port, DBG_LED_EN_Pin);
}

/* Sample UART message enable jumper (true when floating/high). */
bool FM_PORT_GPIO_IsDbgMsgEnabled(void)
{
    return fm_port_gpio_read_jumper_(DBG_MSG_EN_GPIO_Port, DBG_MSG_EN_Pin);
}

void FM_PORT_GPIO_LedErrorOn(void)
{
    HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_SET);
}

void FM_PORT_GPIO_LedErrorOff(void)
{
    HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_RESET);
}

void FM_PORT_GPIO_LedRunOn(void)
{
    HAL_GPIO_WritePin(LED_ACTIVE_GPIO_Port, LED_ACTIVE_Pin, GPIO_PIN_SET);
}

void FM_PORT_GPIO_LedRunOff(void)
{
    HAL_GPIO_WritePin(LED_ACTIVE_GPIO_Port, LED_ACTIVE_Pin, GPIO_PIN_RESET);
}

void FM_PORT_GPIO_LedSignalOn(void)
{
    HAL_GPIO_WritePin(LED_SIGNAL_GPIO_Port, LED_SIGNAL_Pin, GPIO_PIN_SET);
}

void FM_PORT_GPIO_LedSignalOff(void)
{
    HAL_GPIO_WritePin(LED_SIGNAL_GPIO_Port, LED_SIGNAL_Pin, GPIO_PIN_RESET);
}
