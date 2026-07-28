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

**Accepted:** The target acquisition lifecycle distinguishes inactivity
without a periodic acquisition deadline from active observation at a nominal
one-second cadence with Stop2 available between deadlines. Five seconds
without newly counted primary pulses permits the inactive state. The first
integration retains the current one-second refresh while the mechanisms for
entering and leaving indefinite-duration sleep remain unselected.

Evidence:
- roadmap;
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- legacy startup and runtime inventories.

**Unresolved:** Exact lifecycle states outside the accepted Phase 6A
presentation, sleep-entry and activity-detection mechanisms, other wake
sources, and non-LCD failure paths are not approved.

## Power-On And Startup

**Accepted**

- After successful LCD initialization, presentation shows all segments,
  firmware version, and TTL/RATE in that order.
- All-segments and firmware-version views each remain for a nominal 3 seconds.
  Timing begins only after the corresponding view is presented successfully.
- A SHORT ESC has the same transition effect as the active startup timeout.
- TTL/RATE is presented immediately after the version transition and refreshed
  once per second. Entry caused by timeout or SHORT ESC uses the latest
  coherent live snapshot in one accepted presentation, without an intermediate
  provisional TTL/RATE frame.
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

**Deferred:** LCD retry and recovery policy, first-programming behavior,
invalid-retention recovery, and alternate diagnostics remain outside Phase 8.

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

**Accepted:** The first frequency-observation realization establishes its
baseline when acquisition starts, completes consecutive windows at a nominal
one-second cadence, uses actual elapsed time, and makes each completed window
available without multi-window averaging or smoothing.

**Accepted:** A complete, temporally admissible frequency window containing no
pulses is a valid zero observation and may make RATE zero immediately. It does
not end active observation; only the separate five-second inactivity condition
based on newly counted primary pulses permits the future inactive state.

**Accepted:** Frequency quality is `UNAVAILABLE` when no complete usable
pulse/time window currently exists. It is not zero flow and does not provide a
numeric RATE.

**Accepted:** Frequency quality is `STALE` when a previous valid window was not
replaced within its expected deadline. No new RATE is calculated; a retained
previous value is not current and remains explicitly stale. Intentional
inactivity has no frequency deadline and is not stale.

**Accepted:** Frequency quality is `INVALID` when a supplied physical sample
cannot be trusted. It produces no RATE window and ends the active frequency
baseline; one trusted sample establishes a new unavailable baseline before a
later trusted sample can restore valid observation.

**Accepted:** During active observation, the normal one-second ThreadX sleep
may transparently enter Stop2 between deadlines. Accepted target evidence
shows frequency windows and their monotonic timestamps continuing across those
normal Run/Stop2 transitions. The mechanism for entering and leaving the
future indefinite inactive sleep remains unselected.

**Accepted:** The temporary all-segments and firmware-version startup views are
presented on entry and are not periodically re-presented. Once the user menu
begins, processing each accepted one-second presentation event causes exactly
one presentation attempt of the active user view. Live measurement screens are
additionally updated from a fresh snapshot. Entry into every user-menu screen,
including an inert placeholder, causes an immediate presentation attempt, and
ACM/RATE is presented immediately after an accepted reset. An immediate
presentation is independent from and does not suppress the next periodic
attempt. Navigation never shows a retained frame from the preceding screen
before the fresh active frame on a successful write.

Periodic presentation does not restart a startup duration or the backlight
interval.

**Accepted:** POINT is a pulse-activity witness across the five user-menu
screens only:

- its logical state starts off on first entry to TTL/RATE;
- each accepted periodic pulse observation while the user menu is active with
  `pulse_delta > 0` toggles it once, regardless of the number of pulses in that
  observation;
- an accepted observation with `pulse_delta == 0` sets it off;
- the resulting state is applied to that cycle's active user frame;
- it does not react per physical edge and does not replay individual pulses.

During startup, POINT is physically on only as part of the all-segments check
and is off on the firmware-version view. Pulse observations before the user
menu begins do not alter the logical state that starts on TTL/RATE.

**Deferred:** A future presentation cadence may become less frequent during
inactivity and return to one second after activity. This is not Phase 8
behavior.

## Totalization And Resets

**Accepted:** ACM and TTL accumulate from the same accepted pulse source. Their
observable difference includes reset policy: ACM is user-resettable and TTL
requires a privileged flow. Reset authorization occurs before the primitive
total reset operation.

Evidence:
- `src/product/fmc/fmc_model.h`;
- `src/product/fmc/fmc_service.h`.

**Accepted:** While ACM/RATE is active, LONG ENTER and EXT_2 SHORT each
authorize one direct ACM reset. There is no confirmation or cancellation
state. The visible ACM/RATE frame is refreshed immediately from the
post-reset snapshot. Resetting the runtime total does not clear the hardware
counter or observation baseline; a pending or later accepted pulse delta can
make ACM nonzero at the next periodic presentation. TTL reset is unavailable
from the user menu.

**Deferred:** Privileged TTL reset, persistence timing, and reset behavior
during future abnormal states belong to later focused work.

## Operator Input

**Accepted:** Product input preserves key identity and SHORT/LONG action
identity after hardware translation. Product-main serializes accepted input
and gives it directly to the active UI context; measurement runtime does not
retain passive menu input.

Evidence:
- `src/product/fmc/fmc_input.h`;
- `docs/product/fmc/user_interface.md`.

**Accepted:** EXT_1 and EXT_2 generate at most one SHORT action for one
physical press. The first accepted falling transition acts immediately. Each
button remains independently disarmed until a rising transition has remained
released for 100 ms; falling during that interval cancels the release
candidate. Initialization samples each button independently. An initially high
button starts the same 100 ms stable-release interval before arming. An
initially low button generates no action and remains disarmed until a later
rising transition starts that interval. Only a falling transition after
arming generates SHORT and disarms the button again. External buttons do not
generate LONG or repeat actions.

**Accepted:** Successful presentation of the all-segments startup screen
requests backlight activation. Every valid physical mechanical or
external-button press makes the same request without consuming its semantic
action, even when the active UI assigns that action no visible consequence.
Each request first commits a valid ten-second one-shot expiry and then turns
the backlight on. Periodic refresh, pulse observations, and internal UI changes
do not extend the interval. A stale expiry cannot turn off a newer activation
interval.

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
