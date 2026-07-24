# Working Context: Phase 7A Pulse-Accumulation Contract

## Objective And Status

Define the hardware-independent contract for turning physical primary-sensor
observations into bounded pulse deltas before selecting or implementing an MCU
technique.

Status:

- Phase 7-0A legacy evidence is complete in
  `legacy/analysis/fmc_acquisition.md`;
- Phase 7-0B route and document ownership are complete;
- Phase 7A is active and documentation-only;
- no peripheral, pin, clock, interrupt, DMA path, workaround, or CubeMX change
  is approved.

## Authorities And Evidence

- program order, complete Phase 7 route, dependencies, gates, risks, bring-up
  protocol, and exit evidence: `docs/roadmaps/fmc_refactoring.md`;
- product obligations: `docs/product/fmc/requirements.md`;
- observable behavior: `docs/product/fmc/behavior.md`;
- visible behavior, only when affected:
  `docs/product/fmc/user_interface.md`;
- technical specification folder rules: `docs/specs/README.md`;
- implemented ownership boundaries: `src/product/fmc/fmc_runtime.h`,
  `src/product/fmc/fmc_service.h`, and `src/product/fmc/fmc_rate.h`;
- current hardware configuration: `fmc-320u-v2.ioc`, read only; CubeMX remains
  the hardware-configuration authority;
- historical evidence and acquisition hypotheses:
  `legacy/analysis/fmc_acquisition.md`;
- frozen legacy product evidence:
  `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- reviewed legacy disposition: `docs/workflow/fmc_legacy_coverage.md`.

Legacy sources and the reported STM32U575 behavior are evidence, not current
product or hardware authority.

## Active Slice Boundary

Phase 7A inherits the durable acquisition boundaries and correct-first baseline
from the roadmap. This cut defines only the hardware-independent
pulse-accumulation contract. It does not select hardware, form a frequency
observation, calculate RATE, or integrate live acquisition.

## Active Deliverables

- create `docs/specs/fmc/acquisition.md` as the owner of the reviewed
  pulse-accumulation contract;
- define the accepted pulse-delta meaning, numeric bounds, first observation,
  wrap, ambiguity, invalid observation, resynchronization, and ownership;
- define hardware-independent acceptance examples;
- update product documentation only for outcomes explicitly approved by the
  human;
- record exact legacy disposition only if Phase 7A accepts, rejects, replaces,
  or defers a reviewed legacy statement.

## Decisions Required Before Phase 7B1

- supported sensor electrical and timing envelope;
- operating and low-power states in which accepted pulses must accumulate;
- maximum observation latency and counter-wrap assumptions;
- pulse-loss guarantee and behavior when exact delta recovery is impossible;
- startup, reset, and resynchronization semantics;
- error/status information delivered with a pulse delta.

The legacy claim that repeatable loss of one pulse is unacceptable is a
candidate for review, not an accepted bound.

## Out Of Scope

- selecting or implementing a counter, pin, clock, filter, interrupt, DMA, or
  autonomous-mode path;
- `.ioc` edits, CubeMX regeneration, or generated-code edits;
- hardware bring-up;
- frequency observation, RATE integration, or presentation changes;
- user/configuration screens and navigation;
- Backup SRAM, Flash, RTC, temperature, Bluetooth, or printing;
- adopting or testing the legacy workaround.

## Closure Criteria

Phase 7A closes when:

- approved product outcomes are recorded in their owning product documents;
- `docs/specs/fmc/acquisition.md` defines one reviewable pulse-accumulation
  contract without selecting hardware;
- normal, first-sample, wrap, ambiguous, invalid, and recovery examples are
  explicit;
- runtime/service ownership remains unchanged and unambiguous;
- unresolved hardware choices remain gated for Phase 7B1;
- documentation closure and Git audit pass.

## Next Context Candidate

`Phase 7B1 Pulse-Counter Bring-Up In Run`

It will select the minimum documented counter technique only after human
approval of the peripheral, pin, clock, filter, CubeMX change, signal matrix,
and acceptance evidence. It will not introduce Stop2 or RATE.
