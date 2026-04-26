# FMC Presentation Refactor

## Purpose

This workstream plans a refactor focused on FMC semantics and presentation for
the new firmware stack.

The goal is not to port the old FMC module directly.
The goal is to extract the behavior that a flow-computer instrument needs to
model and present:
- ACM volume
- TTL volume
- RATE
- pulse counters
- calibration and operative factor behavior
- volume units
- rate time base
- reset semantics
- scaled values
- decimal placement
- row responsibilities
- unit cues
- alpha usage

and rebuild that behavior as cleaner layers above the validated LCD stack.

This workstream is also intentionally educational in the product-engineering
sense: legacy code is evidence, not authority. A legacy name, shape, or behavior
should survive only when it remains the best current product decision, not
because it already exists.

Important clarification for this refactor:
- `fm_user.*` and `fm_setup.*` are not semantic sources of truth
- all of those modules are expected to be rewritten
- legacy code is used only to discover domain concepts, naming collisions, and
  accidental couplings that should not survive

---

## Current State

- the legacy FMC behavior reference lives in:
  - `legacy_backup/libs/fm_fmc.c`
  - `legacy_backup/libs/fm_fmc.h`
- the legacy consumers that reveal real display usage live in:
  - `legacy_backup/libs/fm_user.c`
  - `legacy_backup/libs/fm_setup.c`
- the current repo already has a validated LCD stack under:
  - `src/bsp/devices/lcd/`
- the requested first slice is presentation-semantics only
- pulse accumulation, capture timing, and flow math are intentionally out of
  this first slice
- the latest clarification is that the refactor should not start from LCD
  writes; it should first define FMC elements and only later connect them to
  LCD through another module
- the latest clarification is also that legacy user/setup display semantics do
  not need to be preserved
- the current working method is now an explicit clarification loop:
  - review one legacy item at a time
  - ask until intended meaning and ownership are clear
  - treat unresolved interpretation as a blocker for contract refinement
  - only then advance to the next item
- the current first `fmc_model.h` contract is narrow enough to implement:
  - canonical measurement configuration
  - ACM/TTL backing pulse counters
  - total-role reset policy
  - structural access helpers
  - no visible volume, operative factor, rate, presentation, or RTOS ownership

Observed legacy split today:
- `fm_fmc.*` mixes:
  - FMC runtime state
  - factor and rate math
  - fixed-point formatting
  - unit labeling
  - direct LCD LL writes
  - persistence/debug assumptions
- `fm_user.c` shows how ACM, TTL, RATE, decimal selectors, and unit/time cues
  are actually presented to the operator
- `fm_setup.c` shows how configuration-facing displays present factors, units,
  and decimal edits

Observed semantic drift in the legacy:
- the module stores canonical and derived values together:
  - `factor_cal` as pulses per liter
  - `factor_k` derived for the active volume unit
  - `factor_r` derived for the active time base
- display semantics and LCD writes are mixed in the same module
- `ufp3_t` is used both as a storage strategy and as a semantic API surface
- ACM and TTL are meaningful product concepts, but their reset behavior is
  implicit in the code instead of modeled as an explicit contract
- unit handling is not uniformly physical:
  - normal physical units derive from the liter-based conversion table
  - `KG` was handled as a factory-calibrated special case without density
  - one custom/client unit path was effectively treated as another 1:1
    calibrated special case

---

## Scope

In scope:
- identify the minimum FMC model and presentation semantics that must survive
  the refactor
- use `legacy_backup/libs/fm_fmc.*`, `fm_user.c`, and `fm_setup.c` as behavior
  guides only for domain discovery and contrast
- define the first new product-domain module boundary under `src/product/fmc/`
- keep the design oriented to a flow-computer instrument, not generic display
  abstraction for every future use case

Out of scope for the first slice:
- pulse/capture math
- ACM/TTL/RATE computation from pulses
- setup/user menu flow
- `FM_LCD_LL_*`
- Bluetooth
- RTC
- ticketing
- backup SRAM persistence
- factory-profile loading

---

## Observed Validated Case or Trusted Reference Evidence

Trusted display foundation:
- `docs/contexts/archive/lcd_stack_closed/lcd_redesign.md`
- the closed LCD stack under `src/bsp/devices/lcd/`

Trusted FMC legacy behavior guides:
- `legacy_backup/libs/fm_fmc.c`
- `legacy_backup/libs/fm_fmc.h`
- `legacy_backup/libs/fm_user.c`
- `legacy_backup/libs/fm_setup.c`
- `docs/specs/fmc/use_cases.docx`
- `docs/specs/fmc/use_cases.yaml`

Relevant legacy presentation-oriented functions already identified:
- `FM_FMC_Ufp3ToString`
- `FM_FMC_TotalizerStrUnitGet`
- `FM_FMC_RateFpSelGet` / `FM_FMC_RateFpInc`
- `FM_FMC_TotalizerFpSelGet` / `FM_FMC_TotalizerFpInc`
- `FM_FMC_TotalizerTimeUnitSel`

Relevant legacy display usage already identified:
- `MenuUserFormatTotalizerRow1`
- `MenuUserTtlRateRefresh`
- `MenuUserAcmRateRefresh`
- `MenuUserRateRefresh`
- `MenuSetupFactorCalEdit`
- `MenuSetupVolUnitEdit`
- `MenuSetupTimeUnitEdit`

Additional legacy behavior evidence from the use-case document:
- `TTL/RATE` screen:
  - top row shows historical total
  - bottom row shows instantaneous rate
  - volume unit is shared visually by total and rate
  - time base qualifies rate
- `ACM/RATE` screen:
  - top row shows resettable accumulated volume
  - bottom row shows instantaneous rate
- decimal selection for total and rate is independently user-adjustable
- pulse activity has its own visible cue

Applicable use-case YAML signals for this module:
- persistence model:
  - TTL and ACM are derived from accumulated pulses
  - unit or calibration changes update displayed accumulated volumes
- user-screen semantics:
  - `SCREEN_TTL_RATE`:
    - upper row is TTL
    - lower row is RATE
  - `SCREEN_ACM_RATE`:
    - upper row is ACM
    - lower row is RATE
- configuration semantics:
  - ACM/TTL share the same volume-unit configuration
  - RATE owns time-base configuration separately
- refactor guidance:
  - measurement model should remain independent from display formatting

Trusted external reference signals:
- TI MSP430 ultrasonic flow-meter references expose:
  - explicit meter constant calibration against a reference flow rate
  - multi-range flow calibration for different accuracy regions
  - a distinction between measurement algorithms and meter constants
  - integrated segment-LCD-oriented reference designs for low-power meters
- ST flow-metering references expose:
  - explicit counting, tachometer, speed, rotation-direction, and error-detect
    building blocks
  - separation between raw wheel/sensor acquisition and higher-level metering
  - gas/water metering example packages intended as starting points rather than
    fixed application semantics
  - calibration concepts for dynamic sensing and threshold validation

These references support the idea that the new refactor should model FMC
elements first, then presentation semantics, and only after that connect to the
LCD stack.

---

## Analysis Pass Findings

### 1. Recommended layer split

The clean split for this refactor is:

1. `fmc_model.*`
   - defines FMC domain elements and their relationships
   - does not know about LCD rows or `FM_LCD_*`
   - remains plain copyable value state plus pure structural helpers
   - does not expose visible volume, operative pulses-per-active-unit, or
     instantaneous rate as first-slice model queries
   - does not own RTOS synchronization or live shared state
2. `fmc_units.*`
   - owns product unit conversion policy
   - is not a generic portable unit library
   - can remain pure and independently reviewable
3. `fmc_rate.*`
   - owns rate calculation policy from pulse/time windows
   - can start pure, even though it will later be consumed by an RTOS task or
     service
4. `fmc_service.*` or `fmc_runtime.*`
   - owns live FMC state in the RTOS firmware
   - owns synchronization primitives, queues/event flags/timers if needed
   - receives acquisition updates or snapshots
   - exposes coherent snapshots to UI, logging, persistence, and presentation
5. `fmc_config.*`
   - applies product configuration to the model:
     - calibration input
     - active unit
     - time base
   - may later consume storage/persistence services
6. `fmc_presentation.*`
   - decides how one FMC snapshot is rendered semantically for this
     instrument:
     - which value goes on top row
     - which value goes on bottom row
     - which alpha cues are shown
     - which legends/indicators are active
   - still does not write to LCD
7. FMC-to-LCD adapter
   - consumes the semantic presentation result and calls the validated LCD
     stack

Current candidate public entrypoint for the first layer:
- [src/product/fmc/fmc_model.h](/d:/githubs/FMC-320U-FW/firmware/src/product/fmc/fmc_model.h)

Naming decision:
- FMC product modules under `src/product/fmc/` use `fmc_*` filenames
- public C symbols use the `FMC_*` module namespace
- this avoids duplicating the repository prefix inside product-domain module
  names

This keeps the first slice coherent with the user request:
- no pulse/capture math yet
- no direct LCD writes
- no loss of FMC instrument semantics
- no inheritance of old `fm_user` / `fm_setup` screen semantics as a contract
- no RTOS ownership inside the model contract

Important RTOS/product decision:
- these modules are product firmware modules, not portable flow-computer
  libraries
- portability is not the goal; clear ownership under the chosen RTOS is the
  goal
- however, product specificity does not mean every module should include RTOS
  primitives
- `model`, `units`, and early `rate` code should remain pure where possible
- `service` or `runtime` is the intended boundary for mutexes, queues, event
  flags, timers, task ownership, live-state mutation, and coherent snapshots
- this keeps the RTOS from contaminating the core semantics while still
  designing for the actual firmware architecture

### 2. Elements that belong in the FMC model

The minimum FMC model should define at least:
- accumulated total role:
  - `ACM`
- historical or non-resettable total role:
  - `TTL`
- instantaneous flow rate:
  - `RATE`
- raw pulse counters:
  - pulse count feeding ACM
  - pulse count feeding TTL
- calibration ownership:
  - canonical calibration factor as pulses per calibration unit
  - no stored operative pulses-per-active-unit factor in the core model
- unit semantics:
  - active volume unit
  - active time base for rate
- reset semantics:
  - ACM resettable
  - TTL not user-resettable
- status or quality hooks reserved for future:
  - flow active / pulse activity
  - low-flow cutoff or gate
  - future net/gross direction if needed

Latest clarified modeling decisions from the review:
- `TTL` is resettable in the product, but through a more privileged mechanism
  than `ACM`
- `TTL` remains non-user-resettable in normal operation
- the model reset primitive does not need a separate authentication mechanism:
  - UI/service flow protects access to TTL reset
  - once that flow is reached, the same total reset primitive can reset ACM or
    TTL according to the selected role
- `ACM` and `TTL` should remain simple and close to each other in the code
- in practice, `ACM`, `TTL`, and `RATE` should share the same active volume
  unit in this instrument
- `RATE` should differ mainly by its time base, not by a separate volume-unit
  ownership
- `ACM` and `TTL` should not store visible volume as canonical state:
  - their canonical runtime backing state is the pulse counter
  - visible volume should be derived from pulses plus the active measurement
    environment
- `RATE` remains a first-class FMC semantic element, but the first core model
  should not store or return an instantaneous rate value without the
  acquisition/rate behavior that produced it
- the legacy `BLANK` unit should be reinterpreted as an unsupported/future-unit
  placeholder:
  - it is unsupported by the physical conversion table
  - it remains operational with conversion factor `1`
  - whoever configures the instrument must load a calibration factor already
    expressed as pulses per desired custom unit
  - presentation may render that state as `--`
- the barrel unit should use the model name `BBL_US`; the visible label `BR`
  belongs to presentation
- legacy `factor_cal` should now be read as a practical calibration truth
  anchored by liters:
  - calibration is performed from total pulses against a reference reading in
    liters
  - factor 1 is used during that calibration step
  - the editable product range remains `1.000` to `99999.999`, inherited from
    the legacy eight-digit, three-decimal representation
  - the refactor may expose this as `double`, but should preserve the product
    range as semantic validation rather than as a fixed-point type leak
- the refactor should make the calibration unit explicit to avoid hiding the
  liter anchor:
  - no menu, display, or operator-facing change of calibration unit is
    implemented now
  - known physical unit conversions normally use the liter-anchored path
  - unsupported/custom units rely on a factor already loaded for that unit and
    therefore use conversion factor `1`
- `factor_k` should now be read as the operative environment factor:
  - some unit selections derive a new operative factor from calibration
  - others intentionally keep conversion factor 1
  - in those cases the calibrator/operator already converted the reference
    quantity into the final unit before loading calibration
  - the refactor may later choose to expose this operative K to make those
    transitions explicit to the user/config flow
- legacy `factor_k` also had a concrete embedded reason to exist:
  - it precomputed `factor_cal * conversion_value`
  - this reduced runtime arithmetic cost on an ultra-low-power target
  - for conversion factor `1`, the operative factor simply matches
    `factor_cal`
- this means the new design must re-evaluate whether "canonical pulses per
  liter" is a true universal contract or only one normal path among several
  supported calibration modes
- the user clarified that the old scheme was intentionally left extensible:
  - a future cleaner design could add one calibration-unit dimension
  - that would allow explicit calibration in kilograms or another unit
  - but this extension is not required to understand or preserve the current
    working behavior
- for the first slice, `KG` should remain supported as the current intentional
  hack:
  - it stays exposed as a product unit
  - it does not force density-aware redesign now
  - a later refactor may replace this with explicit calibration-unit support
- `ME` should also remain supported as a current intentional hack in the first
  slice:
  - it enters the legacy table through the liters path
  - then uses conversion factor `1`
  - it should not be mistaken for a normal physical-conversion unit like `M3`
    or `GL`
- `unit_convert` should not survive as one mixed concept:
  - the legacy table combined real physical conversions and special hack cases
  - the preferred refactor direction is to split those concerns later
- `LT` should still be treated as a normal selectable user unit:
  - it is not semantically "more real" than the others
  - but the legacy implementation currently uses liters as the table entry
    point that turns `factor_cal` into `factor_k`
  - a future cleaner design may separate that conversion-anchor role from the
    public unit enum
- any secondary or tertiary operative factor used to avoid repeated calculations
  should be treated as an internal optimization, not as a public semantic
  element
- pulse counters remain important candidates as backing state behind totals,
  especially if total values may be recomputed after configuration changes
- `flow_active` and `pulse_activity` were identified as implementation leakage
  and should not stay in the first core model contract
- visible ACM, TTL, and RATE are derived environment views, not editable
  configuration; ACM and TTL are backed by canonical pulse counters
- visible volume and operative pulses-per-active-unit views are derived
  behavior over model state plus unit/rate policy; they should be introduced in
  later helpers rather than as `fmc_model.*` stored state or direct model
  queries

Recommended simplification from those decisions:
- define one primitive reusable total state type
- instantiate it twice:
  - `ACM`
  - `TTL`
- make reset policy part of the total-role contract as information for callers;
  the UI/service layer enforces access to privileged TTL reset
- store canonical total pulse counters in the model, not cached visible total
  volumes
- keep one shared measurement context for:
  - active volume unit
  - active time base
  - calibration value
  - explicit calibration unit without adding a UI for changing it now
- expose visible volume, operative pulses-per-active-unit, and rate later
  through unit/rate helpers or presentation/runtime views, not through the core
  model contract
- keep the full model shape copyable so later product layers can hold:
  - active environment
  - edit copy
  - factory/default copy
  without redesigning the core contract

### 3. Elements that belong in presentation semantics

The presentation layer should own:
- row responsibility:
  - top row = total value view
  - bottom row = rate view
- display mode or view identity:
  - `TTL/RATE`
  - `ACM/RATE`
- decimal policy:
  - total decimals
  - rate decimals
- unit cue policy:
  - alpha pair for the active volume unit
  - slash indicator between unit and time base
  - one active time-base indicator:
    - `S`
    - `M`
    - `H`
    - `D`
- legend policy:
  - `TTL`
  - `ACM`
  - `RATE`
- optional future pulse/activity cue

### 4. Recommendation about scaled math

Scaled/fixed-point remains a valid embedded strategy, but it should not define
the public semantics of the new refactor.

Recommended rule:
- semantic contracts should speak in FMC meanings:
  - total
  - rate
  - pulses
  - units
  - time base
  - decimals
  - reset policy
- the chosen numeric representation:
  - scaled integer
  - decimal struct
  - or another deterministic representation
  should remain an implementation detail unless a later constraint proves it
  must be surfaced

This avoids baking `ufp3_t` into the wrong layer just because it was convenient
in the legacy module.

### 5. Important legacy behaviors worth preserving

These domain ideas appear meaningful and should survive in some form:
- separate total roles with different reset policies
- rate as a first-class FMC element
- explicit volume unit and time-base ownership
- pulse-to-quantity conversion through calibration plus unit policy
- decimals as presentation policy, not domain math

These legacy details should not survive as-is:
- direct `FM_LCD_LL_*` coupling inside FMC semantics
- old screen/menu ownership as semantic truth
- hidden derived/canonical ownership rules inside one struct
- presentation logic mixed with persistence/debug assumptions

---

## Current Modeling Corrections

The first candidate header over-transferred some legacy structure. The current
correction direction is:

- keep:
  - FMC total roles
  - reset policy
  - shared unit/time-base ownership
  - canonical calibration
  - pulse counters as meaningful backing state
- remove or reduce:
  - duplicated unit ownership inside every quantity type
  - public storage of multiple derived operative-factor variants
  - direct model queries for visible volume or operative factor
- generic runtime snapshot shape that looks like a legacy environment dump
- `flow_active` / `pulse_activity` in the first core contract

---

## Intermediate Pass: Header Classification

Current candidate file under review:
- [src/product/fmc/fmc_model.h](/d:/githubs/FMC-320U-FW/firmware/src/product/fmc/fmc_model.h)
- [docs/specs/fmc/fm_fmc_legacy_field_inventory.md](/d:/githubs/FMC-320U-FW/firmware/docs/specs/fmc/fm_fmc_legacy_field_inventory.md)

This intermediate pass classifies each current concept into:
- canonical model state
- semantic public state
- derived behavior for later helpers
- presentation leakage
- questionable runtime state

The editable working inventory now lives in:
- `docs/specs/fmc/fm_fmc_legacy_field_inventory.md`

That file is intended as the mutable review artifact before freezing more
decisions into `src/product/fmc/fmc_model.h`.

### A. Canonical model state

These are good candidates to remain as the instrument source of truth:
- canonical calibration as pulses per calibration unit
- shared active volume unit
- shared active time base
- pulse counters backing totals
- reset policy associated with each total role

Reason:
- they define how the instrument interprets and recomputes measured quantity
- they are not display-specific
- they remain meaningful even if the LCD layer changes completely

### B. Semantic public state

These should remain visible as first-class FMC elements:
- `ACM`
- `TTL`
- `RATE`

Reason:
- they are the values the flow computer exists to expose
- they have stable meaning for product behavior and later presentation
- they are runtime-derived visible views, not parameter-edit state
- ACM and TTL are backed in the model by pulse counters rather than cached
  visible volume

### C. Derived behavior for later helpers

These may become public through later unit/rate/presentation helpers, but should
not be canonical stored state or first-slice `fmc_model.*` queries:
- operative pulses-per-active-unit factor
- any optimized rate conversion factor derived from calibration and time base
- visible volume derived from total pulses

Reason:
- they are useful engineering views
- they are derivable from canonical state
- keeping them outside the core model preserves clarity and avoids duplicated
  ownership

### D. Presentation leakage

These do not belong in the core model contract:
- decimal placement
- alpha usage
- row responsibility
- indicator selection
- text formatting

Reason:
- they belong to FMC presentation semantics or the LCD adapter

### E. Questionable runtime state

These should stay out of the first model contract unless a later requirement
promotes them:
- `flow_active`
- `pulse_activity`

Reason:
- they currently read more like implementation or UI-support state than
  instrument-defining elements
- they may later reappear as status/diagnostic concepts, but not as part of the
  minimum core model

### F. Current over-transfer in the candidate header

The current candidate over-transfers these legacy patterns:
- duplicated unit ownership:
  - shared active unit in context
  - per-value unit in total and rate types
- environment-dump style snapshot instead of a more intentional model
- setters that treat the domain like a bag of fields instead of a coherent
  instrument model

This reinforces the next cleanup target:
- reduce duplication
- prefer one primitive total state type reused by `ACM` and `TTL`
- keep calibration canonical
- move operative factor and visible-volume derivation to later helpers, not
  equal-rank stored truth
- keep the model as plain value state instead of singleton-oriented state

Interpretation risk still present in the candidate header:
- if a reviewer can reasonably ask "what K is this using?"
- or mistake totals for editable configuration
- then the contract still needs stronger comments or an explicit active-derived
  factor in the environment model

---

## Refined Vendor Signals

Silicon-vendor references support the split between canonical meter state and
derived engineering views:

- TI USS calibration guidance treats meter constant calibration and multi-range
  calibration as explicit meter configuration artifacts, separate from runtime
  measurement algorithm details.
- TI ultrasonic water-meter reference material is positioned as a modular
  starting point for customized meter solutions, not as a fixed UI/data model.
- ST LC-sensor metering material separates:
  - counting
  - direction detection
  - tachometer/speed
  - periodic calibration
  from the higher-level application built on top of those blocks.

This supports keeping the new FMC model focused on:
- canonical calibration
- quantities and total roles
- shared unit/time-base ownership
- canonical pulse backing state for totals

Derived engineering views such as operative factor, visible volume, and rate
remain important product behavior, but they should be introduced through later
unit/rate/runtime/presentation helpers rather than as first-slice model storage.

The RTOS-facing product direction adds one more boundary:
- the FMC core is not a portable library, but `fmc_model.*` should still not
  become the live singleton or synchronization owner
- `fmc_service.*` or `fmc_runtime.*` should own the active instance,
  concurrency, pulse/acquisition updates, and snapshot publication
- UI, logging, persistence, and presentation should consume coherent snapshots
  or service APIs rather than directly sharing mutable model storage

---

## Known Gaps

- the new model module name is `fmc_model.*`
- the first `fmc_model.h` contract is ready for a minimal `.c`
  implementation, but the broader FMC module family contract will continue to
  evolve slice by slice
- the RTOS-facing owner module name is not frozen:
  - `fmc_service.*`
  - or `fmc_runtime.*`
- TTL reset policy has a resolved interpretation:
  - `docs/specs/fmc/use_cases.yaml` is still correct that TTL is not
    user-resettable
  - the refactor model exposes a reset primitive that can serve TTL once a
    privileged UI/service flow has authorized access
- the first-slice boundary between:
  - FMC model semantics
  - FMC units/rate helpers
  - FMC service/runtime ownership
  - FMC presentation semantics
  - LCD adapter
  is not frozen yet
- alpha usage for units in the new LCD stack needs an explicit mapping decision
- equivalent cubic-meter label is clarified:
  - model name `FMC_MODEL_VOLUME_UNIT_EQUIV_M3` is acceptable
  - conceptual name is equivalent cubic meter
  - visible label should be `MC`
  - normal cubic meter remains a separate unit with visible label `M3` and
    physical conversion `1 m3 = 1000 L`
  - equivalent cubic meter uses the special product relation
    `1 L = 1 equivalent m3`
- it is still open whether the new layer should accept:
  - a domain snapshot struct
  - or a more granular set of typed semantic fields
- it is still open whether ACM/TTL names remain public API vocabulary or become
  product-level aliases over more generic total roles

---

## Immediate Next Slice

Implement `src/product/fmc/fmc_model.c` for the current header only.

The implementation should cover:
- `FMC_MODEL_Init`
- `FMC_MODEL_GetResetPolicy`
- `FMC_MODEL_GetTotalState`
- `FMC_MODEL_GetTotalStateConst`
- `FMC_MODEL_ResetTotal`

It should not introduce:
- RTOS primitives or singleton ownership
- unit conversion
- visible volume calculation
- instantaneous rate calculation
- LCD or presentation behavior
- persistence or log layout

After that source exists, the next design choice should be between:
- `fmc_units.*` for unit conversion and operative factor behavior
- `fmc_rate.*` for rate calculation from pulse/time windows

`fmc_service.*` or `fmc_runtime.*` should wait until RTOS ownership and snapshot
publication are being modeled explicitly.

---

## Risks

- dragging too much legacy FMC core into what should be a presentation slice
- reproducing `FM_LCD_LL_*` coupling in the new layer
- making the module too generic and losing the semantics of a flow-computer
  instrument
- preserving formatting details without preserving row-level meaning
- freezing `ufp3_t` or current derived fields into the public contract too
  early
- letting future RTOS ownership leak into `fmc_model.*` and obscure the
  pure model semantics

---

## Minimum Plan

1. implement the first `fmc_model.*` slice:
   - canonical model initialization
   - total-role reset policy
   - ACM/TTL state selection helpers
   - total reset over pulse counters
2. keep future derived behavior outside the model:
   - visible volume
   - operative factor
   - instantaneous rate
3. freeze the minimum FMC element model family:
   - totals
   - rate-related time base, not instantaneous rate storage
   - pulses
   - calibration, not operative-factor storage
   - units
   - time base
   - reset semantics
4. define the seam from FMC model to FMC presentation semantics
5. define the future RTOS-facing ownership boundary:
   - live model instance
   - synchronization
   - acquisition updates
   - snapshots for UI/log/persistence
6. define how presentation semantics map onto:
   - top row
   - bottom row
   - alpha pair
   - indicators
   without calling `FM_LCD_LL_*`
7. only after that, decide the LCD adapter that consumes the presentation
   result through the validated LCD stack
