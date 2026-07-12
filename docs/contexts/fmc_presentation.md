# FMC Volume And Display-Facing Formatting

## Purpose

This is the active extended context for the FMC volume and display-facing
formatting workstream.

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

4. `fmc_volume.*`
   - future pure visible-volume calculation for ACM and TTL
   - consumes pulse-backed totals plus measurement configuration
   - does not format strings or write LCD output

5. Reusable display-format helper
   - future technical service above the LCD BSP
   - formats bounded numeric/display fields
   - detects overflow/invalid states
   - stays independent of FMC semantics

6. FMC operation view or `fmc_presentation.*`
   - future product semantic display layer
   - decides mode identity, row roles, legends, unit cues, and decimal policy
   - should not be implemented before volume and display-format boundaries are
     clear

7. FMC-to-LCD adapter
   - future adapter that translates product/display semantics to the validated
     LCD stack

8. `fmc_service.*` or `fmc_runtime.*`
   - future RTOS-facing owner of live FMC state and snapshots

## Decisions In Force

- The next implementation candidate is `fmc_volume.*`, not
  `fmc_presentation.*`.
- `fmc_volume.*` should calculate visible ACM/TTL volumes from canonical
  pulse counters and measurement configuration.
- `fmc_volume.*` should not format strings, own decimals, write LCD output, or
  know keyboard/RTOS flow.
- The LCD BSP should stay focused on physical/custom LCD capabilities.
- A reusable display-format helper above BSP is likely needed for bounded
  numeric fields, fixed decimals, alignment, and overflow states.
- That helper likely belongs under `src/services/`, not under
  `src/bsp/devices/lcd/` and not under `src/product/fmc/`.
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
- the `fmc_volume.*` public contract
  - calculate one selected ACM/TTL volume
  - or calculate ACM and TTL together
  - input as model plus total role
  - or input as total state plus measurement
- the numeric representation returned by volume calculation
  - `double`
  - scaled integer
  - or a small structured quantity
- where decimal selection belongs
  - FMC product config
  - display-format helper
  - or future presentation layer
- the reusable display-format helper contract
  - numeric input type
  - width
  - fractional digits
  - rounding rule
  - overflow/invalid status
- whether an intermediate FMC operation view should exist before a broader
  `fmc_presentation.*` module

## Risks

- letting presentation calculate volumes because `fmc_volume.*` is missing
- putting product formatting rules into the LCD BSP
- creating a broad `fmc_presentation.*` module before the smaller boundaries are
  understood
- pulling runtime, keyboard, or RTOS ownership into pure calculation or
  formatting modules
- duplicating unit policy already owned by `fmc_units.*`

## Near-Term Goal

Define the smallest useful `fmc_volume.*` contract and decide whether a generic
display-format helper is required before product presentation work.

The first useful implementation should:
- consume validated FMC model/unit semantics
- calculate visible ACM/TTL volumes
- stays pure
- can be tested before any LCD adapter or UI flow exists
