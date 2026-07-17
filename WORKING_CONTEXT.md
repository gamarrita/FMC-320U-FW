# WORKING_CONTEXT.md

## Active Workstream

Stage:
- hardware bring-up validated

Short name:
- keyboard input bring-up

Extended context:
- `docs/contexts/keyboard_input_bringup.md`

## Scope Now

Active files/folders:
- `WORKING_CONTEXT.md`
- `docs/contexts/keyboard_input_bringup.md`
- `legacy/`
- `fmc-320u-v2.ioc`
- generated CubeMX GPIO/interrupt files for keyboard EXTI
- `src/port/`, `src/bsp/`, and `src/apps/bringups/` keyboard input files

Current target:
- close the short-press keyboard bring-up slice and choose the next integration
  step deliberately.

## Current State

- `display_format.*` is implemented, regression-tested, built, flashed, and
  visually validated through `bringups/display_format_lcd`.
- `bringups/display_format_lcd` now validates top and bottom LCD rows with
  UART-guided human inspection.
- `src/product/fmc/` currently contains validated pure model, units, rate, and
  volume calculation slices.
- `src/apps/product/main` is still a runnable placeholder/smoke app, not the
  real product runtime.
- Current hardware configuration is intentionally smaller than the legacy
  CubeMX project.
- `legacy/100_main.ioc` is tracked comparison evidence.
- `bringups/keyboard_input` is implemented and human-validated on hardware for
  the four configured short-press keys.
- The first keyboard slice intentionally covers short press only. Long press,
  timers, debounce firmware, menu navigation, and ThreadX queues remain
  deferred.

## Decisions In Force

- CubeMX is the source of truth for MCU hardware configuration.
- Do not edit `fmc-320u-v2.ioc` or generated hardware init manually.
- For new hardware-facing work, the human updates CubeMX first; the agent then
  adds repository wrappers such as `port/`, `bsp/`, apps, and tests.
- Treat legacy code and `legacy/100_main.ioc` as evidence, not authority.
- Hardware debounce is considered sufficient for the first keyboard slice.
- Do not add firmware debounce in the first keyboard slice.
- Do not implement long press before a runtime/timer decision is made.
- Keep the input event boundary RTOS-neutral so future ThreadX integration can
  replace only the event/timer backend.

## Selected Slice

See `docs/contexts/keyboard_input_bringup.md` for scope and rationale.

Validated goal:
- report short key events over UART from a bring-up app using the
  CubeMX-generated falling-edge EXTI path and `fm_debug`.

Validated events:
- `KEY_ESC`
- `KEY_ENTER`
- `KEY_UP`
- `KEY_DOWN`

## Boundaries

Do not add in this slice:
- `.ioc` edits
- generated code edits outside explicit `USER CODE`
- firmware debounce
- long press
- ThreadX enablement or ThreadX queues/timers
- menu navigation or product runtime orchestration
- LPTIM, USART3, DMA, flash-map, or other unrelated hardware integration
  before the corresponding CubeMX configuration has been made by the human
- broad product runtime orchestration

## Next Step

1. Choose the next slice from the remaining options:
   - product presentation semantics,
   - product/UI input integration,
   - ThreadX transition point,
   - pulse acquisition,
   - persistence,
   - communication/MXC.
2. If the next slice requires new hardware configuration, the human updates
   CubeMX first.
3. If the next slice expands keyboard behavior beyond short press, decide the
   ThreadX/input-service boundary before adding timers or queues.

## References

- `AGENTS.md`
- `STYLE.md`
- `docs/contexts/keyboard_input_bringup.md`
- `docs/contexts/next_fmc_slice_selection.md`
- `legacy/analysis/migration_ledger.md`
- `legacy/analysis/module_inventory.md`
