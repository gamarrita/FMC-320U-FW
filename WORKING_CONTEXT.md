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
- a reduced path from mechanical keyboard edges to semantic `SHORT` and `LONG`
  runtime input.

The milestone does not include the complete UI state machine.

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
  ThreadX bootstrap. For `product/main`, its `FM_APP` thread executes
  `FM_MAIN_Main()` as the live `fmc_runtime` owner loop.
- BSP, HAL, GPIO, ThreadX, queue, and timer types must not leak into product
  contracts.
- `fmc_runtime` is owned by the existing `FM_APP` ThreadX thread.
- ISR paths must not call `FMC_RUNTIME_Dispatch()` directly.
- ISR-to-runtime delivery uses a ThreadX owner queue with app-level event
  payloads, not a `fmc_runtime_event_t` as the ISR-facing contract. The queue
  currently carries keyboard events, key-hold timeout events, and a
  provisional 1 second periodic refresh event.
- The initial queue depth is 8 events.
- Queue overflow is considered abnormal for mechanical keyboard input. The
  implementation may panic or reset the queue and enqueue the newest event, but
  must make the abnormal condition explicit.
- The minimal owner-loop short/long recognizer lives in `product/main`.
  Hardware-observed RISING starts one active hold, the 3 second timeout emits
  one semantic `LONG`, and FALLING emits semantic `SHORT` only when no `LONG`
  was already emitted for that hold.
- Timer ownership for the short/long recognizer belongs in `product/main`,
  not in `src/product/fmc`. It uses a simple one-shot timer armed on RISING and
  cancelled on FALLING before the 3 second threshold.
- The hardware is currently assumed not to produce mechanical key bounce.
- The human corrected CubeMX keyboard GPIO mode to falling-and-rising EXTI and
  regenerated/flashed the firmware. Hardware UART smoke confirms RISING and
  FALLING delivery for DOWN, UP, ENTER, and ESC.
- ThreadX low-power support uses the ST scheduler hook path. The current port
  uses CubeMX-generated LPTIM1 plus a local STOP2 compensation layer for idle
  wake and tick adjustment; product-level wake/backlight policy remains
  deferred.
- Hardware smoke validation on target confirms ThreadX idle low power is
  working, with observed current dropping to approximately 23 uA.
- Prior hardware smoke validation on target confirmed the `product/main`
  owner-loop keyboard path for DOWN, UP, ENTER, and ESC falling edges as
  provisional `SHORT` input events. It also confirmed the 1 second periodic
  refresh wake-up path through visible screen refresh.
- Hardware smoke validation on target confirms the implemented short/long
  recognizer for DOWN, UP, ENTER, and ESC: each key emits `SHORT` when RISING
  is followed by FALLING before 3 seconds, each key emits one `LONG` while held
  past approximately 3 seconds, and release after `LONG` does not emit a
  duplicate `SHORT`.

## Current Decision Gates

- Define semantic input before implementing menu behavior.
- Preserve input key identity and action identity before mapping consequences
  such as navigation, editing, wake, backlight, or presentation invalidation.
- Close the minimal short/long recognizer slice before adding menu, wake,
  backlight, debounce, or richer input consequences.
- Defer low-power, wake, backlight, and final timer ownership decisions until
  their selected slices.
- Use `use_cases.yaml` before changing observable product behavior; report
  specification, legacy, test, or code conflicts instead of resolving them
  silently.

## Next Selected Step

- Apply doc closure for the completed minimal short/long recognizer slice.
  Preserve the current boundaries: menu consequences, debounce, wake,
  backlight, external buttons, and richer presentation behavior remain outside
  this working context unless explicitly selected next.

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
- complete menu consequences of short/long input;
- debounce implementation;
- external buttons `EXT_1` and `EXT_2`;
- optimization of unrelated legacy code.

## Exit Criteria

This milestone is complete when:

- runtime contracts no longer depend on BSP, HAL, GPIO, LCD, ThreadX, queue, or
  timer types;
- semantic mechanical-key `SHORT` and `LONG` input can reach runtime under
  ThreadX without leaking BSP, GPIO, ThreadX, queue, or timer details into the
  product contract;
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
