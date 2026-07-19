/**
 * @file    fm_port_time.h
 * @brief   Platform millisecond sleep helpers.
 *
 * This module provides cooperative sleeps for ThreadX task context. It does
 * not provide a pre-kernel, ISR, HAL busy-wait, or runtime-detected fallback
 * delay path.
 */

#ifndef FM_PORT_TIME_H
#define FM_PORT_TIME_H

#include <stdint.h>

/**
 * @brief Suspend the current ThreadX task for the requested milliseconds.
 *
 * A positive millisecond delay is converted to ThreadX ticks by rounding up,
 * so it never accidentally becomes a zero-tick sleep. A zero millisecond delay
 * returns immediately.
 *
 * @warning Thread context only.
 * @warning Do not call before ThreadX has started.
 * @warning Do not call from ISR context.
 *
 * @param time_ms Delay interval in milliseconds.
 */
void FM_PORT_TIME_SleepMs(uint32_t time_ms);

#endif /* FM_PORT_TIME_H */
