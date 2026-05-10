# WORKING_CONTEXT.md

## Active Workstream

Stage:
- implementation

Active pass:
- refactor

Short name:
- FMC model and presentation refactor

Extended context:
- `docs/contexts/fmc_presentation_refactor.md`

## Scope Now

Active files/folders:
- `src/product/fmc/`
- `src/apps/fmc_model_units_test/`
- `src/libs/fm_status.h`
- `legacy_backup/libs/fm_fmc.*` as evidence only
- `legacy_backup/libs/fm_user.c` and `legacy_backup/libs/fm_setup.c` as
  behavior-discovery evidence only
- `docs/specs/fmc/`
- `src/bsp/devices/lcd/` as the already validated display foundation

Current target:
- finish the first pure FMC product slices before introducing runtime,
  presentation, or LCD adapter behavior.

## Current State

- `fmc_model.*` is implemented as canonical copyable FMC state plus structural
  helpers.
- `fmc_units.*` is implemented in the working tree as pure product unit policy.
- `fm_status.h` now owns common lightweight status codes for authored modules.
- `fmc_units.*` has been added to the common build sources.
- `fmc_model_units_test` now provides a repeatable app-level verification path
  for the pure model and unit-policy slices.
- Canonical builds passed for:
  - `main`
  - `template`
  - `panic_demo`
  - `lcd_bringup`
  - `lcd_blink_bringup`
  - `fmc_model_units_test`

## Decisions In Force

- FMC product modules live under `src/product/fmc/`, not `src/libs/`.
- FMC modules are product firmware modules, not portable flow-computer
  libraries.
- `fmc_model.*`, `fmc_units.*`, and early `fmc_rate.*` should remain pure where
  practical.
- RTOS ownership belongs later in `fmc_service.*` or `fmc_runtime.*`.
- LCD formatting and display writes belong later in presentation/adapter layers.
- `fmc_model.*` stores canonical state only:
  - measurement configuration
  - ACM/TTL pulse counters
  - reset policy and structural helpers
- Visible volume, operative factor views, and rate values are derived behavior,
  not stored truth in `fmc_model.*`.
- ACM and TTL are backed by pulse counters.
- TTL is resettable only through a privileged product flow; the model reset
  primitive itself does not authenticate callers.
- Calibration unit is explicit; current supported calculation path is liter
  calibration.
- `CUSTOM`, `KG`, and `EQUIV_M3` are valid 1:1 unit cases.
- Invalid/corrupt volume-unit enum values recover to liters.
- `BBL_US` is the model unit name; `BR` is a later presentation label.
- `FMC_MODEL_VOLUME_UNIT_EQUIV_M3` is the model name for equivalent cubic meter;
  `MC` is the later display label.
- Public FMC product symbols use `FMC_*`; filenames use `fmc_*`.

## Boundaries

Do not add to the current pure slices:
- pulse capture or interrupt acquisition
- RTOS mutexes, queues, timers, event flags, or task ownership
- UI/menu authorization
- LCD rendering or `FM_LCD_LL_*`
- Bluetooth, RTC, ticketing, backup persistence, or log layout
- direct ports from `legacy_backup/`
- fixed-point/scaled types as public semantics
- `flow_active` or `pulse_activity` as core model state without a new explicit
  requirement

Protected/generated-code policy remains in `AGENTS.md`.

## Next Step

1. Commit or preserve the accepted FMC model/units/test baseline.
2. Start the next pure slice: `fmc_rate.*` for rate calculation from
   pulse/time windows.
3. Keep `fmc_service.*` or `fmc_runtime.*` deferred until RTOS ownership and
   snapshot publication are explicit.

## References

- `AGENTS.md`
- `STYLE.md`
- `docs/contexts/fmc_presentation_refactor.md`
- `docs/specs/fmc/fm_fmc_legacy_field_inventory.md`
- `docs/specs/fmc/use_cases.yaml`
- `docs/specs/math/fm_numeric_library_candidate.md`
