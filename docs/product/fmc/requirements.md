# FMC Product Requirements

## Purpose And Boundaries

This document owns FMC-320U product obligations. It records only the depth
needed to cover the complete product and prepare later focused slices.

Detailed observable sequences belong in [behavior.md](behavior.md). Visible and
operable interface decisions belong in
[user_interface.md](user_interface.md). Firmware architecture and implemented
APIs belong in code and public headers.

## Decision States And Evidence

Every substantive entry is marked `Accepted`, `Candidate`, `Unresolved`, or
`Deferred`. `Evidence` is listed separately as provenance and does not determine
acceptance. No formal requirement identifiers are introduced in this cut.

## Product And Lifecycle

This document applies to the FMC-320U product line.

**Accepted:** No detailed lifecycle obligation has been accepted in this
initial coverage.

**Candidate**

- The field product should operate as a battery-powered flow computer and
  initialize measurement, retained state, time, and selected peripherals into
  a coherent operating state.

Evidence:
- `docs/specs/fmc/use_cases.yaml`;
- roadmap power and startup concerns;
- legacy module inventory.

**Unresolved**

- Required startup ordering, failure indications, reset classes, wake behavior,
  and acceptable power limits are not yet approved.

## Measurement, Rate, And Totalization

**Accepted**

- The product shall distinguish accumulated (`ACM`) and total (`TTL`) roles.
- Both total roles shall accumulate the same accepted pulse deltas.
- ACM reset is user-allowed; TTL reset requires a privileged caller or flow.
- Visible ACM and TTL volume shall be derived from pulse-backed totals and the
  active measurement configuration.
- Rate shall be derived from a pulse delta and a positive elapsed-time window,
  using the active volume unit and active time base.
- The active measurement model shall keep calibration, volume unit, and rate
  time base explicit.

Evidence:
- `src/product/fmc/fmc_model.h`;
- `src/product/fmc/fmc_service.h`;
- `src/product/fmc/fmc_volume.h`;
- `src/product/fmc/fmc_rate.h`;
- `src/product/fmc/fmc_units.h`.

**Candidate**

- Physical pulse acquisition should supply bounded pulse deltas without losing
  accepted observations across normal operation or selected low-power states.
- Calibration and supported unit choices should be operator-configurable
  through an authorized flow.

Evidence:
- roadmap Phase 7;
- `docs/specs/fmc/use_cases.yaml`;
- legacy FMC and acquisition-related inventory.

**Unresolved**

- Pulse-loss policy, acquisition window ownership, numeric display limits,
  invalid measurement presentation, calibration-unit expansion, and exact
  reset authorization flows remain undecided.

## Operator Interaction

**Accepted**

- The product-domain input vocabulary shall distinguish DOWN, UP, ENTER, ESC,
  EXT_1, and EXT_2 identities.
- Mechanical-key events shall distinguish SHORT and LONG actions.
- External pushbuttons shall use SHORT events only in the current semantic
  input contract.
- Hardware pins, edges, timers, and RTOS types shall not be exposed as product
  input identities.

Evidence:
- `src/product/fmc/fmc_input.h`.

**Candidate**

- Accepted input should produce predictable visible consequences in operational
  and configuration contexts.

Evidence:
- `docs/specs/fmc/use_cases.yaml`;
- legacy user/setup inventory.

**Unresolved**

- External-button product functions, debounce, wake/backlight effects, complete
  key consequences, and navigation policy are not approved.

## Configuration, Persistence, And Time

**Candidate**

- Product configuration may include calibration, active volume unit, rate time
  base, date/time, alarms, and selected optional settings.
- Approved configuration and required total state should survive the reset and
  power-loss cases selected by the product.
- RTC validity should be checked before time is used for display, logs, or
  reporting.

Evidence:
- current model fields;
- roadmap Phase 7 and Phase 8 decision gates;
- `docs/specs/fmc/use_cases.yaml`;
- legacy setup, backup, flash, factory, and RTC inventories.

**Unresolved**

- The retained data set, storage layout, validation rules, factory defaults,
  recovery policy, save timing, RTC validity rules, and operator authorization
  are not yet accepted.

## Abnormal States And Recovery

**Candidate**

- The product may expose low-battery, measurement, storage, RTC, communication,
  and configuration validity states when they affect operator decisions.

Evidence:
- `docs/specs/fmc/use_cases.yaml`;
- roadmap risks and decision gates.

**Unresolved**

- Alarm priorities, latching, acknowledgement, reset semantics, overlays,
  degraded operation, and recovery behavior remain undefined.

## External And Optional Functions

**Deferred**

- Information logging and retention.
- Command and communication protocol behavior.
- Ticket printing and report content.
- Bluetooth connection and transfer workflow.
- PT100 or other optional sensing behavior.
- Product-facing diagnostics and service procedures.

Evidence:
- roadmap Phase 8;
- `docs/specs/fmc/use_cases.yaml`;
- legacy module inventory and migration ledger.

Each function requires a focused product decision before implementation. Its
presence in evidence does not make it mandatory for a release.
