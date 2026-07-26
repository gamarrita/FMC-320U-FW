/**
 * @file    fm_port_frequency_time.h
 * @brief   LPTIM3 monotonic timestamp source for frequency observation.
 *
 * CubeMX owns the LPTIM3 LSE clock, prescaler, period, and counter
 * configuration. This port starts that configured free-running counter,
 * extends successive 16-bit observations, and converts its 2,048 Hz ticks to
 * a monotonic `uint64_t` microsecond timestamp.
 */

#ifndef FM_PORT_FREQUENCY_TIME_H
#define FM_PORT_FREQUENCY_TIME_H

#include <stdint.h>

#include "fm_status.h"

/**
 * @brief Enable autonomous clock access and start the configured LPTIM3.
 *
 * CubeMX must have completed `MX_LPTIM3_Init()` before this call. The first
 * successful call starts continuous counting; later calls have no effect and
 * also return `FM_STATUS_OK`.
 *
 * @return `FM_STATUS_OK` when the timestamp counter is running.
 * @return `FM_STATUS_ESTATE` when the HAL counter start operation fails.
 *
 * @warning Foreground initialization only. Not thread-safe or IRQ-safe.
 */
fm_status_t FM_PORT_FrequencyTimeStart(void);

/**
 * @brief Read one extended monotonic LPTIM3 timestamp.
 *
 * Two consecutive equal raw counter reads form the stable 16-bit observation.
 * Successive raw observations are subtracted modulo 65,536 and accumulated as
 * 2,048 Hz ticks before conversion to microseconds. Conversion truncates only
 * the fractional microsecond; the physical source resolution remains one
 * 488.28125 microsecond tick.
 *
 * The caller must invoke this function strictly less than 32 seconds apart
 * while temporal continuity is required. A full or multiple raw counter wrap
 * between calls is ambiguous and cannot be detected by LPTIM3 alone.
 *
 * @param p_timestamp_us Receives the monotonic timestamp in microseconds.
 *
 * @return `FM_STATUS_OK` when a timestamp was produced.
 * @return `FM_STATUS_EINVAL` when `p_timestamp_us` is `NULL`.
 * @return `FM_STATUS_ESTATE` when the counter has not been started.
 *
 * @warning Serialized foreground use only. Not thread-safe or IRQ-safe.
 */
fm_status_t FM_PORT_FrequencyTimeRead(uint64_t *p_timestamp_us);

#endif /* FM_PORT_FREQUENCY_TIME_H */
