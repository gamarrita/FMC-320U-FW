/**
 * @file    fm_port_gpio.h
 * @brief   Platform GPIO helpers for LEDs and debug jumpers.
 */

#ifndef FM_PORT_GPIO_H
#define FM_PORT_GPIO_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   Configure debug LEDs and jumper GPIOs.
 */
void FM_PORT_GPIO_Init(void);

/**
 * @brief   Sample the debug LED enable jumper.
 *
 * @return  true when the jumper enables debug LEDs.
 */
bool FM_PORT_GPIO_IsDbgLedEnabled(void);

/**
 * @brief   Sample the debug message enable jumper.
 *
 * @return  true when the jumper enables debug UART messages.
 */
bool FM_PORT_GPIO_IsDbgMsgEnabled(void);

/**
 * @brief   Turn on the error LED.
 */
void FM_PORT_GPIO_LedErrorOn(void);

/**
 * @brief   Turn off the error LED.
 */
void FM_PORT_GPIO_LedErrorOff(void);

/**
 * @brief   Turn on the run LED.
 */
void FM_PORT_GPIO_LedRunOn(void);

/**
 * @brief   Turn off the run LED.
 */
void FM_PORT_GPIO_LedRunOff(void);

/**
 * @brief   Turn on the signal LED.
 */
void FM_PORT_GPIO_LedSignalOn(void);

/**
 * @brief   Turn off the signal LED.
 */
void FM_PORT_GPIO_LedSignalOff(void);

#endif /* FM_PORT_GPIO_H */
