/**
 * @file    fm_port_time.c
 * @brief   Platform millisecond sleep helpers.
 */

#include <fm_port_time.h>
#include "main.h"

void FM_PORT_TIME_SleepMs(uint32_t time_ms)
{
    HAL_Delay(time_ms);
}
