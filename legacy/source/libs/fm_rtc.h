/**
 * @file fm_rtc.h
 * @brief RTC helpers for date/time formatting and adjustment.
 */

#ifndef FM_RTC_H_
#define FM_RTC_H_

#include <stdint.h>

typedef enum {
    FM_RTC_SET_YEAR,
    FM_RTC_SET_MONTH,
    FM_RTC_SET_DAY,
    FM_RTC_SET_HOUR,
    FM_RTC_SET_MINUTE,
    FM_RTC_SET_SECOND,
    FM_RTC_SET_END,
} fm_rtc_set_t;

void FM_RTC_GetPpt(char *time_str, char *date_str);
void FM_RTC_Gets(char *time, char *date);
uint32_t FM_RTC_GetUnixTime(void);
void FM_RTC_Set(fm_rtc_set_t sel, uint8_t mode);
void FM_RTC_Init(void);

#endif // FM_RTC_H_

