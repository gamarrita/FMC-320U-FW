/*
 * @file    fm_port_platform.c
 * @brief   Minimal platform adapter over Cube/HAL for the hello world flow.
 */

/* Includes */
#include "fm_port_platform.h"

#include "main.h"

/* Public Bodies */
void FM_PORT_PLATFORM_Init(void)
{
    /*
     * First iteration keeps the LED mapping in port to avoid introducing a
     * BSP layer before it provides clear value.
     */
    FM_PORT_PLATFORM_SetStatusLed(false);
}

uint32_t FM_PORT_PLATFORM_GetTickMs(void)
{
    return HAL_GetTick();
}

void FM_PORT_PLATFORM_SetStatusLed(bool led_on)
{
    HAL_GPIO_WritePin(LED_ERROR_GPIO_Port,
                      LED_ERROR_Pin,
                      led_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/*** end of file ***/
