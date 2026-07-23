# FMC User Interface

## Scope And Boundaries

This document owns the visible and operable FMC-320U experience. Product
obligations belong in [requirements.md](requirements.md); cross-cutting
observable behavior belongs in [behavior.md](behavior.md).

It does not own LCD physical mapping, driver APIs, firmware architecture, or a
complete UI state machine. Decision states are applied to substantive content;
evidence is recorded separately as provenance.

## Input And Output Surfaces

**Accepted**

- The product-domain input vocabulary includes mechanical DOWN, UP, ENTER, and
  ESC keys plus EXT_1 and EXT_2 external input identities.
- Mechanical keys distinguish SHORT and LONG actions; external identities use
  SHORT only in the current semantic contract.
- The visible output uses the custom segmented LCD represented by the current
  LCD technical specification.

Evidence:
- `src/product/fmc/fmc_input.h`;
- `docs/specs/lcd/lcd_true_source.yaml`.

**Unresolved**

- Physical availability and final product purpose of external buttons.
- Audible, printed, wireless, or service outputs that should count as operator
  interface surfaces.

## LCD Technical Authority

**Accepted:** `docs/specs/lcd/lcd_true_source.yaml` is the technical authority
for glass elements, symbols, digit groups, COM/SEG routing, RAM mapping, and
validated physical capabilities.

Evidence:
- `AGENTS.md`;
- `docs/specs/lcd/lcd_true_source.yaml`.

This product document may state what information the operator needs to see. It
must not duplicate COM/SEG tables, controller registers, memory bits, or
physical routing.

## Operational Interface

**Candidate:** The operational interface includes startup feedback, firmware
identity, TTL/RATE, ACM/RATE, time, status, and selected optional workflows.

Evidence:
- `docs/specs/fmc/use_cases.yaml`;
- legacy user-flow inventory.

**Unresolved:** Screen order, automatic transitions, return behavior, refresh
timing, decimal and overflow presentation, alarm overlays, backlight policy, and
complete key consequences are not approved.

## Configuration Interface

**Candidate:** A separate authorized interface may expose calibration, units,
rate time base, date/time, alarm thresholds, and selected optional settings.

Evidence:
- current model configuration fields;
- working inventory;
- legacy setup inventory.

**Unresolved:** Entry authorization, edit cursor behavior, validation feedback,
save/cancel rules, navigation, and exact configuration scope are not approved.

Configuration screens remain outside the Phase 6A slice.

## Semantic Input And Visible Consequences

**Accepted:** Product UI work consumes semantic key/action identity rather than
GPIO, edge, timer, HAL, or RTOS details.

Evidence:
- `src/product/fmc/fmc_input.h`;
- `src/apps/product/main/README.md`.

**Candidate:** The active UI context will assign visible meaning to each
accepted semantic event.

**Unresolved:** No complete key-to-transition table is accepted. Existing
working-inventory and legacy transitions remain evidence or candidates until a
focused UI slice reviews them.

## Superficial Interface Map

| Area | Visible purpose | Decision state | Current depth |
|---|---|---|---|
| Startup | Confirm display availability and identify firmware before normal operation | Candidate | Phase 6A target |
| Primary operation | Show TTL and rate | Candidate | Phase 6A target |
| Secondary operation | Show ACM and rate | Candidate | Surface |
| Time | Show date/time when valid | Candidate | Surface |
| Status and alarms | Indicate conditions requiring operator attention | Candidate | Surface |
| Configuration | Edit authorized product settings | Candidate | Surface |
| Printing | Initiate and report ticket output | Deferred | Domain only |
| Bluetooth | Expose a bounded connection workflow | Deferred | Domain only |
| Service and diagnostics | Support authorized inspection or tests | Unresolved | Domain only |

This map describes documentation coverage, not implemented firmware.

## Phase 6A Planned Depth

**Accepted:** The roadmap selects these visible states for the next focused
presentation slice:

- startup all-segments;
- firmware version;
- steady TTL/RATE.

Evidence:
- `docs/roadmaps/fmc_refactoring.md`.

Phase 6A is not active implementation in this context. The prior
`screen_spec_style.md` does not authorize creating individual screen files.

## Decisions Required Before Phase 6A

**Unresolved**

- Exact startup order, duration, ownership, and skip behavior.
- Firmware version source and rendered form.
- TTL/RATE labels, units, decimal policy, overflow, invalid values, and update
  cadence.
- Snapshot-to-semantic-LCD projection and adapter ownership.
- Which indicators are intentionally on, off, or preserved in each state.
- Controlled bring-up values and human acceptance observations.

These decisions should be deepened in the next approved documentation cut
without defining complete navigation or configuration behavior.

## Deferred Interface Functions

**Deferred:** Complete operational navigation, configuration screens, logging
views, printer flow, Bluetooth flow, optional PT100 presentation, complete
alarm behavior, and exact legacy UI reproduction.

Evidence:
- roadmap Phase 6B and Phase 8;
- working inventory and legacy evidence.
