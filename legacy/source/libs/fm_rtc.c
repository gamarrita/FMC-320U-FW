/**
 * @file fm_rtc.c
 * @brief Date/time utilities built on top of the embedded RTC.
 */

#include "fm_debug.h"
#include "fm_rtc.h"
#include <time.h>

extern RTC_HandleTypeDef hrtc;

/**
 * Formats the current time/date for the LCD (hh.mm.ss / dd.mm.yy).
 * @param time_str Buffer with at least 9 bytes for the time string.
 * @param date_str Buffer with at least 11 bytes for the date string.
 */
void FM_RTC_Gets(char *time_str, char *date_str)
{
    RTC_TimeTypeDef time = {0};
    RTC_DateTypeDef date = {0};

    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

    sprintf(time_str, "%02d.%02d.%02d ", time.Hours, time.Minutes, time.Seconds);
    sprintf(date_str, "%02d.%02d.20%02d ", date.Date, date.Month, date.Year);
}

/**
 * Formats the current time/date for printing (hh:mm:ss / dd/mm/yyyy).
 * @param time_str Buffer with at least 9 bytes for the time string.
 * @param date_str Buffer with at least 13 bytes for the date string.
 */
void FM_RTC_GetPpt(char *time_str, char *date_str)
{
    RTC_TimeTypeDef time = {0};
    RTC_DateTypeDef date = {0};

    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);

    sprintf(time_str, "%02d:%02d:%02d ", time.Hours, time.Minutes, time.Seconds);
    sprintf(date_str, "%02d/%02d/20%02d ", date.Date, date.Month, date.Year);
}

/**
 * Returns the current UNIX timestamp (seconds since 1 Jan 1970).
 */
uint32_t FM_RTC_GetUnixTime(void)
{
    RTC_TimeTypeDef time = {0};
    RTC_DateTypeDef date = {0};
    struct tm t;

    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

    t.tm_year = (int)(2000 + date.Year) - 1900;
    t.tm_mon  = (int)date.Month - 1;
    t.tm_mday = date.Date;
    t.tm_hour = time.Hours;
    t.tm_min  = time.Minutes;
    t.tm_sec  = time.Seconds;
    t.tm_isdst = 0;

    return (uint32_t)mktime(&t);
}

/**
 * Adjusts one field of the RTC date/time structure.
 * @param sel Field to modify.
 * @param mode 0 decrements, 1 increments.
 */
void FM_RTC_Set(fm_rtc_set_t sel, uint8_t mode)
{
    RTC_TimeTypeDef time = {0};
    RTC_DateTypeDef date = {0};

    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

    switch (sel) {
    case FM_RTC_SET_YEAR:
        date.Year = (mode && date.Year < 99u)       ? date.Year + 1u :
                    (!mode && date.Year > 0u)       ? date.Year - 1u : date.Year;
        break;
    case FM_RTC_SET_MONTH:
        date.Month = (mode && date.Month < 12u)     ? date.Month + 1u :
                     (!mode && date.Month > 1u)     ? date.Month - 1u : date.Month;
        break;
    case FM_RTC_SET_DAY:
        date.Date = (mode && date.Date < 31u)       ? date.Date + 1u :
                    (!mode && date.Date > 1u)       ? date.Date - 1u : date.Date;
        break;
    case FM_RTC_SET_HOUR:
        time.Hours = (mode && time.Hours < 23u)     ? time.Hours + 1u :
                     (!mode && time.Hours > 0u)     ? time.Hours - 1u : time.Hours;
        break;
    case FM_RTC_SET_MINUTE:
        time.Minutes = (mode && time.Minutes < 59u) ? time.Minutes + 1u :
                       (!mode && time.Minutes > 0u) ? time.Minutes - 1u : time.Minutes;
        break;
    case FM_RTC_SET_SECOND:
        time.Seconds = (mode && time.Seconds < 59u) ? time.Seconds + 1u :
                       (!mode && time.Seconds > 0u) ? time.Seconds - 1u : time.Seconds;
        break;
    default:
        FM_DEBUG_LedError(1);
        break;
    }

    if (HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN) != HAL_OK) {
        FM_DEBUG_LedError(1);
    }

    if (HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN) != HAL_OK) {
        FM_DEBUG_LedError(1);
    }
}

/**
 * Initializes the RTC with baseline test values (intended for lab use only).
 */
void FM_RTC_Init(void)
{
    RTC_TimeTypeDef time = {0};
    RTC_DateTypeDef date = {0};

    time.Hours = 0x08u;
    time.Minutes = 0x37u;
    time.Seconds = 0x00u;
    time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    time.StoreOperation = RTC_STOREOPERATION_RESET;

    if (HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BCD) != HAL_OK) {
        FM_DEBUG_LedError(1);
    }

    date.WeekDay = RTC_WEEKDAY_WEDNESDAY;
    date.Month = 0x08u;
    date.Date = 0x29u;
    date.Year = 0x07u;

    if (HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BCD) != HAL_OK) {
        FM_DEBUG_LedError(1);
    }
}

