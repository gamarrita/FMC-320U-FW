# WORKING_CONTEXT.md

## Active Workstream

Stage:
- selected next refactor slice

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
  clear place to connect.

## Current State

- Validated base: `fmc_model.*`, `fmc_units.*`, `fmc_rate.*`,
  `fmc_volume.*`, `display_format.*`, LCD bring-ups, debug UART/LEDs, and
  keyboard short-press bring-up.
- `src/apps/product/main` is still a runnable placeholder/smoke app, not the
  real product runtime.
- Current hardware configuration is smaller than legacy; use
  `legacy/100_main.ioc` only as comparison evidence.
- Next active task: define the FMC service/runtime boundary.

## Decisions In Force

- CubeMX is the source of truth for MCU hardware configuration.
- Do not edit `fmc-320u-v2.ioc` or generated hardware init manually.
- For new hardware-facing work, the human updates CubeMX first; the agent then
  adds repository wrappers such as `port/`, `bsp/`, apps, and tests.
- Treat legacy code and `legacy/100_main.ioc` as evidence, not authority.
- Keep input/runtime boundaries RTOS-neutral until the ThreadX transition is
  selected deliberately.

## Last Closed Slice

Keyboard short-press bring-up is closed for now. It reports
`KEY_ESC`, `KEY_ENTER`, `KEY_UP`, and `KEY_DOWN` over UART.

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

1. Choose the first sub-slice inside the FMC service/runtime boundary:
   - pure service state/snapshot contract,
   - runtime event loop skeleton,
   - ThreadX/low-power risk study.
2. Keep the first implementation small enough to preserve agent-assisted
   development clarity.
3. If the chosen sub-slice requires CubeMX, ThreadX, or low-power changes,
   make that decision explicit before editing generated configuration.

## References

- `AGENTS.md`
- `STYLE.md`
- `docs/contexts/fmc_runtime_boundary.md`
- `legacy/analysis/migration_ledger.md`
- `legacy/analysis/module_inventory.md`
