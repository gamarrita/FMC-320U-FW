# WORKING_CONTEXT.md

## Active Milestone

- FMC runtime foundation

## Product Behavior Authority

- `docs/specs/fmc/use_cases.yaml`

## Milestone Outcome

Establish the runtime foundation for the refactored FMC-320 firmware:

- live FMC state owned through `fmc_service`;
- RTOS-neutral runtime events for service updates and semantic input;
- stable snapshots for future presentation;
- product contracts independent from HAL, GPIO, LCD, BSP, ThreadX, queue, and
  timer types;
- a clear path to ThreadX, low-power, input recognition, acquisition, and
  presentation without redesigning public product contracts.

The milestone does not include the complete UI state machine.

## Decisions In Force

- Confirmed requirements in `docs/specs/fmc/use_cases.yaml` are the current
  product behavior authority.
- Legacy sources, including `legacy/specs/fmc/use_cases.docx`, are evidence,
  not authority over the current specification.
- `WORKING_CONTEXT.md` controls current scope, sequencing, and temporary
  boundaries; it does not override confirmed product requirements.
- Public contracts for implemented modules belong in their headers.
- BSP, HAL, GPIO, ThreadX, queue, and timer types must not leak into product
  contracts.
- Product event contracts remain RTOS-neutral until the ThreadX ownership model
  is selected deliberately.

## Current Decision Gates

- Define semantic input before implementing menu behavior.
- Preserve input key identity and action identity before mapping consequences
  such as navigation, editing, wake, backlight, or presentation invalidation.
- Define ThreadX, ISR-to-thread delivery, timer, and low-power ownership before
  implementing final short/long recognition.
- Use `use_cases.yaml` before changing observable product behavior; report
  specification, legacy, test, or code conflicts instead of resolving them
  silently.

## Next Selected Step

- Define semantic input contract and pure tests.

## Milestone Boundaries

Do not include unless explicitly selected by the current user request:

- complete menu navigation;
- persistence implementation;
- Bluetooth or printer workflows;
- complete alarm behavior;
- optional PT100 behavior;
- broad CubeMX changes;
- ThreadX enablement;
- low-power entry/exit policy;
- backlight or wake policy;
- optimization of unrelated legacy code.

## Exit Criteria

This milestone is complete when:

- runtime contracts no longer depend on BSP, HAL, GPIO, LCD, ThreadX, queue, or
  timer types;
- semantic keyboard and external-button input can reach runtime without
  interface changes for future short/long recognition;
- ThreadX ownership and ISR-to-thread delivery are defined and demonstrated;
- runtime state can produce stable presentation snapshots;
- regression tests cover the pure product contracts;
- at least one hardware bring-up demonstrates the complete input path.

## Maintenance Rule

Update this file only when one of these changes:

- the active milestone;
- the product behavior authority;
- a decision in force;
- a decision gate;
- the selected next step in a material way;
- milestone boundaries;
- exit criteria.

Do not update it only because:

- a commit was made;
- a file was added;
- a routine micro-slice was completed;
- a test passed;
- the exact implementation status changed.

## References

- `AGENTS.md`
- `STYLE.md`
- `docs/specs/fmc/use_cases.yaml`
- `docs/roadmaps/fmc_refactoring.md`
- `docs/workflow/doc_closure.md`
