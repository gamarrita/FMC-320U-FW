# FMC Presentation Refactor

## Purpose

This workstream extracts the behavior that the FMC-320U flow computer must
model and present, then rebuilds that behavior as cleaner product layers above
the validated LCD stack.

The goal is not to port the old FMC module directly. Legacy code is evidence,
not authority; inherited behavior survives only when it remains a deliberate
current product decision.

This file is the extended context for `WORKING_CONTEXT.md`. It keeps rationale,
decisions, history, risks, and backlog. Immediate execution state belongs in
`WORKING_CONTEXT.md`.

## Active Summary

Implemented in the active working tree:
- `src/product/fmc/fmc_model.*`
- `src/product/fmc/fmc_units.*`
- `src/libs/fm_status.h`
- `src/apps/fmc_model_units_test/`

Current pure slices:
- `fmc_model.*`: canonical copyable FMC state and structural helpers.
- `fmc_units.*`: product unit policy and operative pulses-per-active-unit
  calculation.
- `fmc_model_units_test`: repeatable app-level verification for the pure
  model and unit-policy boundary.

Not implemented yet:
- `fmc_rate.*`
- `fmc_service.*` or `fmc_runtime.*`
- `fmc_config.*`
- `fmc_presentation.*`
- FMC-to-LCD adapter

Current validation:
- canonical builds passed for `main`, `template`, `panic_demo`,
  `lcd_bringup`, `lcd_blink_bringup`, and `fmc_model_units_test`.

## Source Evidence

Legacy evidence:
- `legacy_backup/libs/fm_fmc.c`
- `legacy_backup/libs/fm_fmc.h`
- `legacy_backup/libs/fm_user.c`
- `legacy_backup/libs/fm_setup.c`

Normalized product/spec evidence:
- `docs/specs/fmc/fm_fmc_legacy_field_inventory.md`
- `docs/specs/fmc/use_cases.yaml`
- `docs/specs/math/fm_numeric_library_candidate.md`

Validated display foundation:
- `src/bsp/devices/lcd/`
- `docs/contexts/archive/lcd_stack_closed/lcd_redesign.md`

Legacy modules are used only to discover concepts, naming collisions, hidden
couplings, and behavior worth deliberately preserving.

## Layer Ownership

The intended FMC module family is:

1. `fmc_model.*`
   - owns canonical FMC state
   - owns total roles and reset policy
   - stores pulse counters behind ACM/TTL
   - stays copyable and pure
   - does not own RTOS synchronization, LCD formatting, persistence, rate
     calculation, or visible-volume queries

2. `fmc_units.*`
   - owns product unit validity
   - owns invalid/corrupt unit recovery
   - owns liters-per-unit policy
   - owns 1:1 unit classification
   - calculates operative pulses per active unit from model measurement state
   - does not own labels, LCD strings, persistence, or RTOS state

3. `fmc_rate.*`
   - future pure helper for rate calculation from pulse/time windows
   - should not own acquisition interrupts or RTOS scheduling

4. `fmc_service.*` or `fmc_runtime.*`
   - future RTOS-facing owner of live FMC state
   - owns synchronization, queues/event flags/timers if needed
   - receives acquisition updates or snapshots
   - publishes coherent snapshots to UI, logging, persistence, and presentation

5. `fmc_config.*`
   - future product configuration application layer
   - applies calibration, active unit, and time base
   - may later consume storage/persistence services

6. `fmc_presentation.*`
   - future semantic presentation layer
   - decides row roles, legends, unit cues, decimal policy, and mode identity
   - does not call the LCD low-level API directly

7. FMC-to-LCD adapter
   - future adapter that consumes presentation output and calls the validated
     LCD stack

Validation app:
- `fmc_model_units_test` exercises only the current pure FMC model and unit
  policy boundary, then reports PASS/FAIL through the normal debug app
  infrastructure.
- It is a verification harness, not a product runtime or presentation layer.

## Decisions In Force

### Product And RTOS Direction

- FMC modules under `src/product/fmc/` are product firmware modules, not generic
  portable libraries.
- Product specificity does not mean every module should include RTOS primitives.
- `model`, `units`, and early `rate` code should remain pure where practical.
- RTOS ownership is deferred to `fmc_service.*` or `fmc_runtime.*`.

### Naming

- Product FMC filenames use `fmc_*`.
- Public C symbols use the `FMC_*` module namespace.
- The old `fm_fmc_*` shape is not used for the new product module family.

### Canonical State

The core model stores:
- measurement configuration
- explicit calibration unit
- active volume unit
- active rate time base
- ACM pulse counter
- TTL pulse counter
- total-role reset policy and structural helpers

The core model does not store:
- visible volume
- operative factor as canonical truth
- instantaneous rate
- decimal placement
- LCD labels
- activity indicators
- RTOS-owned live singleton state

### Totals

- `ACM` and `TTL` are first-class FMC total roles.
- Both are backed by pulse counters.
- `ACM` is resettable in normal user operation.
- `TTL` is not user-resettable in normal operation.
- `TTL` can still be reset through a privileged product flow.
- The model reset primitive does not authenticate; UI/service flow protects
  privileged access.

### Units And Calibration

- Calibration unit is explicit in the model.
- The current supported calculation path is calibration in liters.
- No menu, display, or operator-facing calibration-unit change is implemented
  in this slice.
- The product editable calibration range remains `1.000` to `99999.999`.
- That range is semantic validation, not a reason to leak fixed-point types into
  the public contract.
- `BBL_US` is the model unit; `BR` is a later presentation string.
- `FMC_MODEL_VOLUME_UNIT_EQUIV_M3` represents equivalent cubic meter; `MC` is
  the later display label.
- Normal `M3` remains separate and uses physical conversion:
  `1 m3 = 1000 L`.
- Equivalent cubic meter uses the product relation:
  `1 L = 1 equivalent m3`.

### 1:1 Units

`CUSTOM`, `KG`, and `EQUIV_M3` are valid 1:1 unit cases inside the flow
computer:
- conversion is performed outside the computer by the technician/configuration
  process
- the loaded calibration is already expressed as pulses per desired active unit
- `CUSTOM` has no firmware label path and may be rendered as `--`
- invalid/corrupt enum values are not `CUSTOM`; they recover to liters

### Derived Behavior

These are derived views, not canonical state:
- visible ACM volume
- visible TTL volume
- operative pulses per active unit
- instantaneous rate
- optimized derived factors

They should be introduced through unit/rate/runtime/presentation helpers rather
than as stored truth inside `fmc_model.*`.

## Legacy Findings To Preserve

Meaningful concepts:
- ACM and TTL as separate total roles
- rate as a first-class instrument view
- pulse-to-quantity conversion through calibration and unit policy
- shared volume unit for ACM, TTL, and RATE
- separate time base for RATE
- reset semantics as product behavior
- decimal placement as presentation policy

Legacy couplings to avoid:
- direct `FM_LCD_LL_*` calls inside FMC semantics
- display rows mixed into calculation code
- `ufp3_t` as a semantic API boundary
- canonical and derived values stored together without ownership clarity
- persistence/debug assumptions mixed into presentation and math
- treating `fm_user.*` or `fm_setup.*` as semantic sources of truth

## Presentation Semantics To Define Later

The future presentation layer should own:
- display mode:
  - `TTL/RATE`
  - `ACM/RATE`
- row responsibility:
  - top row = total view
  - bottom row = rate view
- decimal policy:
  - total decimals
  - rate decimals
- unit cue policy:
  - active volume-unit alpha pair
  - slash indicator between unit and time base
  - active time-base indicator: `S`, `M`, `H`, or `D`
- legend policy:
  - `TTL`
  - `ACM`
  - `RATE`
- optional future pulse/activity cue

The LCD adapter should be separate from these semantic decisions.

## Numeric Representation

Scaled or fixed-point arithmetic may remain a valid embedded implementation
strategy.

The public semantic boundary should still speak in product meanings:
- pulses
- totals
- rate
- units
- calibration
- time base
- decimals
- reset policy

Numeric representation should remain an implementation detail unless a later
constraint proves it must be part of the contract.

## External Reference Signals

Vendor flow-meter references support the split between low-level measurement
mechanics, calibration constants, and higher-level application behavior:
- TI ultrasonic flow-meter material separates meter constants and measurement
  algorithms.
- ST metering examples separate counting, tachometer/speed, direction/error
  detection, and application-level metering.

These signals support the local direction:
- model FMC elements first
- derive engineering views through helpers
- connect presentation only after the semantics are stable
- keep RTOS ownership out of pure calculation/model modules

## Open Questions

- The RTOS-facing owner name is not frozen:
  - `fmc_service.*`
  - `fmc_runtime.*`
- The future public shape of snapshots is not frozen:
  - one domain snapshot struct
  - or granular typed semantic fields
- It is still open whether ACM/TTL remain public API vocabulary forever or
  become product aliases over more generic total roles.
- Alpha mapping for volume units in the new LCD stack still needs an explicit
  decision.

## Risks

- dragging legacy runtime/UI coupling into pure FMC modules
- preserving legacy names without preserving real product meaning
- making the core too generic and losing flow-computer semantics
- freezing `ufp3_t` or derived factors into the wrong public layer
- letting RTOS ownership leak into `fmc_model.*`, `fmc_units.*`, or early
  `fmc_rate.*`
- scattering product decisions between short context, long context, specs, and
  README files without one clear owner

## Roadmap

1. Preserve the current `fmc_model.*`, `fmc_units.*`, `fm_status.h`, validation
   app, and documentation baseline.
2. Start the next pure slice: `fmc_rate.*`.
3. Keep derived behavior outside the model:
   - visible volume
   - operative factor views
   - instantaneous rate
4. Define the RTOS-facing owner:
   - live model instance
   - synchronization
   - acquisition updates
   - snapshot publication
5. Define presentation semantics without LCD calls.
6. Add the FMC-to-LCD adapter over the validated LCD stack.
