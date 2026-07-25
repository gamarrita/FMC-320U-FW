# FMC Refactoring Roadmap

## Purpose

Define the durable refactor roadmap for the FMC-320 firmware. This roadmap
describes phases, dependencies, decision gates, risks, and exit criteria. It is
not the active work tracker and does not define module APIs.

Accepted product behavior comes from the reviewed documentation under
`docs/product/fmc/`. The frozen extraction at
`legacy/derived/fmc/use_cases.extraction-v1.yaml` is derived legacy evidence,
not a current product contract. Public contracts for implemented modules belong
in their headers.

## Program Sequence From The Current Point

The remaining program follows this dependency order:

1. Reframe documentation and establish the human-agent-repository workflow.
2. Build the FMC product-documentation backbone.
3. Give the principal product functions broad, superficial coverage.
4. Incorporate useful requirements and use cases incrementally, with explicit
   review before they become authoritative.
5. Deepen the next visible programming slice.
6. Implement Phase 6A: the bounded initial presentation slice.
7. Add essential acquisition through the dedicated incremental acquisition
   route.
8. Add the minimum measurement user screens and their navigation.
9. Add RTC/calendar behavior with its required user and configuration screens
   and navigation.
10. Add the minimum measurement configuration screens and their navigation.
11. Retain the approved high-change state in Backup SRAM.
12. Persist the approved low-change configuration in Flash.
13. Add temperature measurement as one vertical capability, including only the
    UI and configuration it requires.
14. Add Bluetooth as one vertical capability, including only the UI and
    configuration it requires.
15. Add ticket printing as one vertical capability after its Bluetooth
    and RTC dependencies.
16. Perform integral product, power, and release validation.

Each phase depends only on the dependencies stated in its section. The numeric
order is the current delivery route, not permission to infer product behavior.
Independent vertical capabilities may be deferred without collapsing their UI
into another phase; ticket printing retains its explicit Bluetooth dependency.

This is not a requirement to complete all product documentation before
programming: breadth comes first, then depth follows the next slice.

Phases 8 and 10 are separate because operational navigation and measurement
configuration navigation have different behavior, authorization, validation,
and test boundaries. Phase 9 is a complete RTC/calendar vertical slice, including
its own user and configuration screens. Phases 13 through 15 likewise do not
defer their necessary UI to a later generic screen phase.

## Phase 1: Model And Pure Calculations

Objective:
- isolate canonical FMC model state and pure unit, volume, rate, and formatting
  calculations.

Dependencies:
- reviewed product units, totals, calibration, and rate semantics for the
  selected slice; the frozen legacy extraction may supply evidence;
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

Status:
- completed and human-accepted on target hardware.

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
- treating behavior in the frozen legacy extraction as approved requirements;
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

Completion:
- all exit criteria above are satisfied by the Phase 6A presentation,
  product-main LCD adapter, focused regression coverage, canonical builds, and
  controlled-value target validation.

## Phase 7: Essential Acquisition

Objective:
- replace provisional Phase 6A TTL/RATE inputs through short, independently
  reviewable acquisition slices;
- connect accepted pulse deltas and validated RATE observations through runtime
  ownership without coupling acquisition to product totals or presentation.

Responsibility boundaries:
- physical pulse accumulation;
- counter observation and pulse-delta formation;
- ACM/TTL totalization;
- physical frequency observation;
- pulse/time observation quality;
- pure RATE mathematics;
- runtime and presentation integration.

Dependencies:
- completed Phase 6A runtime/presentation path;
- current `fmc_runtime`, `fmc_service`, and pure `fmc_rate` ownership;
- reviewed product decisions for the bounded slice;
- human-approved hardware and CubeMX changes before each hardware bring-up;
- target equipment capable of generating and independently counting pulses and
  measuring current where low-power acceptance is in scope.

Decision gates:
- supported sensor signal, frequency, pulse-width, and low-power envelope;
- pulse-delta acceptance, modulo-wrap precondition, and reset boundary;
- LPTIM4 pin, clock, filtering, autonomous-mode, and CubeMX path;
- frequency observation semantics, accuracy, latency, and quality states;
- capture, DMA, interrupt, polling, or other frequency technique;
- acceptance or rejection of any legacy workaround after target evidence.

Risks:
- designing around an unconfirmed silicon defect;
- coupling totalization correctness to frequency availability;
- losing pulses across sleep or context transitions;
- hiding counter wrap, stale data, or invalid observations as zero flow;
- expanding one bring-up into the complete acquisition subsystem.

Correct-first baseline:
- legacy firmware and field reports remain evidence, not implementation
  authority;
- documented correct STM32U5 use is exercised without assuming the reported
  historical LPTIM behavior is a silicon defect;
- pulse accumulation is completed before frequency observation can endanger
  totalization;
- physical observation remains separate from RATE mathematics;
- DMA, interrupts, polling, autonomous peripherals, and other candidates are
  evaluated from required behavior and measured energy;
- a workaround is considered only after a minimal correct technique fails or
  cannot meet an approved constraint on target.

### Incremental Acquisition Route

`7-0A -> 7-0B -> 7A -> 7B1 -> 7B2 -> 7B3 -> 7C -> 7D -> 7E1 -> 7E2 -> [7E3] -> 7F -> 7G`

`7E3` is conditional. It opens only when accepted target evidence from 7E1 or
7E2 demonstrates an unmet approved constraint or a reproducible failure.

| Slice | Bounded result | Entry or decision gate | Exit evidence |
|---|---|---|---|
| 7-0A: Evidence and hypothesis formulation | Reconstruct legacy acquisition and classify the reported STM32U575 behavior | Legacy sources and identifiable public technical evidence are available | `legacy/analysis/fmc_acquisition.md` records provenance, uncertainties, hypotheses, and the correct-first baseline without selecting hardware |
| 7-0B: Foundation and route | Establish document ownership, incremental route, approval gates, and the next slice | 7-0A evidence is reviewable | Repository documents agree on the route and one active slice; no product or hardware decision is inferred |
| 7A: Pulse-accumulation contract | Define the bounded LPTIM4 counter observation accepted as a pulse delta | Human decisions on counter assumptions, selected low-power states, observation cadence, and loss policy | Product owners contain approved outcomes and `docs/specs/fmc/acquisition.md` defines delta, modulo wrap, reset, numeric, ownership, and acceptance semantics |
| 7B1: Pulse-counter bring-up in Run | Exercise the minimum documented LPTIM4 counter technique without Stop2 or RATE | Human-approved pin, clock, filter, CubeMX change, and Run signal matrix | Target evidence demonstrates exact raw counting and stable observation at approved Run limits |
| 7B2: Pulse-counter bring-up across Stop2 and wrap | Exercise the same counter path through low power and rollover | 7B1 accepted; Stop2 matrix and current-measurement method approved | Instrumented results demonstrate counting across Stop2 and wrap; separate silent runs establish current |
| 7B3: Counter-observation and pulse-delta module | Convert trusted counter observations into bounded pulse deltas | 7B1 and 7B2 hardware behavior accepted | Regression vectors cover first sample, zero, normal delta, modulo wrap, delayed observation, and reset without RATE or totals |
| 7C: Totalization/runtime integration | Deliver accepted deltas to ACM and TTL exactly once through runtime/service | 7B3 accepted; acquisition-to-runtime boundary reviewed | Regression and target evidence show no loss, duplication, or acquisition ownership of product totals |
| 7D: Frequency-observation contract | Define a pulse/time observation with explicit quality | Accumulation is stable enough that frequency work cannot endanger totals | Specification defines time-window ownership, range, accuracy, latency, zero, absent, stale, invalid, and RATE handoff |
| 7E1: Frequency bring-up in Run | Characterize the minimum documented frequency technique in Run | Human-approved technique, CubeMX path, signal matrix, and accuracy target | Target results cover the approved frequency range, elapsed time, wrap, and quality reporting |
| 7E2: Frequency bring-up across Stop2 | Exercise the accepted Run technique through low-power intervals | 7E1 accepted; Stop2 matrix and current-measurement method approved | Instrumented results cover Stop2 and wake behavior; separate silent runs establish current |
| 7E3: Conditional technique comparison | Compare a demonstrated failure with another documented technique or workaround | 7E1 or 7E2 records a minimal failure and the human approves comparison | Evidence selects, rejects, or defers alternatives without changing pulse-total correctness |
| 7F: RATE integration | Feed validated pulse/time observations into pure RATE calculation with explicit quality | 7D and required 7E evidence accepted; 7E3 closed if entered | Math vectors, boundaries, and runtime tests agree on units, elapsed time, invalid input, and zero/stale distinctions |
| 7G: Combined live integration | Combine accepted counter and frequency paths and replace provisional TTL/RATE inputs | 7C and 7F accepted; visible invalid/zero behavior approved | Combined bring-up, canonical builds, regression, target pulse accuracy, presentation, and current validation pass |

Phase 7A is a documentation-only, hardware-configuration-independent contract
slice. Phase 7B1 is the first
possible acquisition implementation and cannot start before approval of its
hardware and CubeMX gates.

### Human-Agent Bring-Up Protocol

Bring-ups use the existing debug UART as a transmit-only observation channel:

- the human flashes the board, controls the signal generator, and performs
  physical current measurement;
- the agent may monitor UART output, analyze evidence, and direct the next
  approved signal case;
- UART RX is not introduced;
- no UART transmission occurs while the MCU is in Stop2;
- bounded observations collected during Stop2 are emitted after return to Run.

Correctness and power are separate runs. Instrumented UART runs support
functional diagnosis; silent runs provide accepted current-consumption
evidence.

Human approval is required before:

- promoting acquisition behavior into product requirements;
- selecting a signal envelope, observation latency, low-power guarantee,
  pulse-loss policy, quality state, or visible failure behavior;
- selecting an LPTIM4 pin, clock, filter, detailed counter mode, interrupt, DMA
  path, autonomous mode, or CubeMX change;
- using the suspected STM32U575 behavior as a current design constraint;
- comparing or adopting the legacy wake-up workaround;
- beginning an implementation slice.

Exit criteria:
- the required acquisition slices close with their specified evidence, or are
  explicitly deferred while provisional inputs remain identified;
- acquisition updates service state only through the reviewed runtime boundary;
- pulse totals do not depend on frequency-observation availability;
- RATE receives validated pulse/time observations with explicit quality;
- selected low-power behavior and current consumption are accepted on target;
- any adopted workaround is traceable to a reproduced failure and explicit
  human decision;
- tests and bring-ups cover selected normal, boundary, low-power, and recovery
  paths.

## Phase 8: Minimum Measurement User Screens And Navigation

Objective:
- complete the minimum operator path for live flow measurement;
- integrate TTL/RATE and ACM/RATE into one bounded navigation model.

Dependencies:
- Phase 7 live TTL/RATE acquisition;
- completed Phase 6A presentation path;
- semantic input and runtime-owner contracts.

Decision gates:
- exact two-screen navigation and return behavior;
- ACM reset authorization, confirmation, and feedback;
- visible zero, absent, stale, invalid, and overflow behavior;
- refresh, activity, backlight, and pulse-indicator policy.

Legacy evidence:
- the implemented legacy user path placed TTL/RATE before ACM/RATE;
- broader legacy menus also included date/time, printing, Bluetooth, and
  optional temperature, but those are not measurement-minimum dependencies.

Exit criteria:
- TTL/RATE and ACM/RATE navigation is traceable to approved UI behavior;
- transitions, formatting, live updates, and approved reset behavior have
  focused tests and target validation;
- configuration screens and advanced capabilities remain out of scope.

## Phase 9: RTC, Calendar, And Associated Screens

Objective:
- add RTC/calendar as one complete vertical capability;
- include its user screen, configuration screen, and navigation in the same
  phase.

Dependencies:
- Phase 8 user navigation;
- approved time display and editing use cases;
- available CubeMX RTC configuration as the hardware authority.

Decision gates:
- RTC validity, initialization, format, edit, and recovery behavior;
- user-screen placement and return behavior;
- configuration authorization, save/cancel, and invalid-time feedback;
- backup-supply behavior before general Backup SRAM retention is introduced;
- which later reporting, logging, or communication functions may consume time.

Legacy evidence:
- legacy provides both a user date/time screen and date/time configuration;
- the legacy ticket formatter later consumes RTC date and time.

Exit criteria:
- valid and invalid time are distinguishable;
- approved date/time can be displayed and edited through reviewed navigation;
- formatting, calendar boundaries, save/cancel, reset, and recovery behavior
  have focused tests and target validation;
- RTC UI and configuration are complete inside this phase;
- general Backup SRAM and Flash ownership remain in Phases 11 and 12.

## Phase 10: Minimum Measurement Configuration Screens And Navigation

Objective:
- add the minimum authorized configuration path required by acquisition and
  measurement;
- validate and apply edits to active state before persistence is introduced.

Dependencies:
- Phase 8 operational navigation;
- approved editable measurement fields and ranges;
- current model separation between canonical configuration and derived values.

Reuse boundary:
- if Phase 9 is already implemented, reuse its reviewed
  configuration-navigation pattern without making RTC a prerequisite.

Minimum candidate scope:
- configuration entry or authorization screen;
- calibration factor K;
- ACM/TTL volume unit and resolution;
- RATE time base and resolution;
- navigation, edit buffer, validation, apply, and exit behavior.

Decision gates:
- exact minimum field set and authorization;
- save/cancel and invalid-edit behavior;
- whether any resolution shortcut remains on an operational screen;
- whether applied configuration remains volatile until Phases 11 and 12.

Legacy evidence:
- the implemented legacy setup contains password, factor, volume-unit, and
  time-unit screens;
- linearization, alarms, and unfinished settings do not enter the minimum slice
  automatically.

Exit criteria:
- the approved measurement settings can be edited and applied through focused
  configuration navigation;
- unit and factor changes update derived TTL, ACM, and RATE consistently;
- tests cover navigation, validation, cancel/apply, and boundary values;
- Backup SRAM, Flash, alarms, and advanced capability settings remain out of
  scope.

## Phase 11: Backup SRAM Retention

Objective:
- retain a human-approved minimal set of frequently changing runtime state,
  initially expected to fit within 1 KiB, while the backup supply remains
  valid.

Dependencies:
- stable canonical state from implemented release-scope capabilities, including
  RTC only when selected;
- approved reset classes and retention guarantees.

Decision gates:
- exact retained variables;
- integrity, version, initialization, and recovery;
- synchronization between live state and the backup domain;
- interaction with RTC backup-supply state without merging their ownership.

Risks:
- copying the legacy memory layout instead of retaining current canonical
  state;
- treating backup-powered retention as survival after loss of all supplies.

Exit criteria:
- retained state survives approved reset and supply cases;
- invalid retained data recovers according to approved policy;
- RTC and general retained state have explicit, non-overlapping ownership;
- Flash persistence remains separate.

## Phase 12: Flash Persistence

Objective:
- persist the approved low-change configuration that must survive loss of the
  backup supply.

Dependencies:
- Phase 10 measurement-configuration ownership;
- Phase 11 retained-state boundaries;
- approved factory/default and recovery behavior.

Decision gates:
- exact persisted variables and format versioning;
- write timing, atomicity, integrity, wear, and recovery;
- migration or factory reset behavior.

Risks:
- persisting derived values instead of canonical configuration;
- unnecessary writes or ambiguous recovery after interrupted updates.

Exit criteria:
- approved configuration survives full power removal;
- corruption and interrupted-write cases recover deterministically;
- Flash and Backup SRAM ownership does not overlap ambiguously.

## Phase 13: Temperature Measurement

Objective:
- add temperature as one complete vertical capability, from physical
  observation through product state and any required user/configuration
  screens and navigation.

Dependencies:
- stable runtime, UI, and persistence boundaries;
- human selection of the temperature capability and supported sensor.

Decision gates:
- sensor and electrical interface, range, resolution, accuracy, cadence, and
  energy budget;
- invalid/stale behavior and any compensation semantics;
- required operator view and editable parameters.

Legacy evidence:
- the extraction calls the optional sensor PT100 and sketches temperature and
  expansion-coefficient screens;
- preserved legacy firmware does not establish those sketches as a completed
  product implementation.

Exit criteria:
- temperature acquisition and quality are validated independently;
- approved temperature behavior is integrated with focused tests and target
  evidence;
- necessary temperature screens, configuration, navigation, and persistence
  are included in this phase rather than deferred to a generic UI phase.

## Phase 14: Bluetooth

Objective:
- add Bluetooth as one complete vertical communication capability, including
  only its required operator and configuration flows.

Dependencies:
- stable runtime, power, UI, and persistence boundaries;
- approved communication purpose, security boundary, and energy budget.

Decision gates:
- module and transport contract;
- connection roles, activation window, timeout, errors, and authorization;
- required screen, configuration, and diagnostic behavior.

Legacy evidence:
- legacy firmware provides a bounded Bluetooth connection window and uses the
  module as the transport for the printer;
- those control flows remain evidence, not current protocol authority.

Exit criteria:
- connection, transfer, timeout, recovery, and power behavior meet approved
  requirements;
- required Bluetooth UI/configuration/navigation is complete;
- printing is not implemented in this phase.

## Phase 15: Ticket Printing

Objective:
- add ticket printing as one complete vertical capability, including ticket
  content, Bluetooth transport integration, operator flow, configuration, and
  navigation.

Dependencies:
- Phase 9 RTC/calendar for approved ticket date/time;
- Phase 14 Bluetooth transport;
- stable ACM/TTL snapshot and persistence ownership.

Decision gates:
- ticket content, numbering, formatting, and privacy;
- print trigger, progress, timeout, retry, cancellation, and error behavior;
- whether RTC date/time is mandatory;
- printer compatibility and energy budget.

Legacy evidence:
- the preserved ticket contains ticket number, TTL, date, time, and ACM;
- the legacy user flow powers Bluetooth, connects to the printer, and sends the
  ticket in staged steps.

Exit criteria:
- approved ticket data is formed from coherent product state;
- Bluetooth transport and all operator-visible print states are validated;
- necessary print settings and navigation are complete inside this phase.

## Phase 16: Integral Validation, Power, And Product Behavior

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
