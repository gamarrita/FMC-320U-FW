# Working Context: Phase 7 Essential Acquisition

## Active State

- Active workstream: `Phase 7 — Essential Acquisition`.
- Workstream state: administratively paused pending review of this workflow
  normalization and explicit human authorization to resume.
- Active slice: `7A — Pulse-Accumulation Contract`.
- Slice state: drafted; technical decisions are recorded; closure is not
  authorized.
- Next gated slice: `7B1 — Pulse-Counter Bring-Up In Run`.
- 7B1 authorization state: not approved.

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
- 7A decisions on delta meaning, numeric bounds, startup, Stop2, reset,
  validity, legacy traceability, and ownership are recorded in their owning
  documents.

## Active Slice: 7A Pulse-Accumulation Contract

7A defines only the hardware-configuration-independent contract that turns
successive legacy-shaped LPTIM4 counter observations into bounded pulse deltas.
It does not configure hardware, form a frequency observation, calculate RATE,
or integrate live acquisition.

7A closes only after explicit human authorization and confirmation that:

- approved product outcomes remain in their owning product documents;
- `docs/specs/fmc/acquisition.md` defines one reviewable contract without
  defining hardware configuration;
- normal, first-sample, zero, wrap, Stop2/delayed, precondition-boundary, reset,
  and recovery examples are explicit;
- runtime/service ownership remains unchanged and unambiguous;
- unresolved hardware choices remain gated for 7B1;
- documentation and the Git diff have been reviewed.

No further 7A closure action is authorized while the workstream pause remains.

## Next Gated Slice: 7B1 Pulse-Counter Bring-Up In Run

7B1 exercises the minimum documented LPTIM4 counter technique in Run without
introducing Stop2 or RATE. It cannot open until the human approves:

- LPTIM4 pin, clock, filter, edge, and complete counter configuration;
- intended CubeMX change and regeneration;
- Run-mode signal matrix and acceptance evidence;
- authorization to begin the first acquisition implementation.

## Current Exclusions

- closing 7A or activating 7B1 while the administrative pause remains;
- configuring or implementing LPTIM4 or its hardware path;
- `.ioc` edits, CubeMX regeneration, or generated-code edits;
- hardware bring-up;
- frequency observation, RATE integration, or presentation changes;
- user/configuration screens and navigation;
- Backup SRAM, Flash, RTC, temperature, Bluetooth, or printing;
- adopting or testing the legacy workaround.
