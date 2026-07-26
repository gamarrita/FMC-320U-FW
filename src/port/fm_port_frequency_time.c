/**
 * @file    fm_port_frequency_time.c
 * @brief   LPTIM3 extended monotonic timestamp implementation.
 */

#include "fm_port_frequency_time.h"

#include <stdbool.h>
#include <stddef.h>

#include "lptim.h"

#define FM_PORT_FREQUENCY_TIME_TICKS_PER_SECOND   (2048ULL)
#define FM_PORT_FREQUENCY_TIME_US_PER_SECOND      (1000000ULL)

/* Private variables */
static bool started;
static bool raw_baseline_available;
static uint16_t previous_raw_count;
static uint64_t extended_ticks;

/* Private function declarations */
static uint16_t fm_port_frequency_time_read_stable_(void);
static uint64_t fm_port_frequency_time_ticks_to_us_(uint64_t ticks);

/* Public function definitions */
fm_status_t FM_PORT_FrequencyTimeStart(void)
{
    if (started)
    {
        return FM_STATUS_OK;
    }

    /*
     * CubeMX does not expose the STM32U5 RCC autonomous-access bit for LPTIM3.
     * Leave it enabled so the LSE-clocked counter continues across Stop2.
     */
    __HAL_RCC_LPTIM3_CLKAM_ENABLE();

    if (HAL_LPTIM_Counter_Start(&hlptim3) != HAL_OK)
    {
        return FM_STATUS_ESTATE;
    }

    raw_baseline_available = false;
    previous_raw_count = 0U;
    extended_ticks = 0U;
    started = true;

    return FM_STATUS_OK;
}

fm_status_t FM_PORT_FrequencyTimeRead(uint64_t *p_timestamp_us)
{
    uint16_t raw_count;
    uint16_t elapsed_ticks;

    if (p_timestamp_us == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    if (!started)
    {
        return FM_STATUS_ESTATE;
    }

    raw_count = fm_port_frequency_time_read_stable_();

    if (!raw_baseline_available)
    {
        extended_ticks = (uint64_t) raw_count;
        raw_baseline_available = true;
    }
    else
    {
        elapsed_ticks = (uint16_t) (raw_count - previous_raw_count);
        extended_ticks += (uint64_t) elapsed_ticks;
    }

    previous_raw_count = raw_count;
    *p_timestamp_us = fm_port_frequency_time_ticks_to_us_(extended_ticks);

    return FM_STATUS_OK;
}

/* Private function definitions */
static uint16_t fm_port_frequency_time_read_stable_(void)
{
    uint16_t first;
    uint16_t second;

    do
    {
        first = (uint16_t) HAL_LPTIM_ReadCounter(&hlptim3);
        second = (uint16_t) HAL_LPTIM_ReadCounter(&hlptim3);
    }
    while (first != second);

    return second;
}

static uint64_t fm_port_frequency_time_ticks_to_us_(uint64_t ticks)
{
    uint64_t whole_seconds = ticks / FM_PORT_FREQUENCY_TIME_TICKS_PER_SECOND;
    uint64_t remaining_ticks = ticks % FM_PORT_FREQUENCY_TIME_TICKS_PER_SECOND;

    return (whole_seconds * FM_PORT_FREQUENCY_TIME_US_PER_SECOND) +
           ((remaining_ticks * FM_PORT_FREQUENCY_TIME_US_PER_SECOND) /
            FM_PORT_FREQUENCY_TIME_TICKS_PER_SECOND);
}
