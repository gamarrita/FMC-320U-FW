# WORKING_CONTEXT.md

## Active Workstream

Stage:
- minimal runtime contract micro-slice implemented

Short name:
- FMC service/runtime boundary

Extended context:
- `docs/contexts/fmc_runtime_boundary.md`

## Scope Now

Active files/folders:
- `WORKING_CONTEXT.md`
- `docs/contexts/fmc_runtime_boundary.md`
- `legacy/`
- `fmc-320u-v2.ioc`
- `src/product/fmc/`
- `src/port/`, `src/bsp/`, and `src/apps/`

Current target:
- define the smallest useful `fmc_service` / `fmc_runtime` boundary so future
  RTOS, low-power, input, acquisition, persistence, and presentation work has a
  clear place to connect. The first cut, `fmc_service.*`, is closed. The second
  cut, minimal RTOS-neutral `fmc_runtime.*` event dispatch, is ready for review.

## Current State

- Validated base: `fmc_model.*`, `fmc_units.*`, `fmc_rate.*`,
  `fmc_volume.*`, `display_format.*`, LCD bring-ups, debug UART/LEDs, and
  keyboard short-press bring-up.
- `fmc_service.*` now owns live FMC state, pulse-delta updates, total reset
  forwarding, and snapshots with derived ACM/TTL visible volume.
- `fmc_runtime.*` now owns one `fmc_service_t`, accepts minimal product events,
  and tracks pending presentation updates without owning keyboard, LCD, HAL,
  queues, or RTOS primitives.
- `src/apps/product/main` is still a runnable placeholder/smoke app, not the
  real product runtime.
- Current hardware configuration is smaller than legacy; use
  `legacy/100_main.ioc` only as comparison evidence.
- Next active task: review the minimal runtime contract and choose the next
  micro-slice deliberately.

## Decisions In Force

- CubeMX is the source of truth for MCU hardware configuration.
- Do not edit `fmc-320u-v2.ioc` or generated hardware init manually.
- For new hardware-facing work, the human updates CubeMX first; the agent then
  adds repository wrappers such as `port/`, `bsp/`, apps, and tests.
- Treat legacy code and `legacy/100_main.ioc` as evidence, not authority.
- Keep input/runtime boundaries RTOS-neutral until the ThreadX transition is
  selected deliberately.

## Last Closed Slice

FMC runtime minimal event-dispatch contract is implemented and ready for
review. FMC service state/snapshot contract is closed as the first boundary
sub-slice. Keyboard short-press bring-up was closed previously.

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

1. Review whether `fmc_runtime` should next receive keyboard-originated product
   events, acquisition-originated pulse windows, or a product-app smoke wiring.
2. Keep the next change to one or two small decisions so human/agent review can
   steer the boundary deliberately.

## References

- `AGENTS.md`
- `STYLE.md`
- `docs/contexts/fmc_runtime_boundary.md`
- `legacy/analysis/migration_ledger.md`
- `legacy/analysis/module_inventory.md`
