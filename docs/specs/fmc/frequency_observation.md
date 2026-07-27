# FMC Frequency-Observation Contract

## Purpose And Scope

This document owns the reviewed Phase 7D contract for the physical pulse/time
observation consumed later by pure RATE mathematics.

Frequency observation remains independent from the accepted LPTIM4
pulse-accumulation and ACM/TTL totalization path. A missing, stale, or invalid
frequency observation must not discard pulses, change either total, or prevent
the totalization path from advancing.

Phase 7D does not select or implement a hardware technique, time source,
CubeMX change, RTOS mechanism, or target bring-up.

## Accepted Ownership

- A dedicated technical `frequency_observation` module owns the semantic
  observation window and its previous baseline.
- The module is RTOS-neutral and hardware-agnostic. It does not read a
  peripheral, schedule a window, block, dispatch product events, calculate
  product RATE, update totals, or present a value.
- The caller owns the observer storage and lifetime. The product-main owner may
  serialize and invoke it without becoming the owner of the window semantics.
- A future port supplies physical observations without owning the window or
  its baseline.
- The frequency observer retains state independently from the accepted
  `pulse_delta` observer. The two paths may receive the same stable physical
  counter observation, but neither path depends on the other path accepting or
  producing a result.
- `fmc_runtime` remains the later RTOS-neutral product handoff boundary.
  `fmc_rate` remains pure and stateless and consumes only an already accepted
  pulse/time observation.

## Accepted Simplified-Measurement Compatibility

The contract shall support a first realization that estimates frequency from
the number of pulses observed during nominal one-second windows, using the
actual elapsed time rather than assuming that every scheduled interval is
exactly one second.

That realization may reuse stable observations from the already accepted
LPTIM4 counter path and therefore need no additional frequency-measurement
peripheral or input pin. Its frequency baseline and failure handling remain
independent from the pulse-delta baseline used for ACM/TTL.

This compatibility decision does not yet select the physical technique or time
source. Their approval remains a Phase 7E1 entry gate after the complete 7D
contract establishes the required range, accuracy, latency, cadence, numeric,
wrap, and quality behavior.

## Accepted Input Shape And First Baseline

- Each input sample contains one cumulative stable pulse-counter observation
  paired with one monotonic timestamp observation.
- The frequency observer owns the previous accepted counter and timestamp as
  its baseline.
- The first accepted sample establishes that baseline and does not produce a
  complete pulse/time window.
- Each later accepted sample forms a candidate window from the modulo counter
  difference and the actual timestamp difference. The scheduled cadence is not
  substituted for measured elapsed time.
- After forming an accepted window, the current pair becomes the baseline for
  the next consecutive window.
- The input shape does not select how or when the pair is produced. Periodic
  sampling and a future edge-coherent capture may both satisfy this semantic
  boundary if they meet this approved contract.

## Accepted Numeric Representation And Wrap

- `pulse_count` is a `uint16_t` cumulative stable counter observation in the
  range `0..65,535`.
- The pulse difference is formed modulo 65,536. The true number of pulses
  between successive accepted frequency samples must be in the inclusive
  range `0..65,535`; a complete or multiple counter cycle is ambiguous and
  violates the input precondition.
- `timestamp_us` is a `uint64_t` monotonic timestamp expressed in
  microseconds. Its epoch is arbitrary and may restart when the equipment and
  frequency observer are reset.
- Successive samples within one active observation interval must have a
  strictly increasing timestamp. An equal or lower timestamp does not form a
  valid elapsed-time window.
- Timestamp wrap is not supported during an active observation interval. The
  `uint64_t` microsecond representation makes wrap irrelevant over the
  equipment's practical operating lifetime.
- The microsecond representation does not claim one-microsecond source
  resolution or accuracy. A coarser physical source may be converted into
  `timestamp_us`; its source-specific resolution is documented in 7E1 and must
  satisfy the approved physical accuracy target below.
- Elapsed seconds for later pure RATE mathematics are derived only after
  integer timestamp subtraction:

```text
elapsed_us = current_timestamp_us - previous_timestamp_us
elapsed_seconds = elapsed_us / 1,000,000
```

## Accepted Measurement Range

- The required nonzero physical-input range for the first realization is
  `1 Hz..1 kHz`, inclusive.
- The accepted `1 kHz` upper bound is the existing primary-input product
  limit. At the nominal one-second cadence it remains far below one complete
  16-bit counter cycle per observation window.
- A temporally admissible window containing no pulses follows the accepted
  valid-zero semantics below.
- Inputs below `1 Hz` have no guaranteed representative RATE observation in
  the first realization. This limitation does not affect pulse accumulation or
  ACM/TTL totalization.
- The legacy factory-data comment suggesting `0.1 Hz` is explicitly rejected
  as a requirement for this refactor. Its stored legacy value is internally
  unclear, and supporting `0.1 Hz` would require a longer or different
  observation policy than the approved first-realization direction.

## Accepted Physical Accuracy Target

- Within each accepted physical observation window, the pulse component must
  match the independently observed physical pulses exactly, with no loss or
  duplication.
- `elapsed_us` must differ by no more than `+/-1%` from an independent
  elapsed-time reference over the approved window range.
- The technical observer adds no approximation: modulo pulse subtraction and
  integer timestamp subtraction must produce exact results from their accepted
  inputs.
- Target comparison uses the physical reference window:

```text
observed frequency = observed pulses / observed elapsed time
reference frequency = reference pulses / reference elapsed time
```

- An individual finite window is not required to reproduce the generator's
  nominal continuous-frequency label when edge phase changes the integer pulse
  count near a window boundary. The instrument and reference must instead
  agree on the pulses and elapsed time belonging to that window.
- No legacy `+/-0.1 Hz` criterion is inherited. Pure observer regressions
  verify exact transformations; the physical pulse and time targets are
  verified on target in 7E1.

## Accepted Regression And Verification Boundary

- Pure regressions shall verify exact deterministic results from the frequency
  observer's own counter/timestamp sample sequence. They do not assign a
  physical accuracy to the sample producer.
- Required pure vectors include first-sample baseline establishment, a normal
  window using actual elapsed time, zero pulses, 16-bit pulse-counter wrap,
  the largest valid modulo delta, the inclusive elapsed-time boundaries, early
  rejection without baseline advance, late rejection with resynchronization,
  equal and decreasing timestamps, reset and new baseline, invalid pointers,
  and independent observer instances.
- Integration regressions shall verify that `pulse_delta` and
  `frequency_observation` retain separate baselines and can consume different,
  interleaved observation sequences.
- A frequency sample does not modify the totalization baseline; a
  totalization sample does not modify the frequency baseline. Missing,
  rejected, or reset frequency state must not prevent subsequent pulse deltas
  from reaching ACM/TTL.
- Sharing one physical LPTIM4 observation between the two paths is not a
  contract requirement. Such fan-out is tested only if the selected
  first-realization technique actually uses it.
- Counter and timestamp coherence is required within one frequency sample. It
  does not require that sample to coincide with a totalization observation.
- Physical resolution and accuracy belong to the selected sample producer and
  target bring-up. The accepted exact-pulse and `+/-1%` elapsed-time targets
  apply in 7E1; no legacy accuracy criterion is inherited automatically.

## Accepted Cadence And Latency

- The frequency baseline is established when frequency acquisition starts and
  before periodic observation is activated.
- The first realization requests one new frequency sample at a nominal
  one-second cadence through the existing product-main periodic coordination
  event. It requires no additional product thread or application timer.
- Sharing the coordination event does not require frequency and totalization
  to use the same LPTIM4 observation or observation instant.
- Each accepted sample closes at most one consecutive window and becomes the
  next baseline.
- The observer uses the actual `timestamp_us` difference. Queueing or owner-loop
  delay is measured as part of the window rather than replacing elapsed time
  with the nominal cadence.
- A completed window is made available immediately. The first realization does
  not average, smooth, or deliberately buffer multiple windows.
- The first complete window is nominally available one second after baseline
  establishment. An input-frequency change begins to affect the next completed
  window, with nominal latency no greater than one cadence interval.
- The accepted temporal admission range and stale behavior below govern early
  and late samples.
- Before the first complete window, frequency quality is `UNAVAILABLE`.
  Exact presentation of that quality remains outside this contract.

## Accepted Activity Ownership And Deferred Dormant Mechanism

- `product/main` is the single owner that invokes the frequency observer and
  will own the future acquisition activity state.
- The target lifecycle distinguishes an inactive state without a periodic
  acquisition deadline from an active state that requests observations at the
  nominal one-second cadence and may enter Stop2 between those deadlines.
- A future activity controller waits until five seconds without newly counted
  primary pulses before permitting the inactive state. Its decision is based
  on pulse-accumulation evidence, not on a zero, unavailable, or invalid RATE
  observation.
- On a future inactive-to-active transition, the frequency observer begins a
  new baseline. The independent pulse-delta baseline is retained so that the
  activity pulse and any other accumulated pulses are not discarded.
- `frequency_observation` does not detect system activity, arm or cancel
  application timers, enter Stop2, select a wake source, or own the product
  `IDLE/ACTIVE` state.
- The mechanisms that enter the indefinite-duration sleep and detect the
  activity that leaves it are not selected.
- To preserve the current simple execution state, the first integration
  continues the existing nominal one-second periodic refresh. Implementing the
  future inactive state is not part of 7D and is not implied by this contract.

## Accepted Window Admission And Resynchronization

- A candidate window is temporally admissible when its actual elapsed time is
  in the inclusive range `900,000..1,100,000 us`.
- An admissible window retains its actual `elapsed_us`; the observer never
  substitutes the nominal one-second duration.
- A sample earlier than `900,000 us` produces no window and does not modify the
  existing baseline. Later calls continue measuring from that same baseline.
- A sample later than `1,100,000 us` produces no valid window but replaces the
  existing baseline with the current counter/timestamp pair. This
  resynchronization allows the next nominal sample to form a new window.
- When a valid observation existed previously, that late rejection makes the
  current quality `STALE`. Before any valid observation exists, the quality
  remains `UNAVAILABLE`.
- Equal or decreasing timestamps are `INVALID` and follow the accepted
  baseline-discard recovery below.

## Accepted Valid-Zero Semantics

- A temporally admissible window with `pulse_delta == 0` and positive
  `elapsed_us` is a valid observation. It is represented by `quality = VALID`
  and retains the zero pulse delta; no separate `VALID_ZERO` quality value is
  required.
- The observation means only that no pulses were counted during that complete
  window. It does not prove sensor presence, sensor absence, a persistent
  physical zero-flow condition, or a frequency below the supported range.
- The valid-zero window is made available immediately and may later produce
  numeric RATE zero through pure RATE mathematics.
- A valid-zero frequency window does not itself move acquisition from active
  to inactive. The future activity controller continues observing until its
  separate five-second condition based on newly accumulated primary pulses is
  satisfied.
- Inputs below the accepted `1 Hz` range may produce alternating valid-zero and
  nonzero windows. RATE is not guaranteed to be representative there, while
  every accumulated pulse remains eligible for ACM/TTL and resets the future
  inactivity watch.

## Accepted Unavailable Semantics

- `quality = UNAVAILABLE` means only that no complete, usable pulse/time window
  currently exists.
- It applies after observer initialization or reset, after the first baseline
  is established but before the first complete window, after future
  acquisition reactivation while a new baseline is being formed, or when the
  producer reports that no counter/timestamp pair is available.
- `UNAVAILABLE` does not mean zero pulses, zero flow, a disconnected sensor, a
  previous valid observation that has aged, or a supplied sample that violates
  the contract.
- An unavailable result has no usable pulse/time payload. Consumers must not
  infer a numeric RATE or call `fmc_rate` from it.
- A sample rejected as too early produces no new window and does not
  automatically replace an already available valid observation with
  `UNAVAILABLE`.
- If an expected call never occurs, the observer cannot detect that absence by
  itself. The product-main cadence owner is responsible for publishing the
  applicable unavailable or expiration condition.
- `UNAVAILABLE` never changes pulse totals or the independent activity watch.

## Accepted Stale Semantics

- `quality = STALE` means that a valid window existed previously but was not
  replaced by another valid window within the expected deadline.
- A sample received later than `1,100,000 us` produces `STALE` after a previous
  valid window and establishes the approved recovery baseline. A product-main
  cadence deadline that expires without the expected call has the same stale
  meaning.
- If no valid window has ever existed in the active interval, failure to form
  one remains `UNAVAILABLE`, not `STALE`.
- A stale result contains no new usable pulse/time window and must not invoke
  `fmc_rate`.
- Runtime may retain the last valid numeric RATE for diagnostic or later
  presentation policy, but it must accompany that value with `quality =
  STALE`; the value is no longer current.
- The next accepted window restores `quality = VALID`.
- `STALE` does not affect pulse totals or the independent five-second activity
  watch.
- Intentional future inactivity does not become stale because no frequency
  deadline is expected while acquisition is inactive. Valid-zero observation
  establishes the stopped RATE behavior before that transition.

## Accepted Invalid Semantics And Recovery

- `quality = INVALID` means that a counter/timestamp sample was supplied but
  cannot be trusted to form a frequency window.
- Invalid measurement cases include an equal or decreasing timestamp relative
  to the active baseline, a producer-reported incoherent counter/timestamp
  pair, and producer-reported capture loss, overrun, or acquisition failure.
- A sample earlier than `900,000 us` is an early sample, not invalid. A sample
  later than `1,100,000 us` follows the approved stale or unavailable semantics.
  Baseline-only state is unavailable, and an admissible zero-pulse window is
  valid.
- API misuse such as a `NULL` pointer returns `FM_STATUS_EINVAL`; it does not
  create an invalid measurement-quality result.
- An invalid sample produces no pulse/time window, must not invoke `fmc_rate`,
  and discards the active frequency baseline.
- The next trusted sample establishes a new baseline and produces
  `UNAVAILABLE`. A later trusted, temporally admissible sample may restore
  `VALID`.
- Runtime may retain a previous numeric RATE for diagnostics, but that value is
  not usable while quality is invalid.
- `INVALID` does not modify the independent pulse-delta baseline, ACM/TTL, or
  the activity watch.
- An undetected true pulse delta of 65,536 or more is not an invalid sample that
  this contract can classify. It remains an externally enforced violation of
  the modulo-counter precondition.

## Accepted Runtime And RATE Handoff

- The technical observer exposes one by-value result with this semantic shape:

```c
typedef struct
{
    frequency_observation_quality_t quality;
    uint64_t pulse_delta;
    uint64_t elapsed_us;
} frequency_observation_result_t;
```

- `quality` distinguishes `VALID`, `UNAVAILABLE`, `STALE`, and `INVALID`.
  `pulse_delta` and `elapsed_us` are usable only when quality is `VALID`.
- A sample rejected as too early produces no new result and therefore no
  runtime frequency event.
- `product/main` transports each new frequency result by value through one
  dedicated `fmc_runtime` event. No additional product thread or queue is
  required.
- Runtime invokes `fmc_rate` only for a `VALID` result. It converts the accepted
  integer elapsed time to seconds after timestamp subtraction:

```text
elapsed_seconds = elapsed_us / 1,000,000
```

- `fmc_rate` remains pure and quality-agnostic. It does not retain baselines,
  validate cadence, capture time, or interpret unavailable, stale, or invalid
  observations.
- Runtime retains the current RATE value together with its quality as one
  inseparable product state. A stale or invalid transition may preserve the
  previous numeric value, but that value is not current or usable without its
  non-valid quality.
- `UNAVAILABLE`, `STALE`, and `INVALID` do not invoke RATE mathematics.
- A frequency-result event never modifies the pulse-delta baseline, ACM/TTL,
  or the independent five-second activity watch.

## Selected Phase 7E1 Physical Realization

The first target realization uses:

- independent stable cumulative reads of the existing LPTIM4 primary-pulse
  counter;
- free-running LPTIM3 as the timestamp source, without a capture channel,
  input GPIO, interrupt, or DMA;
- the 32,768 Hz LSE divided by 16, giving LPTIM3 a 2,048 Hz tick rate and
  488.28125 us physical resolution;
- software extension of consecutive 16-bit LPTIM3 observations into the
  contract's monotonic `uint64_t timestamp_us`.

LPTIM3's raw 16-bit counter wraps every 32 seconds. While frequency acquisition
is active, the producer must be sampled strictly before that interval expires.
A full or multiple raw wrap between producer calls is ambiguous and cannot be
detected by LPTIM3 alone; any known continuity violation requires discarding
the frequency time/baseline state. Future indefinite inactivity starts a new
epoch and frequency baseline instead of attempting to reconstruct elapsed
inactive time.

The selected technique does not align its timestamp to an input edge. That is
not required for pulse-count-over-elapsed-time observation. Edge-coherent
capture remains a possible future technique for period or edge-timestamp
measurement, not a prerequisite for this realization. Its open implementation
and decision work is tracked as `FREQ-1` in
`docs/roadmaps/fmc_refactoring.md`; that follow-up is not an accepted extension
of this contract until its own gates are reviewed.

Target temporal validation uses the connected stable signal generator as the
independent pattern at 1 kHz. For a valid window:

```text
reference_us = observed_pulses * 1,000,000 / generator_frequency_hz
difference_us = abs(elapsed_us - reference_us)
error_percent = 100 * difference_us / reference_us
```

The arbitrary-phase uncertainty is less than one 1 kHz input period
(`< 1,000 us`), and LPTIM3 conversion contributes less than one 488.28125 us
tick. The approved internal comparison limit is `+/-0.5%`, conservatively
inside the contractual `+/-1%` elapsed-time target given the generator's error
of much less than `0.1%`. Detailed human steps and a worked example belong to
the selected bring-up header rather than being duplicated here.

## Evidence Boundary

The legacy coherent edge-time observation is useful evidence, but its combined
ownership of capture, window formation, RATE calculation, totalization, and
presentation is not retained.

Relevant evidence and current authorities:

- `legacy/analysis/fmc_acquisition.md`;
- `docs/specs/fmc/acquisition.md`;
- `docs/product/fmc/requirements.md`;
- `src/services/pulse_delta.h`;
- `src/product/fmc/fmc_runtime.h`;
- `src/product/fmc/fmc_rate.h`.
