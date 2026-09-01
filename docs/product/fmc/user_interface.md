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
- EXT_1 and EXT_2 are physical active-low pushbuttons. Their current semantic
  SHORT action occurs on the accepted press transition, not on release.
- The visible output uses the custom segmented LCD represented by the current
  LCD technical specification.

Evidence:
- `src/product/fmc/fmc_input.h`;
- `docs/specs/lcd/lcd_true_source.yaml`;
- reviewed Phase 8 input decision.

**Deferred:** Audible, printed, wireless, or service outputs beyond the
Phase 8 placeholder screens require their own focused product decisions.

## LCD Technical Authority

**Accepted:** `docs/specs/lcd/lcd_true_source.yaml` is the technical authority
for glass elements, symbols, digit groups, COM/SEG routing, RAM mapping, and
validated physical capabilities.

Evidence:
- `AGENTS.md`;
- `docs/specs/lcd/lcd_true_source.yaml`.

## Screen Modeling And Contract Promotion

**Accepted:** Product requirements and accepted operator behavior remain owned
by the applicable documents under `docs/product/fmc/`. The LCD Studio project
is an executable design proposal for screen composition, example values, and
semantic transitions. It can initiate a screen contract, but neither the tool
nor an exported project becomes product authority merely by existing or
rendering successfully.

A modeled screen becomes an accepted detailed contract only after explicit
human review and a product document identifies the accepted project/schema
revision and the declarative details it owns. Requirements that the project
does not express remain in product documentation. In particular, simulated
variable overrides do not define operator editing, authorization, validation,
draft, apply, cancel, or persistence behavior.

Generated catalogs and future frame/transition fixtures are derived evidence.
Generated production firmware is not implied by accepting a screen model.

This product document may state what information the operator needs to see. It
must not duplicate COM/SEG tables, controller registers, memory bits, or
physical routing.

## Operational Interface

**Accepted:** The operational interface starts with startup feedback and
provisional firmware identity, then exposes this user-menu order:

1. TTL/RATE;
2. ACM/RATE;
3. PRINT;
4. LOG_DOWNLOAD;
5. DATE_TIME.

TTL/RATE and ACM/RATE are live measurement screens. The remaining three are
visible, inert placeholders so the complete Phase 8 menu can be traversed in
`product/main` before their functions are implemented.

Evidence:
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- legacy user-flow inventory.

Temperature, alarm overlays, printing, logged-data transfer, and date/time
behavior beyond these placeholders are outside Phase 8.

## Configuration Interface

**Candidate:** A separate authorized interface may expose calibration, units,
rate time base, date/time, alarm thresholds, and selected optional settings.

Evidence:
- current model configuration fields;
- frozen legacy extraction;
- legacy setup inventory.

**Unresolved:** Entry authorization, edit cursor behavior, validation feedback,
save/cancel rules, navigation, and exact configuration scope are not approved.

Configuration screens remain outside Phase 8.

## Semantic Input And Visible Consequences

**Accepted:** Product UI work consumes semantic key/action identity rather than
GPIO, edge, timer, HAL, or RTOS details.

Evidence:
- `src/product/fmc/fmc_input.h`;
- `src/apps/product/main/README.md`.

**Accepted:** The Phase 8 startup and user-menu consequences are defined below.
Configuration meanings remain deferred to their configuration slice.

## Superficial Interface Map

| Area | Visible purpose | Decision state | Current depth |
|---|---|---|---|
| Startup | Confirm display availability and identify firmware before normal operation | Accepted | Phase 6A implemented |
| Primary operation | Show TTL and rate | Accepted | Phase 7 live |
| Secondary operation | Show ACM and rate | Accepted | Phase 8 contract |
| Time | Expose a DATE_TIME menu position | Accepted | Phase 8 inert placeholder |
| Status and alarms | Indicate conditions requiring operator attention | Candidate | Surface |
| Configuration | Edit authorized product settings | Candidate | Surface |
| Printing | Expose a PRINT menu position | Accepted | Phase 8 inert placeholder; workflow deferred |
| Logged-data download | Expose a LOG_DOWNLOAD menu position | Accepted | Phase 8 inert placeholder; transport and workflow deferred |
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
- POINT is physically forced on in this view solely as part of the segment
  check. It does not represent pulse activity during startup.
- Writing numeric eights alone does not satisfy this state.
- The state serves both as a visual check and an unambiguous startup signal.
- Its nominal duration is 3 seconds, beginning after successful presentation.
- SHORT ESC advances to firmware version exactly as timeout does.
- Every other SHORT or LONG input is a no-op for startup navigation. Any
  physical press still applies the accepted backlight activity policy before
  semantic filtering.

Physical element identity and mapping remain owned by
`docs/specs/lcd/lcd_true_source.yaml`.

### Firmware Version

**Accepted**

- The top numeric row is empty.
- The bottom numeric row shows `00.01.00`.
- The alphanumeric field shows `B0`.
- Every screen-specific standalone indicator, including POINT, is off.
- The value is an unpublished Phase 6A dummy, not a released firmware version.
- Its nominal duration is 3 seconds, beginning after successful presentation.
- SHORT ESC advances to TTL/RATE exactly as timeout does.
- Every other SHORT or LONG input is a no-op for startup navigation. Any
  physical press still applies the accepted backlight activity policy before
  semantic filtering.

Released version meaning, LCD encoding limits, and tag traceability belong to
[Firmware Versioning](../../project/firmware_versioning.md).

### TTL/RATE

**Accepted**

- The top row shows TTL and the bottom row shows RATE.
- Both values are non-negative, right-aligned, rounded to one decimal, and use
  blank unused positions.
- Zero is valid and displays as `0.0`.
- TTL and RATE share the active volume unit. The initial state uses liters.
- Presentation supports every model RATE time base and selects exactly one of
  the `S`, `M`, `H`, or `D` indicators. Second is the initial live state; the
  other bases are technical capabilities and are not yet operator-selectable.
- The shared alphanumeric field represents the active volume unit for both
  rows according to this accepted table:

  | Model value | Visible legend |
  |---|---|
  | `CUSTOM` | `--` |
  | `L` | `Lt` |
  | `M3` | `M3` |
  | `GAL_US` | `GL` |
  | `BBL_US` | `BR` |
  | `KG` | `KG` |
  | `EQUIV_M3` | `MC` |

  No configured unit is silently represented as another unit.
- `TTL`, `RATE`, slash, and the selected time-base indicator remain active,
  including during visual overflow. All unrelated screen-specific indicators
  remain off; POINT follows its transverse activity rule.
- The initial controlled values are `1234.5` TTL and `12.3` RATE. They are valid
  provisional inputs, not absent or invalid states.
- Values are shown immediately on entry and presented again once per second.
- Phase 7 live integration replaces those controlled inputs atomically on the
  first TTL/RATE presentation. Timeout and SHORT ESC transitions both use the
  latest coherent live snapshot and do not briefly show an older snapshot
  before refreshing it.

Presentation receives a coherent snapshot containing accepted TTL and RATE,
unit, time base, and resolutions. Configuration validity belongs to the
configuration load/apply boundary; presentation translates the accepted model
values exhaustively and does not calculate totals, flow, or the RATE
observation window.

When a rounded value does not fit, the row keeps the least significant digits
that fit, including one fractional digit, and discards the most significant
digits visually. It does not display `E`, saturate, or trigger another action.

**Accepted:** `UNAVAILABLE`, `STALE`, and `INVALID` use one common nonnumeric
RATE representation. Their distinct quality remains available to runtime and
diagnostics, but the TTL/RATE screen does not distinguish them and does not
show a retained non-valid RATE value as current. The seven numeric positions
of the RATE row show `-------`. The normal `TTL`, `RATE`, active-unit legend,
slash, and selected time-base indicator remain active.

Visible representation of internal formatting errors is deferred.

## Phase 8 User Menu

### Navigation

**Accepted**

- TTL/RATE is the initial user-menu screen after startup.
- Mechanical SHORT DOWN moves one position forward and stops at DATE_TIME.
- Mechanical SHORT UP moves one position backward and stops at TTL/RATE.
- EXT_1 SHORT moves one position forward and wraps from DATE_TIME to TTL/RATE.
- Mechanical SHORT ENTER and SHORT ESC are no-ops on every Phase 8 user
  screen.
- EXT_2 SHORT is a no-op except for the ACM reset defined below.
- Every unassigned mechanical LONG action is a no-op. LONG ENTER resets ACM
  only while ACM/RATE is active.
- One accepted input event causes at most one transition or one reset request.

The complete Phase 8 startup and user-menu input table is:

| Active view | SHORT DOWN | SHORT UP | SHORT ENTER | SHORT ESC | EXT_1 SHORT | EXT_2 SHORT | LONG ENTER | Other LONG |
|---|---|---|---|---|---|---|---|---|
| ALL_SEGMENTS | No-op | No-op | No-op | FIRMWARE_VERSION | No-op | No-op | No-op | No-op |
| FIRMWARE_VERSION | No-op | No-op | No-op | TTL/RATE | No-op | No-op | No-op | No-op |
| TTL/RATE | ACM/RATE | No-op | No-op | No-op | ACM/RATE | No-op | No-op | No-op |
| ACM/RATE | PRINT | TTL/RATE | No-op | No-op | PRINT | Reset ACM | Reset ACM | No-op |
| PRINT | LOG_DOWNLOAD | ACM/RATE | No-op | No-op | LOG_DOWNLOAD | No-op | No-op | No-op |
| LOG_DOWNLOAD | DATE_TIME | PRINT | No-op | No-op | DATE_TIME | No-op | No-op | No-op |
| DATE_TIME | No-op | LOG_DOWNLOAD | No-op | No-op | TTL/RATE | No-op | No-op | No-op |

LONG ESC, LONG UP, and LONG DOWN do not enter configuration or change
resolution in Phase 8. Those meanings belong to Phase 10.

### ACM/RATE

**Accepted**

- The upper row shows ACM and the lower row shows RATE.
- The upper-row `ACM` indicator is active. `TTL` and the lower-row `ACM`
  indicator are off.
- RATE, slash, time-base indicator, shared active-unit alpha field, numeric
  formatting, visual overflow, and RATE-quality representation match
  TTL/RATE.
- ACM uses the same active volume unit and visible resolution as TTL.
- Zero ACM displays as `0.0`.
- The screen is presented immediately on entry from a fresh coherent snapshot
  and again once per accepted one-second presentation cycle.

The LCD's upper and lower ACM indicators remain independent BSP capabilities.
Their use is selected by each screen composition; there is no global rule that
forbids the lower indicator.

### ACM Reset

**Accepted:** LONG ENTER or EXT_2 SHORT while ACM/RATE is active directly
requests one ACM reset. There is no confirmation, cancellation, password,
transient state, or separate success screen. After the reset, ACM/RATE is
presented immediately from a fresh snapshot and normally shows `0.0`.

The reset does not clear the hardware pulse counter or acquisition baselines.
A pulse delta pending at the reset boundary, or accepted afterward, may make
the next periodic ACM value nonzero. TTL is never reset from the Phase 8 user
menu.

### Deferred-Function Placeholders

**Accepted:** These screens are static, inert, and have no peripheral,
communication, RTC, print, configuration, workflow, or timeout side effects:

| Screen | Alpha field | Upper row | Lower row |
|---|---|---|---|
| PRINT | `PR` | Empty | `OFF` |
| LOG_DOWNLOAD | `LD` | Empty | `OFF` |
| DATE_TIME | `DT` | Empty | `OFF` |

`OFF` is right-aligned across the seven lower numeric positions. The exact
semantic lower-row text is `"    OFF"` with four leading spaces.

All screen-specific indicators are off. The user-menu POINT activity indicator
remains governed by [behavior.md](behavior.md).

Entry into each placeholder causes an immediate presentation attempt. Its
static content is also presented during the normal user-menu periodic cycle so
the POINT state remains current.

`LOG_DOWNLOAD` names the operator purpose. Bluetooth may later be one transport
for that function, but is not the screen identity because printing may also
use Bluetooth.

### Presentation Failures

**Accepted**

- LCD initialization failure prevents the sequence from starting.
- A view is considered presented only after its LCD write completes
  successfully.
- A failed presentation does not start its nominal duration and does not
  advance automatically.
- Physical content after a partial or failed write is indeterminate.

Retry, recovery, logging, and alternate diagnostics remain implementation
decisions for a later selected slice.

### Backlight

**Accepted:** Successful presentation of the startup all-segments screen and
every physical mechanical or external-button press request backlight
activation. The request is made before semantic filtering, so a rejected,
disarmed, or otherwise ineffectual press still illuminates the display. Each
handled request restarts the same ten-second inactivity interval. A semantic
action, when one exists, is not consumed only to illuminate the display.
Detailed timing and failure behavior are owned by [behavior.md](behavior.md).

## Deferred Interface Functions

**Deferred:** Configuration screens, actual logged-data download, printer
flow, date/time content and editing, optional PT100 presentation, complete
alarm behavior, and exact legacy UI reproduction.

Evidence:
- roadmap Phases 8 through 10 and Phases 13 through 15;
- frozen legacy extraction and other legacy evidence.
