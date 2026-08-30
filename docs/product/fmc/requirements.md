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
acceptance. No formal requirement identifiers are introduced in this slice.

## Product And Lifecycle

This document applies to the FMC-320U product line.

**Accepted**

- After each boot or reset, a successfully initialized LCD shall present one
  startup sequence before normal operation.
- The startup sequence shall not repeat solely because the product wakes or
  the display is reactivated.
- A failed LCD initialization shall prevent the visible startup sequence from
  beginning.
- Successful presentation of the startup all-segments screen shall request
  display-backlight activation for the accepted bounded interval.
- The product shall support an activity-gated low-power lifecycle. After five
  seconds without newly counted primary pulses, acquisition may remove its
  periodic one-second deadline and remain inactive until new primary activity
  is detected, without discarding accumulated pulses.

**Candidate**

- The field product should operate as a battery-powered flow computer and
  initialize measurement, retained state, time, and selected peripherals into
  a coherent operating state.

Evidence:
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- roadmap power and startup concerns;
- legacy module inventory.

**Unresolved**

- Reset classes, broader startup dependency ordering, the mechanisms used to
  enter and leave activity-gated sleep, wake behavior outside presentation,
  and acceptable power limits are not yet approved.

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
- The Phase 8 user menu shall authorize ACM reset only from ACM/RATE, through
  LONG ENTER or EXT_2 SHORT, without confirmation. It shall not expose TTL
  reset.
- Visible ACM and TTL volume shall be derived from pulse-backed totals and the
  active measurement configuration.
- Rate shall be derived from a pulse delta and a positive elapsed-time window,
  using the active volume unit and active time base.
- The first frequency-observation realization shall support nonzero physical
  inputs from `1 Hz` through the accepted `1 kHz` primary-input limit.
  Frequencies below `1 Hz` have no guaranteed representative RATE observation,
  but shall remain eligible for normal pulse accumulation and totalization.
- Within an accepted frequency window, observed pulses shall match the
  independently observed physical pulses exactly and observed elapsed time
  shall be within `+/-1%` of an independent temporal reference.
- Runtime shall retain RATE value and explicit observation quality together.
  RATE mathematics shall execute only for a valid pulse/time window, and
  frequency-result events shall remain independent from ACM/TTL updates.
- The TTL/RATE screen shall use one common nonnumeric RATE representation for
  `UNAVAILABLE`, `STALE`, and `INVALID`. Their distinct quality remains
  available in runtime and diagnostics, and a retained non-valid RATE value
  shall not be presented as current. The common RATE-row pattern is
  `-------`; the normal TTL, RATE, active-unit legend, slash, and selected
  time-base indicator remain active.
- The initial live TTL/RATE screen shall use liters per second, following the
  current active measurement time base without a product-main override.
- Presentation shall support `SECOND`, `MINUTE`, `HOUR`, and `DAY` RATE time
  bases and their corresponding `S`, `M`, `H`, and `D` LCD indicators.
  `SECOND` is the initial live state; retaining the other bases does not make
  them operator-selectable.
- Entry into TTL/RATE, whether caused by timeout or SHORT ESC, shall present
  the latest coherent live snapshot in one accepted transition. A provisional
  or older snapshot shall not be shown and then corrected by a second write.
- The active measurement model shall keep calibration, volume unit, and rate
  time base explicit.
- Presentation shall receive accepted TTL and RATE values; it shall not own
  totalization, acquisition, filtering, or the RATE observation window.
- Presentation shall render the shared active-volume legend exhaustively as
  `--` for `CUSTOM`, `Lt` for `L`, `M3` for `M3`, `GL` for `GAL_US`, `BR` for
  `BBL_US`, `KG` for `KG`, and `MC` for `EQUIV_M3`. It shall not silently
  represent one configured unit as another.
- Configuration values shall be validated before they are applied to the
  active model or supplied to presentation. Presentation shall not duplicate
  configuration policy; it shall reject only unknown enum values or content
  that cannot be rendered by the active screen.
- ACM/RATE shall place ACM on the upper row and RATE on the lower row, reuse
  TTL/RATE unit, time-base, quality, formatting, and overflow rules, and show
  zero ACM as `0.0`.

Evidence:
- `docs/specs/fmc/acquisition.md`;
- `docs/specs/fmc/frequency_observation.md`;
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

**Unresolved:** Calibration-unit expansion and privileged TTL-reset
authorization remain undecided.

## Operator Interaction

**Accepted**

- The product-domain input vocabulary shall distinguish DOWN, UP, ENTER, ESC,
  EXT_1, and EXT_2 identities.
- Mechanical-key events shall distinguish SHORT and LONG actions.
- External pushbuttons shall use SHORT events only in the current semantic
  input contract.
- Hardware pins, edges, timers, and RTOS types shall not be exposed as product
  input identities.
- Normal operation shall expose TTL/RATE, ACM/RATE, PRINT, LOG_DOWNLOAD, and
  DATE_TIME in that order, with TTL/RATE as the initial user screen.
- Mechanical SHORT DOWN and UP shall traverse that order without wrapping;
  EXT_1 SHORT shall traverse forward and wrap.
- Every in-scope input shall have the consequence or explicit no-op defined in
  the accepted Phase 8 transition table.
- PRINT, LOG_DOWNLOAD, and DATE_TIME shall be visible inert placeholders until
  their respective functions are implemented. Their lower-row `OFF` text shall
  be right-aligned across the seven numeric positions.
- Each physical EXT_1 or EXT_2 actuation shall produce at most one SHORT event;
  each button shall remain disarmed at boot until its released level has been
  stable for 100 ms, whether it starts released or held.
- Every physical mechanical or external-button press edge shall request
  backlight activation before semantic filtering, including presses that are
  rejected, disarmed, or semantic no-ops. Activity shall not consume a
  semantic action and each handled request shall restart a fixed ten-second
  interval.
- Each accepted periodic presentation event shall cause exactly one
  presentation attempt of the active user-menu screen. Entry into each of the
  five user-menu screens shall cause an immediate presentation attempt,
  including entry into an inert placeholder. Immediate entry or reset
  presentation shall not suppress the next periodic attempt.
- Pulse observations shall drive one POINT indication consistently across the
  five user-menu screens. Startup pulse observations shall not affect its
  initial off state on TTL/RATE.

Evidence:
- `src/product/fmc/fmc_input.h`;
- `docs/product/fmc/user_interface.md`;
- `docs/product/fmc/behavior.md`;
- reviewed legacy user/setup evidence.

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

The accepted Phase 8 PRINT, LOG_DOWNLOAD, and DATE_TIME placeholders reserve
menu positions only; they do not accept or implement these deferred functions.
