# Working Context: Phase 7A Pulse-Accumulation Contract

## Objective And Status

Define the hardware-configuration-independent contract for turning successive
legacy-shaped LPTIM4 counter observations into bounded pulse deltas before
configuring or implementing the MCU path.

Status:

- Phase 7-0A legacy evidence is complete in
  `legacy/analysis/fmc_acquisition.md`;
- Phase 7-0B route and document ownership are complete;
- Phase 7A is active, documentation-only, and drafted for closure review;
- LPTIM4 is the selected pulse counter, following the legacy accumulation
  shape;
- no pin, clock, filter, edge, interrupt, DMA path, workaround, or CubeMX
  change is approved.

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
from the roadmap. This cut defines only the LPTIM4 observation and
pulse-accumulation contract. It does not configure hardware, form a frequency
observation, calculate RATE, or integrate live acquisition.

## Active Deliverables

- create `docs/specs/fmc/acquisition.md` as the owner of the reviewed
  pulse-accumulation contract;
- define the accepted pulse-delta meaning, numeric bounds, first observation,
  modulo wrap, reset boundary, and ownership;
- define hardware-configuration-independent acceptance examples;
- update product documentation only for outcomes explicitly approved by the
  human;
- record exact legacy disposition only if Phase 7A accepts, rejects, replaces,
  or defers a reviewed legacy statement.

## Reviewed Phase 7A Decisions

The reviewed delta, numeric, startup, Stop2, reset, validity, legacy
traceability, and ownership decisions are recorded in
`docs/specs/fmc/acquisition.md`. Approved product outcomes are recorded in
`docs/product/fmc/requirements.md` and `docs/product/fmc/behavior.md`.

For sequencing, LPTIM4 is selected and the Phase 7A decision gates are closed.
Hardware configuration, target evidence, and implementation remain gated below.

## Additional Human Approvals Before Phase 7B1

- LPTIM4 pin, clock, filter, edge, and complete counter configuration;
- intended CubeMX change and regeneration;
- Run-mode signal matrix and acceptance evidence;
- authorization to begin the first acquisition implementation.

## Out Of Scope

- configuring or implementing LPTIM4, its pin, clock, filter, interrupt, DMA,
  or autonomous-mode path;
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
  contract without defining hardware configuration;
- normal, first-sample, zero, wrap, Stop2/delayed, precondition-boundary, reset,
  and recovery examples are explicit;
- runtime/service ownership remains unchanged and unambiguous;
- unresolved hardware choices remain gated for Phase 7B1;
- documentation closure and Git audit pass.

## Next Context Candidate

`Phase 7B1 Pulse-Counter Bring-Up In Run`

It opens only after the additional approvals above and exercises the minimum
documented counter technique in Run. It will not introduce Stop2 or RATE.
