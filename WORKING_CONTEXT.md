# WORKING_CONTEXT.md

## Active Milestone

- FMC reduced product runtime on ThreadX

## Product Behavior Authority

- `docs/specs/fmc/use_cases.yaml`

## Milestone Outcome

Run a reduced FMC product runtime under ThreadX while preserving the product
contracts established by the runtime foundation:

- ThreadX as the only active runtime and execution path;
- live FMC state owned through `fmc_service`;
- one ThreadX owner thread for the live `fmc_runtime`;
- serialized ISR-to-thread delivery for mechanical keyboard input;
- RTOS-neutral runtime events for service updates and semantic input inside
  the product boundary;
- stable snapshots for future presentation;
- product contracts independent from HAL, GPIO, LCD, BSP, ThreadX, queue, and
  timer types;
- a reduced path from mechanical keyboard release to provisional semantic
  `SHORT` runtime input.

The milestone does not include the complete UI state machine or final
short/long recognition.

## Decisions In Force

- Confirmed requirements in `docs/specs/fmc/use_cases.yaml` are the current
  product behavior authority.
- Legacy sources, including `legacy/specs/fmc/use_cases.docx`, are evidence,
  not authority over the current specification.
- `WORKING_CONTEXT.md` controls current scope, sequencing, and temporary
  boundaries; it does not override confirmed product requirements.
- Public contracts for implemented modules belong in their headers.
- ThreadX is the only active development and execution route.
- The last bare-metal state is retained only as historical comparison baseline
  at tag `bare-metal-before-threadx`; it is not a parallel architecture.
- The current `fm_app_threadx.*` harness adapts selectable apps to the CubeMX
  ThreadX bootstrap. It is not the final `fmc_runtime` owner thread.
- BSP, HAL, GPIO, ThreadX, queue, and timer types must not leak into product
  contracts.
- `fmc_runtime` is owned by one dedicated ThreadX thread.
- ISR paths must not call `FMC_RUNTIME_Dispatch()` directly.
- ISR-to-runtime delivery uses a ThreadX queue with an app-level keyboard event
  payload, not a `fmc_runtime_event_t` as the ISR-facing contract.
- The initial queue depth is 8 events.
- Queue overflow is considered abnormal for mechanical keyboard input. The
  implementation may panic or reset the queue and enqueue the newest event, but
  must make the abnormal condition explicit.
- Until final recognition is implemented, release events from mechanical keys
  produce provisional `SHORT` runtime input and press events do not dispatch a
  runtime input event.
- Timer ownership for final long-press recognition is deferred. The current
  preferred direction is a simple timer armed on press and disarmed on release
  before the 3 second threshold.
- The hardware is currently assumed not to produce mechanical key bounce.
- ThreadX low-power support is enabled for idle/run visibility only. The
  current slice uses conservative MCU Sleep/WFI, not STOP mode. It does not
  define product low-power policy, alternate wake timers, or tick adjustment.

## Current Decision Gates

- Define semantic input before implementing menu behavior.
- Preserve input key identity and action identity before mapping consequences
  such as navigation, editing, wake, backlight, or presentation invalidation.
- Define the minimal ThreadX runtime owner, ISR-to-thread keyboard queue, and
  overflow behavior before implementing final short/long recognition.
- Defer low-power, wake, backlight, and final timer ownership decisions until
  their selected slices.
- Use `use_cases.yaml` before changing observable product behavior; report
  specification, legacy, test, or code conflicts instead of resolving them
  silently.

## Next Selected Step

- Close the ThreadX bootstrap baseline by validating every selectable app with
  the canonical build flow and required UART or hardware observation.
- Then define and implement the minimal ThreadX runtime owner thread and
  ISR-to-thread keyboard delivery skeleton for mechanical-key provisional
  `SHORT` events.

## Milestone Boundaries

Do not include unless explicitly selected by the current user request:

- complete menu navigation;
- persistence implementation;
- Bluetooth or printer workflows;
- complete alarm behavior;
- optional PT100 behavior;
- broad CubeMX changes beyond human-selected ThreadX and low-power-support
  enablement;
- low-power entry/exit policy;
- backlight or wake policy;
- final 3 second long-press recognition;
- debounce implementation;
- external buttons `EXT_1` and `EXT_2`;
- optimization of unrelated legacy code.

## Exit Criteria

This milestone is complete when:

- runtime contracts no longer depend on BSP, HAL, GPIO, LCD, ThreadX, queue, or
  timer types;
- semantic mechanical-key input can reach runtime under ThreadX without
  interface changes for future short/long recognition;
- ThreadX ownership and ISR-to-thread delivery are implemented and
  demonstrated for the reduced product runtime;
- runtime state can produce stable presentation snapshots;
- regression tests cover the pure product contracts;
- at least one hardware bring-up demonstrates the mechanical keyboard input
  path into the runtime owner thread.

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
