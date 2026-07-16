# WORKING_CONTEXT.md

## Active Workstream

Stage:
- analysis

Short name:
- FMC volume and display-facing formatting

Extended context:
- `docs/contexts/fmc_presentation.md`

## Scope Now

Active files/folders:
- `src/product/fmc/`
- `src/apps/fmc_model_units_test/`
- `src/apps/display_format_lcd_bringup/`
- `docs/specs/fmc/`
- `src/bsp/devices/lcd/`
- `src/services/` as the likely home for reusable display formatting helpers

Current target:
- validate the reusable `display_format.*` helper through UART tests and LCD
  human-observed bring-up.

## Current State

- `fmc_model.*`, `fmc_units.*`, and `fmc_rate.*` are implemented.
- `fmc_model_units_test` is the current regression harness for those pure FMC
  slices and `display_format.*`.
- `display_format_lcd_bringup` validates `display_format.*` output on the
  physical LCD with UART comparison lines.
- `lcd_bringup` and `lcd_blink_bringup` remain the validated LCD foundation.
- The canonical legacy source is now available under `legacy/source/` for
  evidence.
- `docs/specs/fmc/presentation_screens.md` captures user-visible screen
  behavior as normalized evidence.

## Decisions In Force

- The model/unit/rate slices should not calculate display formatting or write
  LCD output.
- Visible ACM/TTL volume calculation exists as a pure FMC slice.
- LCD BSP should remain a hardware/display foundation, not a product-formatting
  layer.
- Reusable numeric/display-field formatting belongs above BSP, under
  `src/services/`, so it can support quick LCD prototypes without knowing FMC
  semantics.
- `fmc_presentation.*` remains a later candidate, not the assumed immediate
  next module.
- LCD adapters and bring-up apps remain later separate slices.
- RTOS/runtime ownership remains deferred to `fmc_service.*` or
  `fmc_runtime.*`.

## Boundaries

Do not add in this slice:
- direct LCD writes
- RTOS synchronization or task ownership
- persistence, menus, or authorization flows
- pulse capture or interrupt acquisition
- direct ports from `legacy/source/`
- a full `fmc_presentation.*` module before volume and formatting boundaries
  are clear

## Next Step

1. Run `display_format_lcd_bringup` on hardware and compare UART `TOP=` lines
   with the LCD top row.
2. Confirm whether the zero-padded decimal cases match product expectations.
3. Keep `fmc_presentation.*` deferred until the volume and display-format
   boundaries are explicit.

## References

- `AGENTS.md`
- `STYLE.md`
- `docs/contexts/fmc_presentation.md`
- `docs/specs/fmc/fm_fmc_legacy_field_inventory.md`
- `docs/specs/fmc/use_cases.yaml`
- `docs/specs/fmc/presentation_screens.md`
- `legacy/source/libs/fm_fmc.*`
- `legacy/source/FLOWMEET/fm_user.*`
