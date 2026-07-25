# Working Context: Phase 7 Essential Acquisition

## Active State

- Active workstream: `Phase 7 — Essential Acquisition`.
- Workstream span: all Phase 7 slices, from 7-0A through 7G, as ordered by
  `docs/roadmaps/fmc_refactoring.md`.
- Workstream state: active.
- Active slice: `7D — Frequency-Observation Contract`.
- Slice state: opened and authorized for contract work; human decisions are
  pending.
- Next gated slice: `7E1 — Frequency Bring-Up in Run`.
- 7E1 authorization state: not approved.

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

## Active Slice: 7D Frequency-Observation Contract

7D defines the pulse/time observation consumed later by pure RATE mathematics.
It must not select or implement a hardware technique, and it must keep the
accepted LPTIM4 accumulation and totalization path independent from frequency
availability or quality.

Contract work must make explicit:

- which module owns the frequency-observation window and its baseline;
- the pulse and elapsed-time inputs, units, numeric ranges, and wrap behavior;
- required measurement range, accuracy, update latency, and cadence;
- distinct meanings for valid zero flow, absent input, stale observation, and
  invalid observation;
- how observation value and quality reach the RTOS-neutral runtime and later
  `fmc_rate` without coupling them to ACM/TTL accumulation;
- which legacy behavior is retained as evidence and which decisions require
  current human approval.

The 7D exit evidence is a reviewed specification covering time-window
ownership, range, accuracy, latency, zero, absent, stale, invalid, and RATE
handoff semantics. No CubeMX, firmware implementation, flashing, or target
bring-up is part of this slice.

## Next Gated Slice: 7E1 Frequency Bring-Up in Run

7E1 cannot open until the 7D observation contract is reviewed and the human
approves the technique, CubeMX path, signal matrix, and accuracy target.

## Current Exclusions

- further `.ioc`, CubeMX, or generated-code changes without explicit human
  approval;
- frequency-observation implementation or target bring-up;
- RATE runtime integration or presentation changes;
- user/configuration screens and navigation;
- Backup SRAM, Flash, RTC, temperature, Bluetooth, or printing;
- adopting or testing the legacy workaround.
