# FMC Legacy User Navigation Analysis

## Purpose

Reconstruct the legacy user-menu navigation relevant to Phase 8 and identify
the evidence conflicts that required explicit current-product decisions.

This document is historical analysis. It is not a current product contract, a
screen specification, an implementation design, or permission to reproduce the
legacy menu.

## Current Review Disposition

Phase 8-0A used this analysis to expose the legacy choices and gaps. A
subsequent sequential human review resolved the in-scope current-product
decisions. Their authorities are:

- `docs/product/fmc/user_interface.md` for screens and navigation;
- `docs/product/fmc/behavior.md` for temporal behavior;
- `docs/product/fmc/requirements.md` for product obligations;
- `docs/workflow/fmc_legacy_coverage.md` for granular legacy disposition.

The historical reconstruction below remains unchanged evidence unless a
paragraph explicitly names the reviewed current disposition.

## Sources Reviewed

Primary historical use-case source:
- `legacy/specs/fmc/use_cases.docx`.

Frozen structured extraction:
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`.

Implemented legacy evidence:
- `legacy/source/FLOWMEET/fm_user.c`;
- `legacy/source/FLOWMEET/fm_user.h`;
- `legacy/source/FLOWMEET/fmx.c`;
- `legacy/source/FLOWMEET/fmx.h`.

Repository analysis and current boundaries:
- `legacy/analysis/module_inventory.md`;
- `legacy/analysis/migration_ledger.md`;
- `docs/product/fmc/user_interface.md`;
- `docs/workflow/fmc_legacy_coverage.md`;
- `src/product/fmc/fmc_input.h`;
- `src/product/fmc/fmc_runtime.h`;
- `src/product/fmc/fmc_presentation.h`;
- `src/apps/product/main/README.md`.

The Word source, frozen extraction, and legacy code are evidence. Agreement
between them does not make a behavior an accepted current-product decision.

## Evidence Classification

- The Word document records intended historical behavior but contains internal
  omissions and inconsistencies.
- The frozen YAML is the preferred structured legacy reference. It may contain
  normalized interpretations and must not be silently corrected.
- `fm_user.c` records implemented behavior at the preserved snapshot. It also
  mixes navigation, presentation, LCD access, reset actions, communication,
  timers, and ThreadX interaction.
- `fmx.c` records the surrounding event, refresh, backlight, and pulse-indicator
  mechanisms. Those mechanisms are not automatically current architecture or
  product policy.
- Current product documents define accepted behavior. Public headers define
  implemented boundaries; at the close of 8-0A they did not yet implement the
  later operational navigation.

## Legacy User-Menu Structure

The implemented `menu_user_t` order is:

```text
POWER_RESET
-> VERSION
-> TTL_RATE
-> ACM_RATE
-> PRINT_ACM
-> BLUETOOTH
-> DATE_TIME
-> END
```

The normal forward path skips the unfinished Bluetooth screen from
`PRINT_ACM`, so the implemented operational traversal is effectively:

```text
TTL_RATE
-> ACM_RATE
-> PRINT_ACM
-> DATE_TIME
-> TTL_RATE
```

The review did not adopt a reduced two-screen model. The current Phase 8
contract keeps a traversable five-position skeleton:

```text
TTL_RATE -> ACM_RATE -> PRINT -> LOG_DOWNLOAD -> DATE_TIME
```

This order is a current decision, not an inference from the conflicting legacy
flows. Temperature is excluded and the last three positions are inert
placeholders.

## Phase 8 Screen Evidence

### TTL/RATE

The Word source, extraction, and code broadly agree that:
- TTL occupies the upper row and RATE the lower row;
- SHORT DOWN advances to ACM/RATE;
- SHORT UP, ESC, and ENTER have no implemented navigation effect;
- LONG ESC enters the configuration flow;
- LONG DOWN changes RATE resolution;
- LONG UP changes shared ACM/TTL resolution;
- EXT_1 advances in the broader user menu.

The configuration entry and resolution shortcuts cross into Phase 10
decisions. Their legacy presence does not authorize them in Phase 8.

### ACM/RATE

The Word source, extraction, and code broadly agree that:
- ACM occupies the upper row and RATE the lower row;
- SHORT UP returns to TTL/RATE;
- SHORT DOWN and EXT_1 advance to the next broader legacy screen;
- LONG ENTER and EXT_2 reset ACM immediately;
- SHORT ESC names an unimplemented flow-alarm reset;
- other listed key actions have no material implemented effect.

The reviewed current disposition keeps printing as the next visible position
but only as an inert placeholder. Mechanical navigation is bounded; EXT_1
provides the cyclic forward traversal.

## Reset Evidence

Legacy `fm_user.c` invokes the ACM reset directly after LONG ENTER or EXT_2 and
then refreshes ACM/RATE. It has no confirmation state, cancellation path,
authorization step, timeout, or explicit success feedback.

The reviewed current disposition deliberately keeps direct reset without a
confirmation or transient state: LONG ENTER and EXT_2 each request one ACM
reset only while ACM/RATE is active. Product-main later consumes the request,
resets runtime ACM, obtains a fresh snapshot, and immediately presents
ACM/RATE. TTL and acquisition counter/baselines are not reset.

## Refresh, Activity, Backlight, And Pulse Indication

Legacy `fmx.c`:
- serializes menu events in one ThreadX thread;
- refreshes menu content after timeouts or explicit refresh requests;
- turns on the backlight for a range of key events and restarts a timer;
- toggles the LCD point indicator when a nonzero pulse delta is observed.

The Word source associates user interaction with a backlight interval and
describes the pulse indicator as blinking when primary pulses are received.
The frozen extraction nevertheless records general user-screen wake and
backlight behavior as unresolved.

The evidence did not establish one unambiguous current policy for:
- which semantic actions count as operator activity;
- whether a first action only wakes the display or also navigates;
- the exact backlight duration and restart rule;
- whether backlight state affects low-power entry;
- whether pulse indication represents physical edges, accepted counted pulses,
  nonzero observation windows, or another event;
- indicator cadence when pulses are slower or faster than visible refresh.

The current Phase 8 contracts now define these policies independently of LCD
physical mapping: a valid physical press both acts and restarts a ten-second
backlight interval; POINT is driven once per accepted periodic pulse
observation while the user menu is active; the five user screens participate
in the one-second presentation cycle while temporary startup views do not.

## Evidence Conflicts And Gaps

| Topic | Evidence | Reviewed current disposition |
|---|---|---|
| Reduced successor from ACM/RATE | Legacy DOWN/EXT_1 advances to printing | PRINT remains visible as an inert third position; mechanical traversal is bounded and EXT_1 is cyclic |
| ACM reset | Legacy performs an immediate destructive action | Accepted directly on ACM/RATE through LONG ENTER or EXT_2, with immediate refresh and no confirmation |
| External buttons | Legacy assigns navigation/reset actions; current product app does not deliver them | EXT_1 cycles forward; EXT_2 resets only on ACM/RATE; press-edge debounce is explicitly defined |
| LONG shortcuts | Legacy enters setup or changes resolution from TTL/RATE | Deferred to Phase 10; they are no-ops in Phase 8 |
| Flow-alarm reset | Named but marked unimplemented | Alarms remain outside the minimum measurement slice |
| Backlight | Word intent, frozen unresolved entry, and timer-coupled code do not form one contract | Every valid physical press and successful all-segments startup presentation request activation; normal operation rearms a safe fixed ten-second interval, while unexpected ThreadX timer status follows the common fatal app path |
| Pulse indicator | Source wording and implementation observation point differ in precision | POINT toggles once for a nonzero accepted observation and turns off for a zero observation across the five user-menu screens; startup uses no functional pulse witness |
| Later-screen returns | Word/YAML and code differ on date/time return timing and target | DATE_TIME is an inert last position; DOWN is a no-op, UP returns to LOG_DOWNLOAD, and EXT_1 wraps to TTL/RATE |

## Current Architecture Boundary

The implementation present during 8-0A provides:
- semantic mechanical key identity with SHORT and LONG actions;
- serialized delivery to the product owner thread;
- coherent ACM, TTL, and RATE snapshots;
- a primitive authorized ACM reset event;
- startup and live TTL/RATE semantic presentation;
- LCD support for ACM indicators.

It deliberately does not yet provide:
- an operational-navigation owner;
- a complete input-to-transition table;
- an ACM/RATE semantic frame;
- current direct-reset request routing and immediate feedback;
- external-button delivery in `product/main`;
- accepted activity, backlight, or pulse-indicator behavior.

The accepted later implementation boundary replaces `fmc_presentation` with
the RTOS-neutral `fmc_ui`; product-main remains the serialized owner and
measurement runtime stops retaining passive input. The legacy `fm_user` and
`fmx` shapes are not suitable module boundaries to port directly.

## Review Closure And Future Use

This analysis completed the Phase 8-0A route foundation. It does not itself
accept product behavior or dispose legacy entries. The subsequent Phase 8A
contract and coverage review now record the human decisions.

Future use of this file should remain evidentiary: compare provenance, explain
why current behavior replaced or retained a legacy choice, and avoid expanding
the accepted contract from historical implementation details.
