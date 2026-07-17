/**
 * @file    fm_port_time.h
 * @brief   Platform millisecond sleep helpers.
 *
 * This module currently wraps the HAL millisecond delay path. It is intended
 * for foreground code and bring-ups; do not call it from IRQ context.
 */

#ifndef FM_PORT_TIME_H
#define FM_PORT_TIME_H

#include <stdint.h>

/**
 * @brief   Block for the requested number of milliseconds.
 *
 * @param   time_ms  Delay interval in milliseconds.
 */
void FM_PORT_TIME_SleepMs(uint32_t time_ms);

#endif /* FM_PORT_TIME_H */
