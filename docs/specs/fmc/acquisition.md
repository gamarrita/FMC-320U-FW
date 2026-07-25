# FMC Pulse-Accumulation Contract

## Purpose And Scope

This document owns the reviewed Phase 7A contract that converts successive
observations of one wrapping pulse counter into a raw pulse delta.

The contract preserves the legacy accumulation shape:

1. the 16-bit LPTIM4 counter accumulates primary-sensor pulses;
2. successive counter observations are subtracted modulo 16 bits;
3. the resulting raw delta is widened to `uint64_t`;
4. later runtime integration delivers that same delta exactly once to the
   canonical ACM and TTL pulse totals.

The legacy implementation is the primary behavioral guide for this bounded
contract. Current product documents and public headers remain the authorities
for accepted product behavior and implemented ownership.

This cut does not define RATE observation, exact per-second measurement
windows, presentation, or the MCU configuration that will realize the
contract.

## Legacy And Current Traceability

The reviewed legacy path is:

- `legacy/source/FLOWMEET/fmx.c::PulseUpdate()` reads `LPTIM4->CNT` into
  `uint16_t` observations and calculates `vol_pulse_delta` with unsigned
  subtraction;
- `legacy/source/libs/fm_fmc.c::FM_FMC_PulseAdd()` adds that same delta to ACM
  and TTL;
- `legacy/source/libs/fm_fmc.h::fm_fmc_totalizer_t` stores `pulse_acm` and
  `pulse_ttl` as `uint64_t`.

The current path already represents the runtime pulse-delta payload, the
`FMC_SERVICE_AddPulseDelta()` parameter, and both canonical model totals as
`uint64_t`. Phase 7A therefore preserves the legacy modulo-counter behavior
while widening its result at the current product boundary.

The legacy coupling of totalization, flow-state detection, RATE calculation,
logging, and presentation inside `PulseUpdate()` is not part of this contract.
Those responsibilities remain separated by the current reviewed module
boundaries.

## Reviewed Operating Assumptions

- The primary input frequency is at most 1 kHz.
- The LPTIM4 pulse counter is trusted to count accepted input pulses correctly.
- Every counter observation supplied to this contract is valid.
- The counter is 16 bits wide and wraps modulo 65,536.
- Observations occur nominally about once per second.
- Other system mechanisms keep the observation cadence far below one complete
  counter cycle. Their watchdog, reset, and diagnostic behavior is outside
  Phase 7A.
- The number of pulses between successive observations is therefore a
  precondition in the inclusive range `0..65,535`.
- The same counter continues accumulating in Run, throughout Stop2, and across
  the transitions between those states.

Electrical levels, input conditioning, active edge, pin, filter, clock, full
counter configuration, and CubeMX changes remain gated for Phase 7B1.

## Delta Meaning And Numeric Contract

Let:

- `previous_count` be the preceding accepted 16-bit counter observation;
- `current_count` be the new 16-bit counter observation.

The raw pulse delta is:

```text
delta16 = (current_count - previous_count) modulo 65,536
```

After the subtraction, `delta16` is widened without reinterpretation:

```text
pulse_delta = (uint64_t) delta16
```

The output range is `0..65,535`. A zero delta is valid and means that the
counter did not advance between the two observations.

The delta is not required to describe an exact one-second interval. Scheduling,
Stop2 residence, or observation timing may move pulses into a later
observation. The required invariant is cumulative conservation: the sum of
successive deltas eventually includes every pulse counted after the active
baseline, with no loss or duplication.

## Startup, Stop2, And Reset

### Startup

The LPTIM4 counter is initialized to zero before counting begins. That known
zero is the initial `previous_count`.

The first observation is processed normally. If the first observed count is
nonzero, those pulses were accumulated after the counter started and the first
delta includes all of them.

### Stop2

Entering Stop2 does not change `previous_count`, reset the LPTIM4 counter, or
close the accumulation interval. LPTIM4 continues counting in Stop2.

After return to Run, the next observation is subtracted from the last
observation taken before Stop2. Pulses may therefore be reported later than the
nominal cadence, but none may be discarded merely because they arrived during
Stop2.

### Reset

An MCU reset ends the active accumulation interval. Pulses arriving between the
reset and the point at which the LPTIM is rearmed may be discarded.

Rearming initializes LPTIM4 and `previous_count` to zero and begins a new
guaranteed accumulation interval. No attempt is made in Phase 7A to reconstruct
pulses from before that new baseline.

## Validity, Wrap, And Recovery Boundary

There is no quality, ambiguity, invalid-observation, or resynchronization state
in the accepted Phase 7A output. Every supplied observation is trusted and
produces one delta.

One modulo subtraction correctly handles a counter rollover while the true
delta remains below 65,536. A true delta of 65,536 or more is indistinguishable
from a smaller modulo delta and violates the input precondition.

Phase 7A does not define a maximum-latency monitor or recovery from that system
failure. Repository-level mechanisms are expected to keep observations near
the nominal one-second cadence; a failure severe enough to break that
assumption requires equipment reset and belongs to another cut.

## Ownership And Handoff

- The acquisition observation owner retains `previous_count`, reads the current
  counter, calculates the modulo delta, widens it to `uint64_t`, and then stores
  the current count as the next baseline.
- Phase 7A defines the delta value but does not decide whether zero or nonzero
  deltas cause runtime events. Event publication belongs to the later
  acquisition/runtime integration cut.
- `fmc_runtime` remains the RTOS-neutral product event boundary.
- `fmc_service` remains the only owner that adds an accepted delta to both ACM
  and TTL.
- ACM and TTL keep canonical `uint64_t` pulse accumulators. Their reset policies
  differ, but their pulse source does not.
- RATE observation remains independent. No timing or frequency meaning is
  inferred from this accumulation delta.

The existing service overflow rule remains unchanged: if adding a delta would
overflow either canonical `uint64_t` total, neither total is modified.

## Acceptance Examples

| Case | Previous | Current | Delta | Required interpretation |
|---|---:|---:|---:|---|
| First observation, no pulses | 0 | 0 | 0 | Valid first delta |
| First observation after pulses | 0 | 12 | 12 | All pulses since counter start are retained |
| Normal advance | 1,200 | 1,250 | 50 | Fifty counted pulses |
| No advance | 1,250 | 1,250 | 0 | Valid zero delta |
| One wrap | 65,530 | 9 | 15 | Modulo subtraction preserves the wrapped delta |
| Delayed observation | 1,000 | 4,500 | 3,500 | Pulses may move to a later observation |
| Largest valid delta | 100 | 99 | 65,535 | Valid only under the less-than-one-cycle precondition |
| Reset and rearm | 0 | 7 | 7 | New interval begins at the rearmed zero baseline |

A transition from `100` back to `100` after exactly 65,536 pulses is not an
invalid observation to classify inside this contract; it is a violation of the
less-than-one-cycle input precondition. Its detection and recovery are outside
Phase 7A.

## Out Of Scope

- electrical sensor and input-front-end behavior;
- LPTIM4 pin, clock, filter, edge, or CubeMX configuration;
- interrupts, DMA, polling, autonomous-mode implementation, or register-read
  technique;
- observation-cadence enforcement, watchdogs, and failure-triggered reset;
- event publication and runtime integration;
- RATE pulse/time windows, zero-flow recognition, and frequency quality;
- total persistence, reset authorization, presentation, and pulse indication.
