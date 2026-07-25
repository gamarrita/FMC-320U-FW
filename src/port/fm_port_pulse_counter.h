/**
 * @file    fm_port_pulse_counter.h
 * @brief   STM32 LPTIM4 adaptation for the primary pulse counter.
 *
 * CubeMX owns the immutable LPTIM4 pin, clock, edge, filter, and counter
 * configuration. This module only starts that configured counter, keeps its
 * autonomous Stop-mode clock access enabled, and returns stable raw counter
 * observations.
 */

#ifndef FM_PORT_PULSE_COUNTER_H
#define FM_PORT_PULSE_COUNTER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Enable autonomous clock access and start the configured LPTIM4 counter.
 *
 * CubeMX must have completed `MX_LPTIM4_Init()` before this call. The first
 * successful call starts continuous counting; later calls have no effect and
 * also return true.
 *
 * The STM32 asynchronous external-clock path does not count its first five
 * active input edges after startup. That bounded startup loss is accepted for
 * this instrument. The counter is not stopped or reset during normal use.
 *
 * @return true when the counter is running.
 * @return false when the HAL start operation fails.
 *
 * @warning Foreground initialization only. Not thread-safe or IRQ-safe.
 */
bool FM_PORT_PulseCounterStart(void);

/**
 * @brief Read one stable raw observation from the running 16-bit counter.
 *
 * Reads the asynchronous counter until two consecutive values are equal, as
 * required by the STM32 LPTIM guidance. The counter remains free-running and
 * is never reset by this operation.
 *
 * @return Stable raw LPTIM4 counter value.
 *
 * @warning The counter must have been started successfully. Foreground use
 *          only; no internal synchronization is provided.
 */
uint16_t FM_PORT_PulseCounterReadStable(void);

#endif /* FM_PORT_PULSE_COUNTER_H */
