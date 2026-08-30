/**
 * @file    fm_port_gpio.h
 * @brief   Platform GPIO helpers for LEDs and debug jumpers.
 *
 * @details
 *  - This module lives in port because it owns MCU GPIO configuration and HAL
 *    access for the current target.
 *  - It intentionally keeps some board-facing signal names such as LEDs and
 *    debug-enable jumpers.
 *  - That is a conscious compromise: these GPIOs are tightly tied to the board
 *    wiring, and making this API more generic would add indirection without
 *    improving ownership or reuse at the current project stage.
 *
 * Call `FM_PORT_GPIO_Init()` before using the LED helpers or jumper samplers.
 * This module has no internal synchronization and is intended for foreground
 * board/service code.
 */

#ifndef FM_PORT_GPIO_H
#define FM_PORT_GPIO_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   Configure debug LEDs and jumper GPIOs.
 *
 * Enables the required GPIO port clocks, configures debug LEDs as push-pull
 * outputs, drives them off, reasserts the active-low LCD backlight off, and
 * leaves debug-enable jumper pins in analog mode until sampled.
 */
void FM_PORT_GPIO_Init(void);

/**
 * @brief   Sample the debug LED enable jumper.
 *
 * The jumper pin is temporarily configured as input with pull-up and then
 * returned to analog/no-pull to reduce leakage.
 *
 * @return  true when the jumper enables debug LEDs.
 */
bool FM_PORT_GPIO_IsDbgLedEnabled(void);

/**
 * @brief   Sample the debug message enable jumper.
 *
 * The jumper pin is temporarily configured as input with pull-up and then
 * returned to analog/no-pull to reduce leakage.
 *
 * @return  true when the jumper enables debug UART messages.
 */
bool FM_PORT_GPIO_IsDbgMsgEnabled(void);

/**
 * @brief   Turn on the error LED GPIO output.
 */
void FM_PORT_GPIO_LedErrorOn(void);

/**
 * @brief   Turn off the error LED GPIO output.
 */
void FM_PORT_GPIO_LedErrorOff(void);

/**
 * @brief   Turn on the run LED GPIO output.
 */
void FM_PORT_GPIO_LedRunOn(void);

/**
 * @brief   Turn off the run LED GPIO output.
 */
void FM_PORT_GPIO_LedRunOff(void);

/**
 * @brief   Turn on the signal LED GPIO output.
 */
void FM_PORT_GPIO_LedSignalOn(void);

/**
 * @brief   Turn off the signal LED GPIO output.
 */
void FM_PORT_GPIO_LedSignalOff(void);

/** @brief Turn on the active-low LCD backlight output. */
void FM_PORT_GPIO_BacklightOn(void);

/** @brief Turn off the active-low LCD backlight output. */
void FM_PORT_GPIO_BacklightOff(void);

#endif /* FM_PORT_GPIO_H */
