/**
 * @file    fm_board.c
 * @brief   Board-level facade for common board bring-up services.
 *
 * @details
 *  - Wraps common port-layer bring-up so applications avoid HAL sequencing.
 *  - Owns the shared board baseline (GPIO → USART1 → SPI1 → PCF8553 control → DWT).
 *  - GPIO init is manual (CubeMX call generation disabled) to keep board control local.
 *  - Jumper sampling uses temporary pull-ups then returns pins to analog to minimize leakage.
 */

#include <fm_port_dwt.h>
#include <fm_port_gpio.h>
#include <fm_port_pcf8553_ctrl.h>
#include <fm_port_rcc.h>
#include <fm_port_spi1.h>
#include <fm_port_usart1.h>
#include "fm_debug.h"
#include "fm_board.h"

/* Public Bodies */
/* Shared board bring-up: debug GPIO first, then shared peripherals, with DWT last once the runtime baseline is ready. */
void FM_BOARD_Init(void)
{
    FM_PORT_RCC_Init();
    FM_PORT_GPIO_Init();
    FM_PORT_Usart1_Init();
    FM_PORT_Spi1_Init();
    FM_PORT_Pcf8553Ctrl_Init();
    FM_PORT_DWT_Init();
}

/* Debug enable sampling: gate debug messages/LEDs via jumpers with low-leakage pulls. */
bool FM_BOARD_DebugMsgEnabled(void)
{
    return FM_PORT_GPIO_IsDbgMsgEnabled();
}

bool FM_BOARD_DebugLedsEnabled(void)
{
    bool enabled = FM_PORT_GPIO_IsDbgLedEnabled();

    if (!enabled)
    {
        FM_BOARD_LedErrorOff();
        FM_BOARD_LedRunOff();
        FM_BOARD_LedSignalOff();
    }

    return FM_PORT_GPIO_IsDbgLedEnabled();
}

/* LED control wrappers */
void FM_BOARD_LedErrorOn(void)
{
    FM_PORT_GPIO_LedErrorOn();
}

void FM_BOARD_LedErrorOff(void)
{
    FM_PORT_GPIO_LedErrorOff();
}

void FM_BOARD_LedRunOn(void)
{
    FM_PORT_GPIO_LedRunOn();
}

void FM_BOARD_LedRunOff(void)
{
    FM_PORT_GPIO_LedRunOff();
}

void FM_BOARD_LedSignalOn(void)
{
    FM_PORT_GPIO_LedSignalOn();
}

void FM_BOARD_LedSignalOff(void)
{
    FM_PORT_GPIO_LedSignalOff();
}

/* Board debug UART wrapper: exposes the board debug channel, backend lives in the USART1 port module. */
bool FM_BOARD_DebugUartTransmit(const uint8_t *p_data, uint32_t len, uint32_t timeout_ms)
{
    return FM_PORT_Usart1_Transmit(p_data, len, timeout_ms);
}

/* DWT wrappers: expose cycle counter when hardware supports CYCCNT (returns false/0 otherwise). */
bool FM_BOARD_DwtInit(void)
{
    return FM_PORT_DWT_Init();
}

uint32_t FM_BOARD_DwtGetCycles(void)
{
    return FM_PORT_DWT_GetCycles();
}

void FM_BOARD_OnRtcWakeupIrq(void)
{
    /* Non-blocking ISR log: event will be flushed later by foreground code. */
    FM_DEBUG_LogConstISR("Wakeup event");
}
