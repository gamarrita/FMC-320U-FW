# Working Context: Phase 9 RTC Observation And Date-Time User Screen

## Objective And Status

Implement the bounded Phase 9 capability defined in
`docs/roadmaps/fmc_refactoring.md`: coherent RTC/calendar observation,
validity, and the read-only DATE_TIME user screen.

Status:
- Phase 8 is completed and human-accepted;
- UI-MODEL-1 is completed and accepted as an executable prototyping and
  contract-initiation tool, not as complete product authority;
- Phase 9 is the single active workstream;
- contract and existing-RTC ownership analysis is the single active slice;
- no firmware, CubeMX, generated source, RTC configuration, or DATE_TIME
  behavior change is authorized in this analysis slice.

Audit baseline before the UI-MODEL-1 closure commit:
- branch: `main`;
- HEAD: `20d006c3151e406280dfbb883894b6309449a2d0`.

## Authorities And Evidence

- Phase strategy, dependencies, gates, and exit criteria:
  `docs/roadmaps/fmc_refactoring.md`, Phase 9;
- accepted product UI behavior and contract-promotion rule:
  `docs/product/fmc/user_interface.md`;
- cross-cutting presentation behavior: `docs/product/fmc/behavior.md`;
- physical LCD and glyph authority:
  `docs/specs/lcd/lcd_true_source.yaml`;
- hardware configuration authority: `fmc-320u-v2.ioc` and its CubeMX-generated
  sources;
- current authored RTC initialization evidence: `src/port/fm_port_rtc.c`;
- implemented semantic UI contract: `src/product/fmc/fmc_ui.h`;
- executable Phase 8 evidence:
  `src/apps/tests/regression/fm_regression_test.c`;
- LCD Studio modeling input and completed MVP strategy:
  `tools/lcd-ui-studio/` and `docs/roadmaps/lcd_ui_modeling.md`.

Product documents remain authoritative for accepted behavior. LCD Studio may
initiate the detailed DATE_TIME composition, but the selected composition
becomes authoritative only after explicit human review and promotion under the
rule in `docs/product/fmc/user_interface.md`.

## Active Slice Boundary

Analyze the existing CubeMX RTC configuration, the authored
`FM_PORT_RTC_Init()` duplication/ownership concern, current startup and wakeup
use, accepted and legacy date/time evidence, and the minimum product decisions
needed for a read-only DATE_TIME contract.

Produce no firmware implementation in this slice. Use LCD Studio only to
compare concrete valid/invalid screen proposals after the product decision
inventory is explicit.

## Required Decisions And Deliverables

- distinguish CubeMX-owned RTC initialization from repository-facing calendar
  observation and wakeup responsibilities;
- decide the disposition of `FM_PORT_RTC_Init()` without editing generated or
  protected files;
- define coherent read, validity, initialization, reset, and recovery behavior;
- define and review valid and invalid DATE_TIME screen compositions and their
  unchanged Phase 8 navigation;
- identify the smallest deterministic frame/transition fixture support, if
  any, required before firmware implementation;
- propose the next bounded implementation slice and its verification gate.

## Explicit Exclusions

- RTC or general configuration editing;
- authorization, cursor, draft, apply/cancel, and persistence mechanics;
- Backup SRAM and Flash ownership;
- generated production C from LCD Studio;
- changes to `fmc-320u-v2.ioc`, generated `Core/` code, or protected files
  without a separate explicit human gate;
- unrelated PRINT, LOG_DOWNLOAD, temperature, Bluetooth, or alarm behavior.

## Slice Exit Gate

The analysis slice closes only when RTC ownership conflicts and product
decisions are explicit, the human has reviewed the proposed valid/invalid
DATE_TIME contract, and one bounded firmware implementation slice can begin
without inventing product behavior.
