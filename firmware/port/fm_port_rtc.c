/**
 * @file    fm_port_rtc.c
 * @brief   Platform RTC wakeup IRQ forwarding helpers.
 */

#include "fm_port_rtc.h"

#include "fm_board.h"
#include "main.h"

void FM_PORT_RTC_OnWakeupIrq(void)
{
    FM_BOARD_OnRtcWakeupIrq();
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    UNUSED(hrtc);
    FM_PORT_RTC_OnWakeupIrq();
}
