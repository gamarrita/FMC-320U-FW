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

**Accepted**

- After each boot or reset, a successfully initialized LCD shall present one
  startup sequence before normal operation.
- The startup sequence shall not repeat solely because the product wakes or
  the display is reactivated.
- A failed LCD initialization shall prevent the visible startup sequence from
  beginning.
- Phase 6A shall not read, write, or otherwise impose backlight state.

**Candidate**

- The field product should operate as a battery-powered flow computer and
  initialize measurement, retained state, time, and selected peripherals into
  a coherent operating state.

Evidence:
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- roadmap power and startup concerns;
- legacy module inventory.

**Unresolved**

- Reset classes, broader startup dependency ordering, wake behavior outside
  presentation, and acceptable power limits are not yet approved.

## Measurement, Rate, And Totalization

**Accepted**

- The product shall distinguish accumulated (`ACM`) and total (`TTL`) roles.
- Both total roles shall accumulate the same accepted pulse deltas.
- Once the primary pulse counter is armed, every pulse it counts shall
  eventually be included exactly once in the pulse deltas applied to ACM and
  TTL, including pulses counted in Stop2 and across Run/Stop2 transitions.
- Pulse deltas need not correspond to exact one-second windows; delayed
  observation shall not by itself discard or duplicate accumulated pulses.
- After an MCU reset, pulses arriving before the pulse counter is rearmed may be
  discarded. Rearming the counter at zero begins a new guaranteed accumulation
  interval.
- ACM reset is user-allowed; TTL reset requires a privileged caller or flow.
- Visible ACM and TTL volume shall be derived from pulse-backed totals and the
  active measurement configuration.
- Rate shall be derived from a pulse delta and a positive elapsed-time window,
  using the active volume unit and active time base.
- The active measurement model shall keep calibration, volume unit, and rate
  time base explicit.
- Presentation shall receive accepted TTL and RATE values; it shall not own
  totalization, acquisition, filtering, or the RATE observation window.
- When the active visible volume unit is liters, presentation shall render its
  shared alphanumeric legend as `Lt`.

Evidence:
- `docs/specs/fmc/acquisition.md`;
- legacy `fmx.c::PulseUpdate()` and `fm_fmc.c::FM_FMC_PulseAdd()`;
- `src/product/fmc/fmc_model.h`;
- `src/product/fmc/fmc_service.h`;
- `src/product/fmc/fmc_volume.h`;
- `src/product/fmc/fmc_rate.h`;
- `src/product/fmc/fmc_units.h`.

**Candidate**

- Calibration and supported unit choices should be operator-configurable
  through an authorized flow.

Evidence:
- roadmap Phase 7;
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- legacy FMC and acquisition-related inventory.

**Unresolved**

- RATE-observation window ownership, absent or invalid measurement
  presentation, calibration-unit expansion, and exact reset authorization
  flows remain undecided.

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
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
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
- roadmap Phases 9 through 12 decision gates;
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
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
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
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
- roadmap Phases 13 through 15;
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- legacy module inventory and migration ledger.

Each function requires a focused product decision before implementation. Its
presence in evidence does not make it mandatory for a release.
