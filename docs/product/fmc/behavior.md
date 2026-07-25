# FMC Observable Behavior

## Scope

This document owns cross-cutting behavior observable at the product boundary.
Product obligations belong in [requirements.md](requirements.md); interface
layout and operator flows belong in
[user_interface.md](user_interface.md).

Substantive statements are marked with a decision state. Evidence is identified
separately and is not a decision state.

## General Lifecycle

**Candidate:** The product moves through power-on, initialization, active
measurement, operator interaction, and selected recovery or low-power
conditions.

Evidence:
- roadmap;
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- legacy startup and runtime inventories.

**Unresolved:** Exact lifecycle states outside the accepted Phase 6A
presentation, sleep entry, wake sources, and non-LCD failure paths are not
approved.

## Power-On And Startup

**Accepted**

- After successful LCD initialization, presentation shows all segments,
  firmware version, and TTL/RATE in that order.
- All-segments and firmware-version views each remain for a nominal 3 seconds.
  Timing begins only after the corresponding view is presented successfully.
- A SHORT ESC has the same transition effect as the active startup timeout.
- TTL/RATE is presented immediately after the version transition and refreshed
  once per second.
- The sequence runs once after boot or reset and does not repeat merely because
  of wake or display reactivation.
- A failed view presentation is not confirmed, its nominal period does not
  begin, and automatic transition does not continue. Physical LCD content after
  a failed or partial write is indeterminate.
- A failed LCD initialization prevents the sequence from starting and is
  propagated through the existing product-app failure mechanism.

Evidence:
- reviewed Phase 6A product decision;
- `src/product/fmc/fmc_presentation.h`;
- `src/apps/product/main/fm_main.c`;
- frozen legacy startup evidence.

**Deferred:** Backlight behavior, retry and recovery policy, first-programming
behavior, invalid-retention recovery, and alternate diagnostics remain outside
Phase 6A.

## Acquisition, Calculation, And Updates

**Accepted:** An accepted pulse delta is added to both ACM and TTL pulse-backed
totals. Visible volumes are derived from a snapshot of canonical state and the
active measurement configuration. Rate is calculated from a pulse delta and a
positive elapsed-time window.

Evidence:
- `src/product/fmc/fmc_service.h`;
- `src/product/fmc/fmc_volume.h`;
- `src/product/fmc/fmc_rate.h`.

**Accepted:** The primary pulse counter begins from a known zero baseline.
Successive 16-bit observations form one modulo delta that is widened to the
current `uint64_t` runtime/service boundary. The first observation includes any
pulses counted since the counter was armed.

**Accepted:** Pulse accumulation is continuous through Run, Stop2, and their
transitions. Observations occur nominally about once per second, but individual
deltas do not need to represent exact one-second windows. Pulses may appear in
a later observation as long as cumulative delivery neither loses nor duplicates
any.

**Accepted:** An MCU reset ends the current accumulation interval. Pulses
arriving before the counter is rearmed may be discarded; rearming at zero begins
a new guaranteed interval.

Evidence:
- reviewed Phase 7A product decisions;
- `docs/specs/fmc/acquisition.md`;
- legacy `fmx.c::PulseUpdate()` and `fm_fmc.c::FM_FMC_PulseAdd()`.

**Accepted:** Phase 6A presentation consumes a coherent input containing TTL,
RATE, volume unit, RATE time base, and visible resolutions. Presentation
projects those values but does not calculate TTL, RATE, or the RATE observation
window.

Evidence:
- `src/product/fmc/fmc_runtime.h`;
- roadmap Phase 6A and Phase 7.

**Accepted:** The current stable presentation cadence is one second. The first
TTL/RATE values are shown on entry rather than waiting for the first periodic
refresh.

**Unresolved:** RATE-observation cadence, zero-flow recognition, smoothing, and
frequency-observation behavior across low-power transitions remain undecided.

**Deferred:** A future presentation cadence may become less frequent during
inactivity and return to one second after pulses, value changes, or operator
interaction. This is not current product behavior.

## Totalization And Resets

**Accepted:** ACM and TTL accumulate from the same accepted pulse source. Their
observable difference includes reset policy: ACM is user-resettable and TTL
requires a privileged flow. Reset authorization occurs before the primitive
total reset operation.

Evidence:
- `src/product/fmc/fmc_model.h`;
- `src/product/fmc/fmc_service.h`.

**Unresolved:** Confirmation prompts, password or service authorization,
persistence timing, reset feedback, and reset behavior during abnormal
conditions are not approved.

## Operator Input

**Accepted:** Product input preserves key identity and SHORT/LONG action
identity after hardware translation. The current runtime can accept that event
without assigning menu, wake, backlight, reset, or edit consequences.

Evidence:
- `src/product/fmc/fmc_input.h`;
- `src/product/fmc/fmc_runtime.h`.

**Candidate:** Later UI behavior should map semantic input to visible
consequences according to the active operational or configuration context.

**Unresolved:** Complete navigation, external-button consequences, debounce,
activity policy, and long-press meanings remain undecided.

## Configuration And Persistence

**Candidate:** Configuration changes should be reviewed in an edit context,
validated, applied to active measurement behavior, and persisted only according
to an approved save policy.

Evidence:
- current model separation between canonical measurement state and derived
  values;
- roadmap Phases 10 through 12;
- frozen legacy extraction and legacy setup evidence.

**Unresolved:** Edit buffers, save/cancel behavior, authorization, storage
ownership, atomicity, corruption recovery, and factory restore are not
approved.

## RTC And Time

**Candidate:** Valid time may be displayed and used to timestamp logs, reports,
or communication records. Invalid time should not be silently treated as valid.

Evidence:
- roadmap RTC decision gates;
- frozen legacy extraction;
- legacy RTC, logging, and printing inventories.

**Unresolved:** Validity threshold, formatting, edit behavior, backup-domain
recovery, time-zone assumptions, and failure indication remain undecided.

## Abnormal States And Recovery

**Candidate:** Conditions that compromise measurement, retained data, time,
power, or communication may require visible status and a defined recovery
path.

Evidence:
- roadmap risks;
- frozen legacy extraction alarms and gaps.

**Unresolved:** Alarm taxonomy, priority, persistence, acknowledgement,
operator override, and degraded operation are not approved.

## Optional Functions

**Deferred:** Logging, printing, Bluetooth, external command workflows, and
optional sensors remain recognized product domains without accepted detailed
behavior.

Evidence:
- roadmap Phases 13 through 15;
- frozen legacy extraction;
- legacy module inventory.

Their future behavior must reference accepted requirements and must not be
reconstructed automatically from legacy control flow.
