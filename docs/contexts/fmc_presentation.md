# FMC Presentation

## Purpose

This is the active extended context for the FMC presentation workstream.

`WORKING_CONTEXT.md` stays short and operational.
This file keeps the rationale and the few decisions needed to continue safely.

## Validated Base

Already implemented and treated as the current base:
- `src/product/fmc/fmc_model.*`
- `src/product/fmc/fmc_units.*`
- `src/product/fmc/fmc_rate.*`
- `src/apps/fmc_model_units_test/`

Current validation evidence:
- `fmc_model_units_test` passes on hardware for the pure model, unit, and rate
  slices
- `lcd_bringup` validates the static LCD foundation
- `lcd_blink_bringup` validates the current logical blink path

## Layer Ownership

The intended split is:

1. `fmc_model.*`
   - canonical FMC state
   - measurement configuration
   - ACM and TTL pulse-backed totals

2. `fmc_units.*`
   - unit validity and recovery
   - liters-per-unit policy
   - pulses-per-active-unit derivation

3. `fmc_rate.*`
   - pure rate calculation from pulse and elapsed-time windows

4. `fmc_presentation.*`
   - semantic display decisions
   - mode identity
   - legends
   - unit labels and cues
   - decimal policy
   - row responsibility

5. FMC-to-LCD adapter
   - translates presentation output to the validated LCD stack

6. `fmc_service.*` or `fmc_runtime.*`
   - future RTOS-facing owner of live FMC state and snapshots

## Presentation Decisions In Force

- `fmc_presentation.*` is not just for units.
- It should own high-level FMC formatting for the LCD-facing product view.
- LCD-specific segment mapping stays out of this module.
- Direct `FM_LCD_*` calls stay out of this module.
- The shared product view is still:
  - top row for total-oriented data
  - bottom row for rate-oriented data
- `BBL_US` is the model unit name; `BR` is only a presentation label.
- `FMC_MODEL_VOLUME_UNIT_EQUIV_M3` is the model name; `MC` is only a
  presentation label.

## LCD Facts Relevant Here

The presentation layer can assume the current validated LCD foundation exposes:
- one top numeric row with 8 positions
- one bottom numeric row with 7 positions
- one pair of 14-segment alphanumeric characters
- product legends and standalone indicators, including `TTL`, `ACM`, `RATE`,
  and time-base cues

That is enough for the presentation contract.
Detailed LCD redesign history is no longer active context.

## Open Design Work

Still to define explicitly:
- the presentation input shape
  - direct model-plus-derived fields
  - or a dedicated snapshot struct
- the presentation output shape
  - semantic fields
  - or LCD-ready but adapter-neutral fields
- the exact label policy for each unit
- the exact decimal policy for total and rate views
- whether mode selection is explicit or inferred from the requested screen

## Risks

- leaking LCD segment concerns into semantic presentation
- pulling runtime or RTOS ownership into a pure formatting slice
- duplicating unit policy that already belongs in `fmc_units.*`
- freezing labels before the contract shape is clear

## Near-Term Goal

Define and implement the smallest useful `fmc_presentation.*` contract that:
- consumes validated FMC semantics
- produces stable presentation semantics
- stays pure
- can be tested before any LCD adapter exists
