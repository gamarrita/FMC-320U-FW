# Working Context: FMC Phase 6 Presentation And UI State Machine

## Active Workstream

Implement the first product-visible presentation and UI state-machine slice for
`product/main`.

Product behavior authority:
- `docs/specs/fmc/use_cases.yaml`

Durable phase strategy:
- `docs/roadmaps/fmc_refactoring.md`, Phase 6

Operational references:
- `AGENTS.md`
- `STYLE.md`
- `src/apps/product/main/README.md`
- `src/product/fmc/README.md`
- `docs/workflow/doc_closure.md`

## Current Baseline

Phase 5 is closed for the selected input/runtime slice:
- the existing `FM_APP` ThreadX thread runs `FM_MAIN_Main()`;
- `FM_MAIN_Main()` is the only owner of the live `fmc_runtime_t`;
- keyboard ISR callbacks, key-hold timeout, and periodic refresh are serialized
  through the product/main owner queue;
- mechanical `SHORT` and `LONG` input is hardware-smoke validated for `DOWN`,
  `UP`, `ENTER`, and `ESC`;
- the periodic refresh event wakes the owner loop every second and is currently
  a no-op placeholder reserved for measurement and presentation work.

## Milestone Outcome

The firmware should gain one narrow, auditable user-visible LCD path driven by
the existing owner loop:
- screen state is owned by `product/main` or by a small app-level presentation
  helper called only from that owner loop;
- rendering uses board/LCD contracts and existing formatting helpers;
- `src/product/fmc` remains independent of ThreadX, HAL, GPIO, queues, timers,
  BSP, and LCD details;
- behavior implemented in the slice is traceable to
  `docs/specs/fmc/use_cases.yaml`;
- unresolved product behavior is reported instead of guessed.

## Selected First Slice

Recommended first implementation target:
1. `SCREEN_STARTUP_ALL_SEGMENTS` for 3 seconds.
2. `SCREEN_FIRMWARE_VERSION` for 3 seconds.
3. `SCREEN_TTL_RATE` as the steady user screen.

Within this slice:
- `ESC SHORT` may advance/skip startup screens when specified by the product
  spec;
- other key consequences are implemented only where the selected spec section
  is confirmed;
- the existing periodic refresh event may drive presentation refresh;
- timer callbacks publish bounded owner-queue events only;
- LCD writes happen from the owner loop, not from ISR or timer callbacks.

Before coding this slice, audit whether the existing LCD/BSP API can express:
- all visible segments on;
- firmware version with the `VE` legend;
- TTL/RATE rows and indicators needed by `SCREEN_TTL_RATE`;
- any required backlight operation already exposed through board contracts.

If a needed operation is missing from the public LCD or board contract, stop and
report the smallest contract extension instead of writing through HAL, GPIO,
LCD RAM internals, or generated code.

## Decisions In Force

- Do not create a second product thread without a concrete concurrent
  responsibility that cannot live in the owner loop cleanly.
- Keep startup/screen timing serialized through owner-loop events.
- Keep public contracts documented in headers, not in this context file.
- Use `display_format` only for numeric/text formatting; it must not become the
  owner of screen state or LCD hardware writes.
- Treat LCD segment mapping gaps, numeric overflow/invalid display,
  wake/backlight policy, debounce, alarms, config editing, external buttons,
  printer, Bluetooth, and optional PT100 behavior as deferred unless the user
  selects them explicitly.
- Preserve CubeMX as hardware configuration source of truth. Do not edit
  protected or generated hardware configuration paths without explicit human
  approval.

## Audit Gates For The Next Agent

Before implementation:
- identify the exact `use_cases.yaml` sections used by the selected slice;
- classify each required behavior as confirmed, inferred, or unresolved;
- list the public LCD/board/runtime contracts that will be consumed;
- state whether any new public contract is required.

During implementation:
- keep changes small and reviewable;
- keep `fmc_runtime_t` ownership in `FM_MAIN_Main()`;
- keep ISR and timer callbacks bounded and non-blocking;
- avoid adding persistent UI abstractions until the first slice proves the
  shape.

Verification expected for code changes:
- focused tests for selected screen-state transitions and formatting decisions;
- canonical build flow from `docs/canonical-build/stm32cube-cli-workflow.md`
  when buildability or runtime behavior changes;
- hardware smoke for selected visible LCD behavior, key skip behavior, and
  continued periodic wake behavior when the slice reaches the board.

## Out Of Scope For This Context

- full user menu navigation beyond the selected slice;
- configuration menu behavior;
- persistence, RTC editing, pulse acquisition, real rate updates, alarms,
  printer, Bluetooth, optional PT100, or release validation;
- redesigning the LCD stack or recreating a broad presentation design document.

## Closure Criteria

This context can close when:
- the selected Phase 6 slice is implemented or deliberately deferred with a
  recorded reason;
- implemented behavior is traceable to `use_cases.yaml`;
- ownership and boundaries are reflected in the relevant header or README when
  they become implemented facts;
- tests/build/smoke evidence for the slice is recorded in the final report or
  durable docs where appropriate;
- `docs/workflow/doc_closure.md` has been applied.
