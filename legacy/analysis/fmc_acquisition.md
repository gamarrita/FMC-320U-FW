# FMC Legacy Acquisition Analysis

## Purpose

This document reconstructs how the legacy firmware acquired the primary sensor
signal and turned it into pulse totals and RATE input.

It is historical and analytical evidence. It does not define current product
requirements, select a peripheral, or approve a low-power technique.

The current design starts from the best documented use of the target hardware
as if no silicon defect existed. The suspected STM32U575 LPTIM Stop 2 behavior
and the legacy workaround remain risks to characterize later; neither is a
premise of the new architecture.

## Sources Reviewed

- `legacy/100_main.ioc`
- `legacy/source/FLOWMEET/fm_init.c`
- `legacy/source/FLOWMEET/fmx.c`
- `legacy/source/FLOWMEET/fmx_lp.c`
- `legacy/source/libs/fm_fmc.c`
- `legacy/source/libs/fm_fmc.h`
- `legacy/source/libs/fm_factory.c`
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`
- current public boundaries in `src/product/fmc/fmc_runtime.h`,
  `src/product/fmc/fmc_service.h`, and `src/product/fmc/fmc_rate.h`

The historical Word document was not reopened. Its frozen structured extraction
was sufficient for this analysis.

## Evidence Classification

This analysis uses the following states:

- **Legacy fact:** directly observable in the preserved legacy source or
  historical CubeMX configuration.
- **External technical fact:** stated by current ST documentation, public
  errata, or an identifiable ST response.
- **Field observation:** behavior reproduced historically but not established
  as a documented device guarantee or erratum.
- **Inference:** an explanation consistent with available evidence but not yet
  verified on the current target.
- **Hypothesis:** a falsifiable possible cause that requires a focused
  experiment.
- **Open product decision:** behavior or acceptance bounds that only the
  reviewed current product documentation may authorize.

Detail does not promote any item from evidence into a current product
requirement.

## Responsibility Split

The legacy flow must not be treated as one acquisition feature. It contains
separate responsibilities:

| Responsibility | Input | Output | Legacy owner |
|---|---|---|---|
| Pulse accumulation | Electrical sensor edges | Wrapping hardware count | LPTIM4 |
| Counter observation | LPTIM4 count snapshots | Pulse delta | `fmx.c::PulseUpdate()` |
| Edge-time observation | Sensor edge plus 32.768 kHz time base | Captured tick | LPTIM3 capture |
| RATE window formation | Pulse and tick snapshots | Coherent `delta_p`, `delta_t` | `fmx.c::PulseUpdate()` and capture callback |
| Totalization | Accepted pulse delta | ACM and TTL backing pulses | `fm_fmc.c::FM_FMC_PulseAdd()` |
| Volume calculation | Backing pulses plus calibration/unit | ACM and TTL volume | `FM_FMC_AcmCalc()`, `FM_FMC_TtlCalc()` |
| RATE mathematics | Pulse/time window plus measurement configuration | Flow rate | `FM_FMC_RateCalc()` |
| Product/runtime integration | Updated totals and rate | Logging and presentation refresh | `fmx.c` |

This split supports short implementation and validation cycles. In particular:

- accumulating pulses does not require calculating frequency;
- observing frequency does not own accumulated product totals;
- rate mathematics does not capture signals or read clocks;
- presentation does not own any acquisition window.

## Configured Legacy Hardware

The historical CubeMX configuration records:

- `PD13` as `LPTIM4_IN1`;
- LPTIM4 as a falling-edge external counter;
- `PD14` as `LPTIM3_CH1`;
- LPTIM3 clocked from LSE at 32.768 kHz;
- LPTIM1 as the ThreadX low-power timing source;
- autonomous-mode clock enablement for LPTIM1, LPTIM3, and LPTIM4.

At startup, `fm_init.c`:

1. starts LPTIM1;
2. starts LPTIM3 channel 1 in input-capture interrupt mode;
3. starts LPTIM4 as a counter;
4. enables the three autonomous clocks.

These are historical facts, not approval to reproduce the pin or peripheral
selection in the current `.ioc`.

## Legacy Pulse Accumulation

`PulseUpdate()` runs from the main legacy loop and limits its measurement work
to approximately one execution per second using the ThreadX time base.

For totalization it:

1. reads `LPTIM4->CNT` directly;
2. subtracts the previous 16-bit sample from the new sample;
3. relies on unsigned wraparound for one counter rollover;
4. adds the resulting delta to both ACM and TTL;
5. recalculates their cached volume views.

The resulting product intent is partly represented by the current accepted
contract: ACM and TTL receive the same accepted raw pulse delta. The physical
meaning and acceptance guarantees of that delta remain unresolved.

### Accumulation uncertainties

- The running LPTIM counter is read once. ST documentation warns that a running
  LPTIM counter may require repeated reads until two consecutive values agree:
  <https://dev.st.com/stm32cube-docs/stm32u5-hal2/2.0.0-beta.1.1/docs/drivers/hal_drivers/lptim/api/hal_lptim_exported_functions.html>.
- A 16-bit subtraction handles one rollover modulo 65536 but cannot detect an
  unknown number of multiple rollovers between observations.
- The actual observation interval is scheduler-dependent, despite the
  approximate one-second guard.
- Startup behavior does not explicitly establish whether pulses arriving
  around counter start are accepted, discarded, or unknowable.
- No explicit hardware/software overflow, lost-observation, or resynchronizing
  state is published to the product layer.
- The frozen legacy extraction states that repeatable loss of even one pulse is
  unacceptable, but that statement has not yet been promoted into a bounded
  current-product requirement.

## Legacy Frequency Observation

LPTIM3 runs from the 32.768 kHz LSE and captures a timestamp on the primary
sensor input. The capture callback:

1. disables further channel-1 capture interrupts;
2. reads `LPTIM3->CCR1`;
3. reads `LPTIM4->CNT` on the same sensor event;
4. publishes the pair as valid.

On the next eligible `PulseUpdate()`, the main context:

1. consumes the pair inside a short interrupt-disabled critical section;
2. computes a pulse delta between successive captured counter samples;
3. computes a 16-bit tick delta between successive timestamps;
4. re-enables the capture interrupt for a future sensor event.

The interrupt is therefore normally limited to one accepted capture per
software window rather than one interrupt for every incoming pulse.

The useful inherited idea is the coherent observation boundary: pulse count and
time are sampled on the same physical edge. The current design should preserve
that property only if the selected frequency technique needs it.

### Frequency uncertainties

- The valid measurable interval is bounded by a 16-bit 32.768 kHz timestamp,
  which wraps every two seconds.
- A factory-data comment describes a nominal lower limit of 0.1 Hz while its
  stored fixed-point value is `250`; the intended limit is therefore unclear.
  Even the commented 0.1 Hz would imply about ten seconds between pulses and
  conflict with an unextended 16-bit timestamp.
- If no new capture is consumed during the software cycle, the legacy code
  clears RATE. This conflates no observed edge, zero flow, stale observation,
  and invalid observation.
- Capture overrun and loss are not surfaced through a quality contract.
- The relationship between the ThreadX one-second cycle and the actual
  edge-to-edge RATE window is implicit.

## Legacy RATE Mathematics

The legacy calculation is equivalent to:

```text
rate =
    delta_p
    / (delta_ticks - 1)
    * (32768 / pulses_per_active_volume_unit)
    * seconds_per_active_time_unit
```

The result is then scaled by 1000 and stored in a legacy fixed-point type.

Observed issues:

- `delta_t` is documented in the type as seconds with millisecond resolution,
  but it actually holds LPTIM ticks.
- The subtraction of one tick is not justified by a documented hardware or
  mathematical contract.
- The calculation mixes acquisition-clock knowledge, calibration, unit
  conversion, fixed-point storage, and rate policy.
- Numeric truncation, precision, and valid input ranges are not explicit.

The current `fmc_rate` boundary already improves this separation: it accepts an
already observed pulse delta and a positive elapsed time in seconds, and it
does not read hardware or own synchronization.

## Low-Power Coupling

The legacy system uses LPTIM1 to support ThreadX tickless Stop 2 operation.
LPTIM3 capture can wake the CPU on the first rearmed sensor edge, while LPTIM4
continues accumulating external pulses.

This creates a coupling between:

- RTOS sleep lifecycle;
- pulse-counter observation;
- edge timestamp capture;
- APB/register visibility after Stop 2;
- RATE availability.

That coupling must not be inherited by default. The current acquisition design
should first use the documented hardware model correctly, then validate its
behavior and power consumption on the target.

## Suspected STM32U575 LPTIM Behavior

A historical field observation reported that, during Stop 2, LPTIM capture
appeared to update `CCR1` for an odd number of input edges but not for an even
number. The case was reproduced on more than one board and escalated by ST as
internal ticket `197340`:

- <https://community.st.com/stm32-mcus-products-25/nucleo-u575-capture-fail-is-it-a-silicon-level-bug-140945>

Related ST guidance says autonomous input capture in Stop mode uses the capture
DMA request to transfer the captured register through APB to SRAM:

- <https://community.st.com/stm32-mcus-products-25/stm32u-lptim-autonomous-mode-140771>
- <https://www.st.com/resource/en/application_note/dm00290631-low-power-timer-lptim-applicative-use-cases-on-stm32-microcontrollers-stmicroelectronics.pdf>

The public errata reviewed during this analysis does not identify the reported
odd/even behavior as a confirmed device limitation:

- <https://www.st.com/resource/en/errata_sheet/es0499-stm32u575xx-and-stm32u585xx-device-errata-stmicroelectronics.pdf>

This public-source status was last reviewed on 2026-07-24.

Current classification:

- reproducible historical observation;
- possible unsupported or incomplete autonomous-capture usage;
- possible silicon limitation;
- not a confirmed current-product constraint.

### Hypothesis register

| ID | Classification | Possible explanation | Existing support | Discriminating validation |
|---|---|---|---|---|
| H1 | Hypothesis | The pulse counter is correct, but a single asynchronous `CNT` read is stale or incoherent after Stop 2 | Legacy uses one direct read; ST requires stable consecutive reads for a running LPTIM | Compare direct read with repeated reads until two consecutive values agree, using an independent pulse count |
| H2 | Hypothesis | Autonomous capture without DMA does not complete the APB-visible update expected by the legacy design | ST documents `CCxDE` and DMA-to-SRAM for autonomous Stop capture | Exercise the documented DMA path first and compare CCR/SRAM state, wakeups, and current |
| H3 | Hypothesis | A silicon defect causes the reported odd/even CCR behavior even with documented configuration | Historical reproduction on several boards; no matching public erratum found | Reproduce on identified `DEV_ID`/`REV_ID` devices using a minimal documented configuration and controlled edge counts |
| H4 | Inference | The legacy capture interrupt hides H1, H2, or H3 by waking the CPU and causing APB activity | Legacy ISR reads `CCR1` and `CNT` on a sensor edge; the symptom reportedly disappeared in that arrangement | Test only after a documented technique fails; compare correctness and current with and without the added wake |

H1 concerns pulse-counter observation. H2 and H3 concern capture behavior. They
must not be treated as one failure or tested through one combined application.

The legacy capture-interrupt technique may avoid the symptom by waking the CPU
and producing APB activity. It may also merely hide a register-synchronization
or usage problem. Its correctness and energy cost are unproven.

DMA is likewise neither selected nor rejected. A high request rate does not by
itself establish the total energy cost, and an architecture that transfers
every edge may be unnecessary. Any DMA candidate must be justified against the
real pulse envelope, required information, buffer behavior, Stop-mode support,
and measured current.

## Review Closure And Future Use

The Phase 7-0 legacy review is complete enough to begin the
hardware-configuration-independent pulse-accumulation contract. No additional
broad legacy excavation is required before Phase 7A.

Later acquisition slices should return to this evidence only for a bounded
question:

- pulse accumulation and counter reads use the accumulation findings and H1;
- frequency observation uses the capture findings and H2/H3;
- the suspected workaround is consulted through H4 only after a documented
  technique produces a minimal target failure.

The durable route is owned by Phase 7 of
`docs/roadmaps/fmc_refactoring.md`. No conclusion in this analysis selects the
legacy interrupt technique, DMA, periodic polling, or a specific LPTIM
instance.
