/**
 * @file    fm_port_rtc.h
 * @brief   Platform RTC wakeup IRQ forwarding helpers.
 *
 * This module owns the authored RTC handle configuration and forwards the HAL
 * wakeup callback to the board layer. It is not a calendar/time API.
 */

#ifndef FM_PORT_RTC_H
#define FM_PORT_RTC_H

/**
 * @brief   Handle a project-local RTC wakeup IRQ event.
 *
 * This path is called from the HAL RTC wakeup callback and must be treated as
 * IRQ context. It forwards the event to the board layer; it does not clear or
 * format user-visible state.
 */
void FM_PORT_RTC_OnWakeupIrq(void);

/**
 * @brief   Configure the RTC peripheral for the active platform.
 *
 * Applies the current RTC configuration and wakeup timer setup. HAL failures
 * stop in `Error_Handler()`.
 */
void FM_PORT_RTC_Init(void);

#endif /* FM_PORT_RTC_H */
