# WORKING_CONTEXT.md

## Current Work

Stage:
- implementation

Active pass:
- refactor

Operational context:

Area:
- `legacy_backup/libs/fm_fmc.c`
- `legacy_backup/libs/fm_fmc.h`
- `legacy_backup/libs/fm_user.c`
- `legacy_backup/libs/fm_setup.c`
- `src/product/fmc/`
- `src/product/fmc/fmc_model.h`
- `src/bsp/devices/lcd/`

Core structure:
- `legacy_backup/libs/fm_fmc.*` is legacy behavior reference, not a direct
  port target
- `legacy_backup/` is frozen reference evidence; normalized product specs live
  under `docs/specs/`
- the closed LCD stack under `src/bsp/devices/lcd/` is the display foundation to
  preserve
- FMC product-domain modules live under `src/product/fmc/`, not `src/libs/`
- FMC product modules are not generic portable libraries; they may assume this
  product, IDE, and chosen RTOS direction
- this is a learning-oriented product refactor:
  - legacy is evidence, not authority
  - inherited behavior is preserved only when it is a deliberate current product
    decision
- this refactor now targets:
  - an FMC model layer
  - later pure unit and rate helpers where useful
  - later an RTOS-facing FMC service/runtime owner
  - then an FMC presentation-semantics layer above it
  - then a later LCD adapter over the validated LCD stack
- the first slice excludes pulse math, capture logic, and legacy UI flow

Current focus:
- extract the FMC semantic elements that the instrument actually exposes:
  ACM, TTL, RATE, pulse counts, K/calibration, units, and time base
- keep the first `src/product/fmc/fmc_model.h` contract limited to canonical
  model state and structural helpers
- `fmc_model.*` first slice is implemented and compiled as common product code
- avoid adding unit/rate/runtime behavior to `fmc_model.*`
- separate:
  - FMC model semantics
  - FMC unit/rate calculation policy
  - FMC runtime/service ownership
  - FMC presentation semantics
  - LCD adapter concerns
- incorporate the latest domain decisions:
  - TTL is resettable, but by a more privileged mechanism than ACM
  - TTL remains non-user-resettable in normal operation
  - ACM and TTL should stay simple and share the same volume-unit ownership
  - RATE should share the same volume unit and only vary by time base
  - pulse counters remain relevant as the backing state behind totals
  - ACM and TTL store canonical pulse counters; visible volumes are derived
  - calibration unit is explicit in the model; normal physical conversions use
    the liter default, while custom units use a factor already loaded for that
    unit
  - unsupported/custom units use a model placeholder, conversion factor 1, and
    calibration already loaded for the desired custom unit
  - `BBL_US` is the model unit; `BR` is a later presentation string
  - `fmc_model.*` stores canonical state only; visible volume and operative
    pulses-per-active-unit views belong to later unit/rate/presentation helpers
  - FMC product modules under `src/product/fmc/` use `fmc_*` filenames and
    `FMC_*` public symbol prefixes, avoiding the redundant `fm_fmc_*` prefix
- clarify semantic assumptions before accepting any more contract:
  - visible ACM, TTL, and RATE are derived views, not editable configuration
  - ACM and TTL backing state remains canonical pulse counters
  - configuration and runtime ownership must be reviewed explicitly
  - interpretation problems found in the header should be treated as blockers,
    not cosmetic issues
- use the working loop:
  - inspect one legacy item
  - ask until intent and ownership are clear
  - record the clarified meaning in the working artifacts
  - only then move to the next item
- keep the first slice oriented to a flow-computer instrument while improving
  semantics and coherence instead of copying legacy structure or locking the
  contract to `ufp3_t`

Constraints:
- do not port `legacy_backup/` modules directly into `src/product/fmc/` or `src/libs/`
- do not place FMC product-domain modules in `src/libs/`
- do not duplicate normalized specs under `legacy_backup/`
- do not include pulse/capture math in the first slice
- do not put RTOS ownership, mutexes, queues, timers, or live shared-state
  service behavior in `fmc_model.*`
- do not pull in `FM_LCD_LL_*`, setup screens, user menu flow, Bluetooth, RTC,
  ticketing, or backup persistence
- use `legacy_backup/libs/fm_fmc.*`, `fm_user.c`, and `fm_setup.c` only as
  references for domain discovery and contrast, not as semantics to preserve
- use scaled/fixed-point only if it remains an implementation detail, not as
  the semantic boundary of the new module
- do not keep `flow_active` or `pulse_activity` as core model state unless a
  later requirement proves they are first-class FMC elements
- do not keep a legacy name, shape, or behavior only because it exists; keep it
  only when it is the best current product decision
- keep the model shape copyable so the product can later hold:
  - active environment
  - edit buffer
  - factory/default copies

## Next Step

- review and commit the `fmc_model.*` implementation if accepted
- then decide the next FMC slice:
  - `fmc_units.*` for unit conversion and operative factor behavior
  - or `fmc_rate.*` for rate calculation from pulse/time windows
  - keep `fmc_service.*` until RTOS ownership is ready to be modeled
- keep future slices pure and separate until their boundary is explicit:
  - no RTOS ownership in model/units/rate helpers
  - no LCD/presentation code in model/units/rate helpers
  - no persistence or log layout in the model
- use these references while implementing:
  - `docs/specs/fmc/fm_fmc_legacy_field_inventory.md`
  - `docs/specs/fmc/use_cases.yaml`
  - `docs/specs/math/fm_numeric_library_candidate.md`

## References

- `AGENTS.md`
- `docs/contexts/fmc_presentation_refactor.md`
- `docs/specs/fmc/use_cases.yaml`
- `docs/specs/fmc/fm_fmc_legacy_field_inventory.md`
- `docs/specs/math/fm_numeric_library_candidate.md`
- `STYLE.md`

## Invocation Rule

Every time this file is used:
- confirm stage, active pass, current focus, constraints, and next step
- for immediate execution, `Constraints` and `Next Step` are authoritative
- use `legacy_backup/` only as behavior reference, not as direct architecture
- if the real work drifted, update this file and its referenced
  `docs/contexts/` file before continuing

## Maintenance Rule

Keep this file updated when any of these change:
- stage
- active pass
- operational focus
- constraints
- next step
- referenced active context file

Keep the referenced file under `docs/contexts/` updated when any of these
change:
- extended technical rationale
- detailed scope
- validated or pending state
- remaining work
- active technical decisions

Update both together when:
- the short execution state and the extended context would otherwise diverge
- the active line of work changes materially

## Strong Rule

This is execution state, not documentation.
No history.
No long explanation.
Detailed rationale stays in `docs/contexts/fmc_presentation_refactor.md`.
Keep `WORKING_CONTEXT.md` and its referenced `docs/contexts/` file aligned.
Do not let this file absorb extended design detail that belongs in the
referenced context file.
