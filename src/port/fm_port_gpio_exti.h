/**
 * @file    fm_port_gpio_exti.h
 * @brief   STM32 GPIO EXTI forwarding API for authored code.
 *
 * This module owns the HAL GPIO EXTI callbacks for the STM32 port and forwards
 * them through one small authored callback. Consumers must treat the callback as
 * IRQ context: bounded work only, no blocking I/O, no formatting, no allocation.
 */
#ifndef FM_PORT_GPIO_EXTI_H
#define FM_PORT_GPIO_EXTI_H

#include <stdint.h>

typedef enum
{
    FM_PORT_GPIO_EXTI_EDGE_RISING = 0,
    FM_PORT_GPIO_EXTI_EDGE_FALLING
} fm_port_gpio_exti_edge_t;

/**
 * @brief Authored GPIO EXTI callback.
 *
 * @param gpio_pin CubeMX/HAL GPIO pin mask reported by the EXTI handler.
 * @param edge     Edge detected by the HAL callback.
 *
 * @warning Runs in IRQ context.
 */
typedef void (*fm_port_gpio_exti_callback_t)(
    uint16_t gpio_pin,
    fm_port_gpio_exti_edge_t edge);

/**
 * @brief Initialize the keyboard GPIO EXTI path.
 *
 * @details Mirrors the CubeMX configuration for the six keyboard pins and
 *          their NVIC channels without re-running the global generated GPIO
 *          initializer. Unrelated board outputs therefore retain their current
 *          state.
 */
void FM_PORT_GPIO_EXTI_Init(void);

/**
 * @brief Register the board/application GPIO EXTI callback.
 *
 * @details The current adapter intentionally supports one consumer. If another
 *          EXTI client appears, evolve this module into a pin dispatcher instead
 *          of adding hardware knowledge here.
 *
 * @note Pass NULL to unregister.
 * @warning The callback runs in IRQ context.
 */
void FM_PORT_GPIO_EXTI_SetCallback(fm_port_gpio_exti_callback_t p_callback);

#endif /* FM_PORT_GPIO_EXTI_H */
