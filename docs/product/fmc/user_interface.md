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

**Accepted:** The implemented Phase 6A operational interface includes startup
feedback, provisional firmware identity, and a steady TTL/RATE view.

**Candidate:** Later operational depth may include ACM/RATE, time, status, and
selected optional workflows.

Evidence:
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- legacy user-flow inventory.

**Unresolved:** Later screen order, return behavior, alarm overlays, backlight
policy, and complete key consequences are not approved.

## Configuration Interface

**Candidate:** A separate authorized interface may expose calibration, units,
rate time base, date/time, alarm thresholds, and selected optional settings.

Evidence:
- current model configuration fields;
- frozen legacy extraction;
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
frozen-extraction and legacy transitions remain evidence or candidates until a
focused UI slice reviews them.

## Superficial Interface Map

| Area | Visible purpose | Decision state | Current depth |
|---|---|---|---|
| Startup | Confirm display availability and identify firmware before normal operation | Accepted | Phase 6A implemented |
| Primary operation | Show TTL and rate | Accepted | Phase 6A implemented |
| Secondary operation | Show ACM and rate | Candidate | Surface |
| Time | Show date/time when valid | Candidate | Surface |
| Status and alarms | Indicate conditions requiring operator attention | Candidate | Surface |
| Configuration | Edit authorized product settings | Candidate | Surface |
| Printing | Initiate and report ticket output | Deferred | Domain only |
| Bluetooth | Expose a bounded connection workflow | Deferred | Domain only |
| Service and diagnostics | Support authorized inspection or tests | Unresolved | Domain only |

This map describes documentation coverage, not implemented firmware.

## Phase 6A Presentation

**Accepted:** After each boot or reset, successful LCD initialization starts
this sequence:

1. all-segments check;
2. provisional firmware version;
3. steady TTL/RATE.

Evidence:
- reviewed Phase 6A decision;
- `docs/roadmaps/fmc_refactoring.md`;
- frozen legacy startup evidence.

The sequence is owned by product presentation. It does not belong to the LCD
driver. It runs once after boot or reset, not after wake or display
reactivation.

### All-Segments Check

**Accepted**

- Activate every LCD segment controllable by software at the same time.
- Coverage includes numeric digits, decimal points, both alphanumeric
  characters, legends, and indicators.
- Writing numeric eights alone does not satisfy this state.
- Backlight is excluded and remains untouched.
- The state serves both as a visual check and an unambiguous startup signal.
- Its nominal duration is 3 seconds, beginning after successful presentation.
- SHORT ESC advances to firmware version exactly as timeout does.

Physical element identity and mapping remain owned by
`docs/specs/lcd/lcd_true_source.yaml`.

### Firmware Version

**Accepted**

- The top numeric row is empty.
- The bottom numeric row shows `00.01.00`.
- The alphanumeric field shows `B0`.
- Every standalone indicator is off.
- The value is an unpublished Phase 6A dummy, not a released firmware version.
- Its nominal duration is 3 seconds, beginning after successful presentation.
- SHORT ESC advances to TTL/RATE exactly as timeout does.

Released version meaning, LCD encoding limits, and tag traceability belong to
[Firmware Versioning](../../project/firmware_versioning.md).

### TTL/RATE

**Accepted**

- The top row shows TTL and the bottom row shows RATE.
- Both values are non-negative, right-aligned, rounded to one decimal, and use
  blank unused positions.
- Zero is valid and displays as `0.0`.
- TTL uses liters and RATE uses liters per minute.
- The shared alphanumeric field shows `Lt` for both rows.
- `TTL`, `RATE`, slash, and minute indicators remain active, including during
  visual overflow. All unrelated indicators remain off.
- The initial controlled values are `1234.5` TTL and `12.3` RATE. They are valid
  provisional inputs, not absent or invalid states.
- Values are shown immediately on entry and presented again once per second.

Presentation receives a coherent snapshot containing accepted TTL and RATE,
unit, time base, and resolutions. It does not calculate totals, flow, or the
RATE observation window.

When a rounded value does not fit, the row keeps the least significant digits
that fit, including one fractional digit, and discards the most significant
digits visually. It does not display `E`, saturate, or trigger another action.

**Deferred:** Visible representation of absent values, invalid values, and
internal formatting errors.

### Presentation Failures

**Accepted**

- LCD initialization failure prevents the sequence from starting.
- A view is considered presented only after its LCD write completes
  successfully.
- A failed presentation does not start its nominal duration and does not
  advance automatically.
- Physical content after a partial or failed write is indeterminate.

Retry, recovery, logging, and alternate diagnostics remain implementation
decisions for a later selected cut.

### Backlight

**Deferred:** Phase 6A neither reads nor changes backlight state and does not
couple it to startup timing.

## Deferred Interface Functions

**Deferred:** Complete operational navigation, configuration screens, logging
views, printer flow, Bluetooth flow, optional PT100 presentation, complete
alarm behavior, and exact legacy UI reproduction.

Evidence:
- roadmap Phase 6B and Phase 8;
- frozen legacy extraction and other legacy evidence.
