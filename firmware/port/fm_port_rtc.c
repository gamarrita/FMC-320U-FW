/**
 * @file    fm_port_rtc.c
 * @brief   Platform RTC wakeup IRQ forwarding helpers.
 */

#include "fm_port_rtc.h"

#include "fm_board.h"
#include "main.h"

static RTC_HandleTypeDef hrtc;

static RTC_HandleTypeDef *fm_port_rtc_handle_get_(void)
{
    return &hrtc;
}

void FM_PORT_RTC_Init(void)
{
    RTC_HandleTypeDef *rtc_handle = fm_port_rtc_handle_get_();
    RTC_PrivilegeStateTypeDef privilege_state = { 0 };

    rtc_handle->Instance = RTC;
    rtc_handle->Init.HourFormat = RTC_HOURFORMAT_24;
    rtc_handle->Init.AsynchPrediv = 127;
    rtc_handle->Init.SynchPrediv = 255;
    rtc_handle->Init.OutPut = RTC_OUTPUT_DISABLE;
    rtc_handle->Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
    rtc_handle->Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    rtc_handle->Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    rtc_handle->Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
    rtc_handle->Init.BinMode = RTC_BINARY_NONE;

    if (HAL_RTC_Init(rtc_handle) != HAL_OK)
    {
        Error_Handler();
    }

    privilege_state.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
    privilege_state.backupRegisterPrivZone = RTC_PRIVILEGE_BKUP_ZONE_NONE;
    privilege_state.backupRegisterStartZone2 = RTC_BKP_DR0;
    privilege_state.backupRegisterStartZone3 = RTC_BKP_DR0;

    if (HAL_RTCEx_PrivilegeModeSet(rtc_handle, &privilege_state) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_RTCEx_SetWakeUpTimer(rtc_handle, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
    {
        Error_Handler();
    }
}

void FM_PORT_RTC_OnWakeupIrq(void)
{
    FM_BOARD_OnRtcWakeupIrq();
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    UNUSED(hrtc);
    FM_PORT_RTC_OnWakeupIrq();
}
