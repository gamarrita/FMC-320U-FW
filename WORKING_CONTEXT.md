# Working Context: Phase 7 Essential Acquisition

## Active State

- Active workstream: `Phase 7 — Essential Acquisition`.
- Workstream span: all Phase 7 slices, from 7-0A through 7G, as ordered by
  `docs/roadmaps/fmc_refactoring.md`.
- Workstream state: active.
- Active slice: `7F — RATE Integration`.
- Slice state: opened by human approval after accepting all required 7D and 7E
  contract, physical, regression, and current evidence. Entry review and
  human-gated implementation decisions remain pending; no 7F code change has
  been authorized.
- Completed slice: `7E2 — Silent Frequency Current Characterization`.
- 7E2 completion state: human-accepted. The approved PPK2 run measured 23 uA
  average over approximately 74 silent seconds. Reinstalling the debug jumpers
  without reset restored indication and UART while the observation sequence
  continued.
- Completed slice: `7E1 — Unified Functional Frequency Bring-Up`.
- 7E1 completion state: human-accepted. The complete `0 Hz`, `1 Hz`, `100 Hz`,
  and `1 kHz` physical matrix passed, including elapsed-time admission,
  quality transitions, four LPTIM3 wraps, and two LPTIM4 wraps. The canonical
  `tests/regression` build passed and target execution reported every case
  `PASS`, including `FREQUENCY_OBSERVATION_VECTORS`,
  `OBSERVER_INDEPENDENCE`, and the final aggregate `REGRESSION_TEST:PASS`.
- Conditional slice 7E3 was not entered because 7E1 and 7E2 revealed no unmet
  constraint or reproducible failure.
- Next gated slice: `7G — Combined Live Integration`.

## Workstream Objective

Replace provisional Phase 6A TTL/RATE inputs through the incremental
acquisition route without coupling pulse-total correctness to frequency
availability.

Phase 7 keeps these responsibilities separate:

- physical pulse accumulation;
- counter observation and pulse-delta formation;
- ACM/TTL totalization;
- physical frequency observation;
- pulse/time observation quality;
- pure RATE mathematics;
- runtime and presentation integration.

Program order, slice dependencies, decision gates, bring-up protocol, risks,
and Phase 7 exit evidence belong to `docs/roadmaps/fmc_refactoring.md`.

## Authorities And Evidence

- product obligations: `docs/product/fmc/requirements.md`;
- observable behavior: `docs/product/fmc/behavior.md`;
- visible behavior, only when affected:
  `docs/product/fmc/user_interface.md`;
- reviewed pulse-accumulation contract:
  `docs/specs/fmc/acquisition.md`;
- reviewed frequency-observation decisions:
  `docs/specs/fmc/frequency_observation.md`;
- implemented ownership boundaries: `src/product/fmc/fmc_runtime.h`,
  `src/product/fmc/fmc_service.h`, and `src/product/fmc/fmc_rate.h`;
- current hardware configuration: `fmc-320u-v2.ioc`, read only; CubeMX remains
  the hardware-configuration authority;
- historical acquisition evidence and hypotheses:
  `legacy/analysis/fmc_acquisition.md`;
- frozen legacy product evidence:
  `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- reviewed legacy disposition: `docs/workflow/fmc_legacy_coverage.md`.

Legacy sources and the reported STM32U575 behavior remain evidence, not current
product, technical, or hardware authority.

## Phase-Wide Boundaries

- Pulse accumulation must be completed before frequency observation can
  endanger totalization.
- Physical observation remains separate from RATE mathematics.
- Acquisition reaches product totals only through the reviewed runtime/service
  ownership boundary.
- Hardware configuration and every CubeMX change require explicit human
  approval.
- Bring-up begins with the minimum documented correct technique. A legacy
  workaround is considered only after an approved target failure is reproduced.
- Each slice retains its own entry gates and exit evidence; advancing between
  slices updates this context but does not replace the Phase 7 workstream.

## Recorded Progress

- 7-0A legacy evidence is complete in
  `legacy/analysis/fmc_acquisition.md`.
- 7-0B document ownership, route, gates, and correct-first baseline are
  complete.
- Phase 6A provides the current runtime/presentation path and provisional
  TTL/RATE inputs that Phase 7 will replace.
- 7A is human-approved and closed. Its decisions on delta meaning, numeric
  bounds, startup, Stop2, reset, validity, legacy traceability, and ownership
  are recorded in their owning documents.
- The human configured and regenerated LPTIM4 through CubeMX. The audited
  configuration uses locked `PD13/LPTIM4_IN1`, asynchronous external counting,
  falling edges, prescaler 1, direct transition without a digital filter,
  software trigger, a 16-bit full-range period, and the shared LPTIM3/4 LSE
  selection.
- LPTIM4 autonomous-mode clock access is not exposed by CubeMX and is accepted
  as a one-time firmware initialization action.
- The documented loss of the first five active edges after asynchronous counter
  startup is accepted for this instrument. The implementation must make that
  limitation explicit at the LPTIM4 wrapper and use consecutive equal counter
  reads as recommended by ST.
- The 7B1 port wrapper and selectable pulse-counter bring-up are implemented.
  The canonical Debug build passed, STM32CubeProgrammer verified the download
  on target, and UART observations remained stable at zero with no applied
  input pulses.
- The human observed correct pulse accumulation at 50 Hz while the app task
  slept between one-second observations. The existing ThreadX idle path enters
  Stop2 during those sleeps; the Run LED was mostly off and pulsed on at each
  wake, consistently with the implemented low-power hooks.
- The human observed approximately 20 uA during most of that instrumented run,
  with short wake/observation peaks producing an average near 25 uA. This is
  useful Stop2 characterization, not the separate silent-current acceptance
  required by 7B2.
- The human verified correct counting at the approved 1 kHz input limit and
  accepted the 7B1 implementation evidence. A separate successful 3 kHz trial
  records technical margin only; it does not raise the accepted 1 kHz product
  limit.
- 7B1 is human-accepted and closed.
- The human confirmed correct 16-bit rollover behavior while using the same
  one-second observation loop that traverses the configured ThreadX idle
  Stop2 path.
- Normal debug UART output is now an internally gated, caller-neutral
  best-effort operation. Disabled messages no longer produce a false failure
  that can drive bring-up control flow into panic, and all debug UART paths are
  silent under disabled message policy. Canonical Debug builds passed for the
  affected pulse-counter and keyboard-input bring-ups; the corrected
  pulse-counter image was verified on target.
- With both debug jumpers removed, the human observed a 20 uA low-current
  plateau and no UART transmissions.
- The pulse-counter bring-up now resamples message and LED jumpers once per
  observation. Installing both jumpers after a silent interval without
  resetting resumed count output and Run LED activity as expected. Removing
  them again restored silent behavior. This confirms that the same execution
  remained live rather than entering panic during the silent interval.
- 7B2 is human-accepted and closed. Its evidence covers correct counting
  through Stop2, correct 16-bit rollover, a 20 uA silent low-current plateau,
  UART silence under disabled message policy, and dynamic-jumper liveness.
- The pure `pulse_delta` service owns the accepted zero baseline, successive
  16-bit observations, modulo subtraction, `uint64_t` widening, and reset. It
  has no hardware, timing, runtime, totals, frequency, or RATE dependency.
- The regression app contains deterministic vectors for first observation,
  zero advance, normal delta, modulo wrap, delayed observation, largest valid
  delta, reset, cumulative conservation, and invalid pointers. The canonical
  Debug build with `tests/regression` selected completed successfully.
- The human executed that regression image on target with message and LED
  jumpers enabled. `REGRESSION_TEST:PULSE_DELTA_VECTORS:PASS` and the final
  `REGRESSION_TEST:PASS` were observed; every other reported regression case
  also passed.
- 7B3 is human-accepted and closed. Its `pulse_delta` contract, canonical
  build, focused regression vectors, and complete target regression suite are
  accepted.
- 7C is human-accepted and closed. The existing product-main owner performs
  one stable counter observation and one runtime pulse-delta dispatch per
  periodic event. Runtime accepts zero as a no-op, while `fmc_service` remains
  the only owner that applies nonzero deltas exactly once to ACM and TTL.
- Canonical Debug builds passed for `tests/regression` and `product/main`.
  STM32CubeProgrammer verified the product image, and target UART showed equal,
  monotonic ACM/TTL totals from `41/41` through `86/86`.
- The human confirmed that the applied input was 5 Hz. Consecutive one-second
  telemetry increments of five therefore complete the accepted 7C target
  evidence for conservation without loss or duplication.
- 7D is human-accepted and closed. Its frequency-window ownership, input,
  numeric, range, accuracy, cadence, quality, recovery, and RATE-handoff
  decisions are recorded in `docs/specs/fmc/frequency_observation.md`.
- A dedicated RTOS-neutral technical observer owns an independent
  counter/timestamp baseline. Inputs use `uint16_t pulse_count` and monotonic
  `uint64_t timestamp_us`; the first pair establishes the baseline, pulse
  subtraction is modulo 65,536, and timestamp wrap is not supported.
- The first realization targets `1 Hz..1 kHz`, nominal one-second windows,
  inclusive temporal admission of `900,000..1,100,000 us`, exact physical
  pulse agreement, and `+/-1%` elapsed-time accuracy. Below `1 Hz`, RATE is not
  guaranteed but totalization remains correct; legacy `0.1 Hz` and
  `+/-0.1 Hz` indications are not requirements.
- Quality distinguishes `VALID` (including a zero-pulse window),
  `UNAVAILABLE`, `STALE`, and `INVALID`, with the approved baseline recovery
  rules. No non-valid result changes totals or activity.
- `product/main` remains the sole caller and future `IDLE/ACTIVE` owner. The
  target lifecycle permits inactivity after five seconds without newly counted
  pulses, but sleep-entry and activity-detection mechanisms remain unselected;
  the first integration retains the current one-second refresh.
- Pure and integration regressions cover exact observer behavior and
  independent totalization/frequency sequences. New results travel by value
  through a dedicated runtime event; runtime invokes pure `fmc_rate` only for
  `VALID` and retains RATE with explicit quality.
- For 7E1, the approved minimum Run technique reuses independent stable
  cumulative observations of the existing LPTIM4 pulse counter. It does not
  require the frequency and totalization paths to share one read or align to an
  input edge. LPTIM3 supplies only a free-running LSE timestamp at `2,048 Hz`
  from the `32,768 Hz` LSE with prescaler 16; it adds no capture path, input
  pin, interrupt, or DMA. Its 16-bit count is extended in software while
  acquisition is active and must be sampled in strictly less than its
  32-second wrap interval. A cadence violation discards temporal continuity
  and requires a new time/frequency baseline. The connected signal generator
  is set to the approved initial diagnostic point of `100 Hz`.
- The approved CubeMX path enables LPTIM3 as a free-running internal counter
  clocked from the `32,768 Hz` LSE with prescaler 16, period 65,535, software
  trigger, immediate update, and repetition count zero. It adds no capture
  channel, GPIO, interrupt, NVIC route, or DMA. Human CubeMX regeneration has
  been audited; authored firmware later enables LPTIM3 autonomous clock access
  and starts the continuous counter.
- The approved 7E1 Run frequency matrix uses `0 Hz`, `1 Hz`, the initial
  nominal point `100 Hz`, and the approved upper bound `1 kHz`. The `1 kHz`
  case runs for at least 70 seconds so one test crosses at least two LPTIM3
  wraps and one LPTIM4 wrap.
- The functional bring-up uses the existing `FM_APP` task and
  `FM_PORT_TIME_SleepMs(1000U)` between observations. Normal ThreadX idle
  therefore enters Stop2 transparently, as in the accepted pulse-counter
  bring-up, while LPTIM3 supplies actual elapsed time rather than assuming a
  one-second window. Pure observer regressions inject samples without physical
  sleeps. 7E2 reuses the same firmware for explicit Stop2 and silent-current
  acceptance; forced-Run execution is reserved for diagnosing a failure rather
  than required as a default second implementation.
- At the initial `100 Hz` point, the approved physical quality sequence is:
  first-sample `UNAVAILABLE`; a 500 ms early sample without result or baseline
  advance; another 500 ms producing the first `VALID` window; two additional
  approximately one-second `VALID` windows; a 1,200 ms late sample producing
  `STALE` with resynchronization; and an approximately one-second `VALID`
  recovery window. LPTIM3 actual elapsed time, not the requested sleeps,
  determines temporal admission.
- The approved independent temporal validation uses the connected stable
  signal generator as the external reference at `1 kHz`, with stability much
  better than `0.1%`. LPTIM4 supplies the pulse count and LPTIM3 supplies
  `elapsed_us`; arbitrary edge phase contributes less than one `1 ms` input
  period and LPTIM3 quantization contributes less than one `488.28125 us`
  tick. An internal comparison limit of `+/-0.5%` conservatively demonstrates
  the contractual `+/-1%` target. No LED marker, LCD timing indication, or
  edge-capture technique is required. The bring-up documentation must give the
  human detailed arithmetic, units, uncertainty bounds, and pass/fail steps
  for validating each reported measurement.
- The first submitted `100 Hz` target log demonstrates the approved quality
  sequence: baseline `UNAVAILABLE` at count 0/time 37,597 us; early rejection
  at 508,301 us without baseline advance; first `VALID` at 1,019,043 us with
  101 pulses; two further `VALID` windows at 1,023,438 us/102 pulses and
  1,013,672 us/102 pulses; late `STALE` after 1,222,167 us; and `VALID`
  recovery at 1,011,231 us/101 pulses. Subsequent steady observations remain
  valid and consistent with the applied 100 Hz pattern.
- That log also exposed truncation of three bring-up instruction strings at
  the debug transport's intentional 96-byte per-call limit. The measurement
  lines were intact. Human direction established that development telemetry
  must preserve compact production-like behavior rather than fragmenting long
  prose across transmissions. Runtime output now uses one compact record per
  state/sample; detailed arithmetic remains in the bring-up header.
- The first compact-record implementation used newlib-nano `snprintf()` with
  64-bit integer formats and produced a target HardFault on the first sample;
  the debugger stack isolated the fault to that formatting path before any
  observation telemetry was sent. Local and external investigation confirmed
  that the canonical `--specs=nano.specs` configuration enables nano formatted
  I/O without `_WANT_IO_LONG_LONG` or `_WANT_IO_C99_FORMATS`. Human approval
  selected bounded `snprintf()` using only supported 32-bit C89 conversions:
  the `uint64_t` timestamp is represented as `seconds.microseconds`, while
  valid pulse delta and elapsed microseconds are already bounded to 32 bits.
- The compact target rerun removed the formatting HardFault and reproduced the
  approved quality sequence. It also made the LPTIM4 startup loss visible:
  baseline count zero followed by only 97 counted pulses in a 1,012,207 us
  first window at 100 Hz. Human approval added bring-up-only conditioning:
  after starting LPTIM4, wait up to one second at the approved initial 100 Hz
  until a stable raw count is nonzero, then establish the frequency baseline.
  This proves the accepted five-edge hardware synchronization loss has ended
  before any physical frequency window; it changes neither port nor observer
  semantics.
- The target rerun after that conditioning is the accepted `100 Hz` evidence:
  baseline began at raw count 2/time 59,570 us; the 511,230 us early sample
  produced no result and did not advance the baseline; the accumulated first
  `VALID` window reported 102 pulses over 1,025,390 us; the next two windows
  reported 101 pulses over 1,006,348 us and 1,014,649 us; the 1,200,195 us
  late interval produced `STALE`; and the next 1,004,883 us interval restored
  `VALID` with 101 pulses. Later steady windows remained temporally admissible
  and consistent with the applied stable 100 Hz pattern. Compact bounded
  `snprintf()` telemetry completed without HardFault.
- The accepted `0 Hz` transition evidence was collected without reset after
  the 100 Hz run. The transition window reported the final 16 pulses, then raw
  LPTIM4 count remained fixed at 946 for five consecutive windows totaling
  more than five seconds. Every complete zero-pulse window reported
  `quality = VALID`, `pulse_delta = 0`, and an admitted measured elapsed time;
  no stale, invalid, baseline reset, or totalization assumption was involved.
- The accepted `1 Hz` transition evidence was likewise collected during the
  unified steady observation loop, which permits ThreadX Stop2 between
  deadlines. The mixed transition sample 15, containing the final 16 pulses
  from 100 Hz, is excluded. Samples 16 through 22 then provide seven
  consecutive `VALID` windows, each with exactly one newly counted pulse and
  elapsed times from 1,000,488 us through 1,015,137 us, all inside the
  observer's admitted window. Across those samples the counter advanced
  exactly seven pulses over 7,045,410 us. The 1 kHz case, not this phase-
  sensitive 1 Hz case, remains the approved independent temporal-accuracy
  check because its pulse reference bounds endpoint phase to below 1 ms.
- An additional live matrix run reconfirmed seven consecutive valid-zero
  windows, then transitioned to `1 kHz`. The transition sample 24, containing
  250 pulses, is excluded as a settled-frequency observation and retained as
  the aggregate baseline. Samples 25 through 157 provide 133 consecutive
  `VALID` settled windows over 134,010,742 us, with 134,007 observed pulses:
  aggregate frequency 999.9721 Hz and aggregate temporal error 0.0028% against
  the stable 1 kHz generator. Every individual elapsed interval remained
  inside the observer admission window; the worst individual pulse-derived
  temporal error was 0.1021%, below the approved internal 0.5% limit.
  Monotonic observation continued across four LPTIM3 raw-wrap boundaries
  (near 32, 64, 96, and 128 seconds) and exact modular pulse accounting
  continued across two LPTIM4 wraps (64877 + 1001 -> 342, and
  64847 + 1002 -> 313, modulo 65536). This exceeds the approved minimum hold
  of 70 seconds, two LPTIM3 raw wraps, and one LPTIM4 raw wrap.
- The human rejected the proposed adaptive reset-start handling for `0 Hz` and
  `1 Hz` as unnecessary. The bring-up retains its explicit initial `100 Hz`
  setup and one-second LPTIM4 synchronization bound; matrix frequency changes
  occur during the steady `RUN` portion. No adaptive no-edge timeout behavior
  was implemented.

## Completed Slice: 7E2 Silent Frequency Current Characterization

7E1 is human-accepted and closed. Its one functional implementation used
`FM_PORT_TIME_SleepMs(1000U)` in the normal ThreadX application task, so idle
transparently permitted Stop2 between observation deadlines. There is no
forced-Run reference implementation and no second Stop2 implementation.

7E2 retains only the separate silent-current evidence required to avoid
confusing UART/LED instrumentation current with the accepted acquisition path.
It does not repeat the functional frequency matrix, change the selected
LPTIM3/LPTIM4 technique, or add code unless target evidence reveals a
reproducible unmet constraint.

The approved measurement method uses the Power Profiler Kit II with the
generator fixed at 100 Hz. Both debug-message and debug-LED jumpers are removed
before reset and remain removed throughout the accepted capture. After 15
seconds, the PPK2 records at least ten observation intervals. The board must
not be powered simultaneously from the PPK2 and another source. After the
accepted capture, the message jumper may be installed as a separate liveness
check; frequency bring-up observations resample the jumper and should resume
telemetry within approximately one interval.

`bringups/frequency_observation` is selected again as the active CMake app. Its
canonical Debug configuration and build pass, and the resulting ELF is the
image prepared for the silent-current target run.

The accepted PPK2 capture measured 23 uA average during approximately 74
seconds with both debug jumpers removed. After reinstalling the jumpers without
reset, telemetry resumed at sample 76 with timestamp 74,489,746 us and
continued through sample 92 with every window `VALID`. Counts remained
consistent with the applied 100 Hz signal, elapsed windows remained admitted,
and the monotonic timestamp continued transparently beyond two 32-second
LPTIM3 raw-wrap boundaries. The continued sample index, counter, and timestamp
show that the same acquisition execution remained live throughout the silent
interval.

The 7E2 exit evidence is human-accepted and the slice is closed.

## Active Slice: 7F RATE Integration

7F feeds accepted pulse/time observations into the existing pure `fmc_rate`
calculation with explicit quality. The accepted 7D handoff requires one
by-value frequency-result event, conversion of integer `elapsed_us` to seconds
only for `VALID`, and retention of RATE value and quality together. It must
preserve the independent pulse-delta path and may not update ACM or TTL.

No 7F implementation change is authorized yet. Its entry review must reconcile
the accepted handoff with the existing `fmc_rate` and `fmc_runtime` contracts,
then present the first unresolved human decision before editing.

## Next Gated Slice: 7G Combined Live Integration

7G cannot open until 7F math, runtime, and target evidence are
human-accepted.

## Current Exclusions

- further `.ioc`, CubeMX, or generated-code changes without explicit human
  approval;
- another frequency-observation implementation or functional matrix;
- 7F RATE runtime integration before its entry decisions are approved;
- presentation changes before 7G;
- user/configuration screens and navigation;
- Backup SRAM, Flash, RTC, temperature, Bluetooth, or printing;
- adopting or testing the legacy workaround.
