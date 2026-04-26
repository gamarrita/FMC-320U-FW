/**
 * @file    fm_port_rtc.h
 * @brief   Platform RTC wakeup IRQ forwarding helpers.
 */

#ifndef FM_PORT_RTC_H
#define FM_PORT_RTC_H

/**
 * @brief   Handle a project-local RTC wakeup IRQ event.
 */
void FM_PORT_RTC_OnWakeupIrq(void);

/**
 * @brief   Configure the RTC peripheral for the active platform.
 */
void FM_PORT_RTC_Init(void);

#endif /* FM_PORT_RTC_H */
