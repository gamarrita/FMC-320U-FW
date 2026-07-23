# FMC Refactoring Roadmap

## Purpose

Define the durable refactor roadmap for the FMC-320 firmware. This roadmap
describes phases, dependencies, decision gates, risks, and exit criteria. It is
not the active work tracker and does not define module APIs.

Accepted product behavior comes from the reviewed documentation or
specification applicable to each slice. `docs/specs/fmc/use_cases.yaml` is an
evolving inventory and design input, not a complete or universal product
contract. Public contracts for implemented modules belong in their headers.

## Program Sequence From The Current Point

The remaining program follows this dependency order. Phase labels name durable
work packages; their numeric labels do not override this sequence.

1. Reframe documentation and establish the human-agent-repository workflow.
2. Build the FMC product-documentation backbone.
3. Give the principal product functions broad, superficial coverage.
4. Incorporate useful requirements and use cases incrementally, with explicit
   review before they become authoritative.
5. Deepen the next visible programming slice.
6. Implement Phase 6A: the bounded initial presentation slice.
7. Add essential acquisition, persistence, and RTC work when Phase 6A or later
   product dependencies require it.
8. Implement Phase 6B: operational screens and navigation.
9. Add configuration workflows and other deferred functions deliberately.

Each step depends on enough reviewed output from the preceding steps to define
a small implementation or documentation cut. This is not a requirement to
complete all product documentation before programming: breadth comes first,
then depth follows the next slice.

## Phase 1: Model And Pure Calculations

Objective:
- isolate canonical FMC model state and pure unit, volume, rate, and formatting
  calculations.

Dependencies:
- reviewed product units, totals, calibration, and rate semantics for the
  selected slice; `use_cases.yaml` may supply candidate requirements;
- legacy source only as evidence.

Decision gates:
- numeric range and invalid-value behavior when the current specification does
  not define operator-visible output.

Risks:
- accidentally preserving legacy representation rather than current product
  behavior;
- mixing presentation labels into deeper model logic.

Exit criteria:
- pure contracts are documented in headers;
- regression tests cover valid, invalid, boundary, and unsupported cases.

## Phase 2: Live State And `fmc_service`

Objective:
- define the owner of live FMC state and the controlled operations that mutate
  or snapshot it.

Dependencies:
- Phase 1 contracts;
- reviewed reset policy and totalization requirements for the selected slice.

Decision gates:
- whether service state should remain directly inspectable for tests/restores
  or become opaque after persistence is designed.

Risks:
- letting acquisition, UI, or persistence mutate model state directly;
- adding RTOS synchronization before ownership is clear.

Exit criteria:
- service owns one live model instance;
- pulse deltas and resets preserve service invariants;
- snapshots are stable and covered by tests.

## Phase 3: `fmc_runtime` Contract

Objective:
- establish the RTOS-neutral runtime event boundary over `fmc_service`.

Dependencies:
- Phase 2 service contract.

Decision gates:
- event payload representation;
- invalid-event behavior;
- snapshot ownership and update notification semantics.

Risks:
- importing scheduler, queue, HAL, BSP, or LCD types into product contracts;
- encoding temporary UI behavior as permanent runtime API.

Exit criteria:
- runtime accepts product events without platform types;
- runtime preserves service ownership and snapshot access;
- tests cover dispatch and error paths.

## Phase 4: Semantic Input Architecture

Objective:
- preserve product input identity before assigning screen-specific
  consequences.

Dependencies:
- reviewed current-product input requirements and implemented input contracts;
- board key mapping in BSP;
- runtime event contract.

Decision gates:
- final location of app-specific composition code after a second consumer
  exists;
- handling of external buttons alongside mechanical keyboard keys.

Risks:
- losing key identity by converting input directly into presentation updates;
- coupling product input contracts to BSP, GPIO, EXTI, HAL, RTOS, or timer
  types.

Exit criteria:
- product input types include mechanical keys, external buttons, SHORT, and
  LONG;
- short/long recognition can produce runtime input without changing runtime
  input interfaces;
- pure tests prove key/action identity is preserved.

## Phase 5: ThreadX, ISR Delivery, Timers, And Low Power

Objective:
- define and demonstrate ThreadX runtime ownership, serialized ISR-to-thread
  delivery, timer ownership, and low-power interaction.

Runtime direction:
- ThreadX is the only active firmware runtime from this phase onward.
- The last bare-metal firmware state is a historical comparison baseline, not a
  second maintained architecture.
- Selectable apps continue through a ThreadX bootstrap harness. For
  `product/main`, the existing `FM_APP` thread runs the `FM_MAIN_Main()` owner
  loop directly.

Completed slice:
- use the existing `FM_APP` ThreadX thread as the only owner of `fmc_runtime`;
- deliver mechanical keyboard, key-hold timeout, and provisional periodic
  refresh events to that owner through one ThreadX queue;
- use app-level event payloads at producer-to-owner boundaries;
- use an initial queue depth of 8 events;
- treat owner queue overflow as abnormal and make it explicit;
- recognize mechanical-key `SHORT` and `LONG` in `product/main` using
  hardware-confirmed RISING/FALLING edges and a one-shot 3 second timer;
- hardware-smoke validate DOWN, UP, ENTER, and ESC for SHORT, LONG, and no
  duplicate SHORT after LONG;
- defer low-power policy, presentation ownership, wake/backlight policy,
  debounce, menu consequences, and external buttons.

Dependencies:
- semantic input contract;
- runtime contract;
- CubeMX-generated ThreadX and low-power-support configuration.

Decision gates:
- owner loop startup sequencing in the existing `FM_APP` thread;
- exact ThreadX queue storage ownership and overflow action;
- wake source and low-power ownership;
- presentation/backlight activity ownership.

Risks:
- enabling ThreadX only for long press and accidentally redesigning startup,
  low-power, queues, timers, and ownership at once;
- continuing too long on accidental bare-metal assumptions.
- letting a historical bare-metal baseline become an accidental second product
  line.

Exit criteria:
- minimal runtime runs under the selected ThreadX ownership model;
- ISR path does bounded work only;
- delivery to runtime is serialized and testable.

## Phase 6A: Initial Presentation Slice

Objective:
- validate the first bounded path from an FMC runtime snapshot to controlled,
  user-visible LCD output.

Dependencies:
- the product-documentation backbone and broad functional coverage;
- reviewed deep documentation for the selected visible slice;
- stable runtime snapshots;
- `docs/specs/lcd/lcd_true_source.yaml` as the technical LCD authority;
- existing semantic LCD and driver contracts;
- ThreadX/timer ownership only where the selected startup timing requires it.

Included scope:
- all-segments startup screen;
- firmware version screen;
- steady TTL/RATE screen;
- projection of an FMC snapshot into semantic LCD elements;
- a validated presentation-to-driver adapter;
- pure formatting tests;
- hardware bring-up with controlled values.

Decision gates:
- startup sequence ownership;
- numeric overflow/invalid display;
- exact reviewed content and timing for the three selected states;
- adapter ownership at the product/BSP boundary.

Risks:
- treating candidate behavior in `use_cases.yaml` as approved requirements;
- freezing `fmc_presentation` APIs before the slice is active.

Exit criteria:
- the three selected states are traceable to reviewed slice documentation;
- snapshot fields project into semantic LCD elements without exposing LCD
  mapping details to the FMC product model;
- the presentation-to-driver adapter is validated;
- pure tests cover selected formatting decisions;
- controlled-value hardware bring-up is recorded;
- public module contracts live in headers;
- no complete UI state machine, complete navigation, configuration screens, or
  exact legacy reproduction has been introduced.

## Phase 7: Essential Acquisition, RTC, And Persistence

Objective:
- connect pulse acquisition, rate windows, date/time, backup retention, and
  essential persistence to runtime and service state when dependencies from
  Phase 6A or the next operational slice require them.

Dependencies:
- runtime ownership;
- low-power and ISR delivery decisions;
- current product requirements for pulse loss, RTC, and persistence.

Decision gates:
- pulse capture ownership;
- rate window representation;
- backup/flash data model and validation;
- RTC validity and recovery policy.

Risks:
- losing pulses across sleep or context transitions;
- duplicating canonical state between RAM, flash, backup, and presentation.

Exit criteria:
- acquisition updates service state through runtime;
- persistence and RTC behavior preserve confirmed requirements;
- tests and bring-ups cover selected behavior and recovery paths.

## Phase 6B: Operational Screens And Navigation

Objective:
- implement the operational user screens and navigation selected after Phase
  6A, using reviewed behavior and the essential runtime data made available by
  Phase 7 when required.

Dependencies:
- Phase 6A presentation path and lessons;
- semantic input and owner-loop contracts;
- reviewed operational-screen and navigation behavior;
- essential acquisition, persistence, or RTC capabilities required by the
  selected screens.

Decision gates:
- operational state-machine boundaries;
- navigation and return behavior;
- refresh, activity, and backlight policy;
- alarm or transient overlays needed by the selected operational flow.

Risks:
- expanding a selected flow into a complete legacy menu reproduction;
- coupling navigation directly to LCD mapping or storage details;
- implementing unresolved candidate use cases as accepted behavior.

Exit criteria:
- selected operational screens and navigation are traceable to reviewed product
  documentation;
- state transitions and formatting have focused tests;
- hardware bring-up covers the selected operator flow;
- configuration screens and unrelated deferred functions remain out of scope.

## Phase 8: Configuration And Deferred Functions

Objective:
- implement configuration workflows and remaining product functions
  deliberately after core runtime and operational UI ownership are stable.

Dependencies:
- Phase 6B operational navigation;
- persistence and RTC where required;
- communication and power decisions;
- reviewed requirements for each selected deferred function.

Decision gates:
- alarm reset semantics;
- logging wear/energy policy;
- Bluetooth window/date-time overlap;
- printer workflow and transport ownership;
- optional PT100 behavior.

Risks:
- broad workflow coupling;
- excessive power draw from optional features;
- implementing not-yet-confirmed behavior as mandatory.

Exit criteria:
- each workflow has a small product contract, implementation, and validation
  path;
- optional behavior remains isolated when not selected.

## Phase 9: Integral Validation, Power, And Product Behavior

Objective:
- validate the refactored firmware against confirmed product behavior and
  field constraints.

Dependencies:
- previous phases implemented for the selected release scope.

Decision gates:
- release acceptance criteria;
- current consumption thresholds;
- hardware bring-up coverage;
- unresolved requirements that must be settled before release.

Risks:
- passing unit tests while missing field behavior;
- current consumption regressions;
- drift between specification, tests, and firmware.

Exit criteria:
- canonical builds pass;
- regression tests and bring-ups cover selected product paths;
- current consumption and pulse behavior meet confirmed requirements;
- remaining unresolved items are explicitly out of release scope or decided.
