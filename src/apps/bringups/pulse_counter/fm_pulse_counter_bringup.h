/**
 * @file    fm_pulse_counter_bringup.h
 * @brief   Human-observed LPTIM4 pulse-counter bring-up contract.
 */

#ifndef FM_PULSE_COUNTER_BRINGUP_H
#define FM_PULSE_COUNTER_BRINGUP_H

/**
 * @brief Run the LPTIM4 raw-counter bring-up across Run and ThreadX idle.
 *
 * Initializes board/debug services, starts the primary pulse counter once,
 * and, when UART messages are enabled, emits one stable raw counter observation
 * per second. The human controls and independently counts the input pulses.
 * While the app task sleeps between observations, the configured ThreadX idle
 * path enters Stop2 and later returns to Run for the next observation.
 * Message and LED enable jumpers are resampled before each observation, so
 * installing them after a silent interval provides a non-resetting liveness
 * check through resumed UART counts and Run LED activity.
 *
 * This bring-up can exercise rollover and silent current, but does not by
 * itself establish their human acceptance. It does not calculate pulse deltas,
 * update product totals, or calculate RATE.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void FM_PulseCounterBringup_Run(void);

#endif /* FM_PULSE_COUNTER_BRINGUP_H */
