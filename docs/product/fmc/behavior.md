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

**Unresolved:** Exact lifecycle states, startup ordering, sleep entry, wake
sources, and failure paths are not approved.

## Power-On And Startup

**Candidate:** Startup should establish valid runtime state before presenting
normal measurement information. Retained configuration, totals, and RTC data
should be validated before use.

Evidence:
- roadmap Phase 7;
- frozen legacy extraction boot and persistence sections;
- legacy `fm_init`, backup, flash, and RTC inventories.

**Unresolved:** First-programming behavior, invalid-retention recovery, startup
screen timing, skip behavior, and error presentation require focused review.

## Acquisition, Calculation, And Updates

**Accepted:** An accepted pulse delta is added to both ACM and TTL pulse-backed
totals. Visible volumes are derived from a snapshot of canonical state and the
active measurement configuration. Rate is calculated from a pulse delta and a
positive elapsed-time window.

Evidence:
- `src/product/fmc/fmc_service.h`;
- `src/product/fmc/fmc_volume.h`;
- `src/product/fmc/fmc_rate.h`.

**Candidate:** Fresh measurement data should eventually invalidate or refresh
the operator presentation without coupling acquisition to LCD details.

Evidence:
- `src/product/fmc/fmc_runtime.h`;
- roadmap Phase 6A and Phase 7.

**Unresolved:** Acquisition cadence, zero-flow recognition, smoothing,
presentation refresh timing, pulse-loss handling, and behavior across
low-power transitions remain undecided.

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
- roadmap Phase 7 and Phase 8;
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
- roadmap Phase 8;
- frozen legacy extraction;
- legacy module inventory.

Their future behavior must reference accepted requirements and must not be
reconstructed automatically from legacy control flow.
