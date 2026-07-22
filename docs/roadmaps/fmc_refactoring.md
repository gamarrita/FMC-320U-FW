# FMC Refactoring Roadmap

## Purpose

Define the durable refactor roadmap for the FMC-320 firmware. This roadmap
describes phases, dependencies, decision gates, risks, and exit criteria. It is
not the active work tracker and does not define module APIs.

Product behavior comes from `docs/specs/fmc/use_cases.yaml`. Public contracts
for implemented modules belong in their headers.

## Phase 1: Model And Pure Calculations

Objective:
- isolate canonical FMC model state and pure unit, volume, rate, and formatting
  calculations.

Dependencies:
- confirmed product units, totals, calibration, and rate semantics from
  `use_cases.yaml`;
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
- reset policy and totalization requirements from `use_cases.yaml`.

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
- current-product input requirements in `use_cases.yaml`;
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

## Phase 6: Presentation And UI State Machine

Objective:
- implement user-visible screen behavior from the current product
  specification.

Dependencies:
- stable runtime snapshots;
- semantic input;
- ThreadX/timer ownership when screen timing requires it;
- unresolved presentation requirements curated in `use_cases.yaml`.

Decision gates:
- startup sequence ownership;
- user/config state-machine split;
- numeric overflow/invalid display;
- backlight and activity policy;
- alarm overlay ownership.

Risks:
- recreating the deleted presentation design document as a second spec;
- freezing `fmc_presentation` APIs before the slice is active.

Exit criteria:
- behavior is traceable to `use_cases.yaml`;
- public module contracts live in headers;
- tests cover screen state and formatting decisions selected for the slice.

## Phase 7: Acquisition, RTC, And Persistence

Objective:
- connect pulse acquisition, rate windows, date/time, backup retention, and
  flash-backed configuration to runtime and service state.

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

## Phase 8: Alarms, Logging, Bluetooth, Printing, And Optional Features

Objective:
- implement remaining product workflows deliberately after core runtime and UI
  ownership are stable.

Dependencies:
- UI state machine;
- persistence;
- communication and power decisions;
- unresolved optional behavior decisions in `use_cases.yaml`.

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
