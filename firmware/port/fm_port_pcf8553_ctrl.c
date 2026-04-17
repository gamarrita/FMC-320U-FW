/**
 * @file    fm_port_pcf8553_ctrl.c
 * @brief   Platform control-line helpers for the PCF8553 LCD path.
 */

#include "fm_port_pcf8553_ctrl.h"

#include "main.h"

/* Private Prototypes */
static void fm_port_pcf8553_ctrl_enable_clock_(GPIO_TypeDef *port);
static void fm_port_pcf8553_ctrl_write_ce_(GPIO_PinState state);
static void fm_port_pcf8553_ctrl_write_reset_(GPIO_PinState state);

/* Private Bodies */
static void fm_port_pcf8553_ctrl_enable_clock_(GPIO_TypeDef *port)
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

static void fm_port_pcf8553_ctrl_write_ce_(GPIO_PinState state)
{
    HAL_GPIO_WritePin(PCF8553_CE_GPIO_Port, PCF8553_CE_Pin, state);
}

static void fm_port_pcf8553_ctrl_write_reset_(GPIO_PinState state)
{
    HAL_GPIO_WritePin(PCF8553_RESET_GPIO_Port, PCF8553_RESET_Pin, state);
}

/* Public Bodies */
void FM_PORT_Pcf8553Ctrl_Init(void)
{
    GPIO_InitTypeDef gpio_init = { 0 };

    fm_port_pcf8553_ctrl_enable_clock_(PCF8553_CE_GPIO_Port);
    fm_port_pcf8553_ctrl_enable_clock_(PCF8553_RESET_GPIO_Port);

    gpio_init.Pin = PCF8553_CE_Pin | PCF8553_RESET_Pin;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(PCF8553_CE_GPIO_Port, &gpio_init);

    FM_PORT_Pcf8553Ctrl_Disable();
    fm_port_pcf8553_ctrl_write_reset_(GPIO_PIN_SET);
}

void FM_PORT_Pcf8553Ctrl_Enable(void)
{
    fm_port_pcf8553_ctrl_write_ce_(GPIO_PIN_RESET);
}

void FM_PORT_Pcf8553Ctrl_Disable(void)
{
    fm_port_pcf8553_ctrl_write_ce_(GPIO_PIN_SET);
}

void FM_PORT_Pcf8553Ctrl_Reset(void)
{
    fm_port_pcf8553_ctrl_write_reset_(GPIO_PIN_RESET);
    HAL_Delay(5U);
    fm_port_pcf8553_ctrl_write_reset_(GPIO_PIN_SET);
}
