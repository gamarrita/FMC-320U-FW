# WORKING_CONTEXT.md

## Active Workstream

Stage:
- analysis

Short name:
- FMC presentation

Extended context:
- `docs/contexts/fmc_presentation.md`

## Scope Now

Active files/folders:
- `src/product/fmc/`
- `src/apps/fmc_model_units_test/`
- `docs/specs/fmc/`
- `src/bsp/devices/lcd/`

Current target:
- define the first `fmc_presentation.*` slice as a pure semantic layer above
  the validated FMC model/unit/rate code and below any LCD adapter.

## Current State

- `fmc_model.*`, `fmc_units.*`, and `fmc_rate.*` are implemented.
- `fmc_model_units_test` is the current regression harness for those pure FMC
  slices.
- `lcd_bringup` and `lcd_blink_bringup` remain the validated LCD foundation.

## Decisions In Force

- `fmc_presentation.*` owns semantic presentation decisions:
  - mode identity
  - row roles
  - legend selection
  - unit cues
  - decimal policy
- `fmc_presentation.*` does not call the LCD API directly.
- The LCD adapter remains a later separate slice.
- RTOS/runtime ownership remains deferred to `fmc_service.*` or
  `fmc_runtime.*`.

## Boundaries

Do not add in this slice:
- direct LCD writes
- RTOS synchronization or task ownership
- persistence, menus, or authorization flows
- pulse capture or interrupt acquisition
- direct ports from `legacy/source/`

## Next Step

1. Freeze the presentation contract: inputs, outputs, and ownership.
2. Implement only the semantic layer that can later feed an LCD adapter.

## References

- `AGENTS.md`
- `STYLE.md`
- `docs/contexts/fmc_presentation.md`
- `docs/specs/fmc/fm_fmc_legacy_field_inventory.md`
- `docs/specs/fmc/use_cases.yaml`
