/**
 * @file    fm_frequency_observation_bringup.h
 * @brief   Human-observed LPTIM4/LPTIM3 frequency bring-up contract.
 *
 * This bring-up pairs stable cumulative LPTIM4 pulse observations with the
 * extended LPTIM3 timestamp and feeds them to `frequency_observation`. It
 * reports every input and newly produced result through one compact debug-UART
 * record no longer than the transport's intentional 96-byte per-call limit.
 * It does not calculate product RATE, update totals, use an LED or LCD as a
 * timing marker, or require edge capture.
 *
 * Human setup:
 * - enable debug UART messages and open ST-LINK VCP at 115200 8N1;
 * - apply the primary input to `FMC_PRIMARY_PULSE`;
 * - begin at the approved `100 Hz` diagnostic point;
 * - observe the automatic baseline, early, valid, late, and recovery sequence;
 * - then exercise steady `0 Hz`, `1 Hz`, `100 Hz`, and `1 kHz` cases;
 * - keep the `1 kHz` case running at least 70 seconds.
 *
 * Compact UART keys:
 * - `S`: sample sequence;
 * - `L`: bring-up step label;
 * - `C`: raw LPTIM4 count;
 * - `T`: extended LPTIM3 timestamp as `seconds.microseconds`;
 * - `Q`: `V` valid, `U` unavailable, `S` stale, `I` invalid, `-` no result;
 * - `N`: valid-window pulse delta;
 * - `E`: valid-window elapsed microseconds.
 *
 * Step labels are `B` baseline, `E500` early sample, `V500X2` accumulated
 * valid sample, `V1A`/`V1B` nominal valid samples, `L1200` late sample, `REC`
 * recovery, and `RUN` steady observation.
 *
 * Human temporal validation at 1 kHz:
 *
 * 1. Use the stable signal generator's actual frequency `f_ref_hz` as the
 *    independent reference. Its error is much less than 0.1 percent.
 * 2. For each UART result with `QUALITY=VALID`, record `PULSES` and
 *    `ELAPSED_US`.
 * 3. Calculate the pulse-derived reference duration:
 *
 *        reference_us = PULSES * 1,000,000 / f_ref_hz
 *
 * 4. Calculate absolute difference and percentage error:
 *
 *        difference_us = abs(ELAPSED_US - reference_us)
 *        error_percent = 100 * difference_us / reference_us
 *
 * 5. Accept the internal comparison when `error_percent <= 0.5`.
 *
 * Because the samples may occur at any input phase, `reference_us` differs
 * from the exact physical window by less than one generator period. At 1 kHz
 * that phase bound is less than 1,000 us, or 0.1 percent of a nominal
 * one-second window. LPTIM3 contributes less than one 488.28125 us
 * quantization tick. Together with the generator accuracy, the 0.5-percent
 * internal comparison conservatively demonstrates the contractual
 * plus/minus-1-percent elapsed-time target without edge capture.
 *
 * At exactly 1,000 Hz, a typical `PULSES=1000`, `ELAPSED_US=1000488` result
 * gives:
 *
 *     reference_us = 1000 * 1,000,000 / 1000 = 1,000,000 us
 *     difference_us = abs(1,000,488 - 1,000,000) = 488 us
 *     error_percent = 100 * 488 / 1,000,000 = 0.0488 percent
 *
 * The 70-second 1 kHz run must also remain monotonic across at least two
 * 32-second LPTIM3 raw wraps and one 65,536-pulse LPTIM4 wrap.
 */

#ifndef FM_FREQUENCY_OBSERVATION_BRINGUP_H
#define FM_FREQUENCY_OBSERVATION_BRINGUP_H

/**
 * @brief Run the approved frequency-observation target characterization.
 *
 * Starts LPTIM3 and LPTIM4 once, executes the bounded 100 Hz quality sequence,
 * and then reports steady nominal one-second observations indefinitely. Before
 * establishing the first frequency baseline, it waits for LPTIM4's first
 * effective count so the counter's accepted five-edge startup loss cannot
 * contaminate a physical observation window.
 *
 * @warning Foreground ThreadX app context. Does not return normally.
 */
void FM_FrequencyObservationBringup_Run(void);

#endif /* FM_FREQUENCY_OBSERVATION_BRINGUP_H */
