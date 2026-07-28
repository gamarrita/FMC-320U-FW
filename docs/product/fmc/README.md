# FMC Product Documentation

## Purpose

This directory is the owner of current, reviewed FMC-320U product
documentation. It gives the whole product broad, superficial coverage and
provides stable destinations for requirements, observable behavior, and user
interface knowledge.

Documentation depth follows the next programming slice. Broad coverage does
not mean that every statement is accepted or detailed.

## Decision And Evidence Model

Substantive statements use one of these decision states:

- `Accepted`: a current product decision approved by the human or established
  by an unambiguous current contract.
- `Candidate`: a proposal or working input that still needs review.
- `Unresolved`: available information is insufficient or contradictory.
- `Deferred`: the domain is real, but detailed decisions are intentionally
  postponed.

`Evidence` is not a decision state. It identifies provenance or support for a
statement. A detailed source, a legacy implementation, or a `confirmed` entry
in a working inventory does not make a statement `Accepted`.

## Document Ownership

| Document | Owns | Does not own |
|---|---|---|
| [Requirements](requirements.md) | Product obligations | Detailed flows, UI design, firmware architecture |
| [Behavior](behavior.md) | Cross-cutting observable behavior | Normative obligations already owned by requirements, screen-by-screen navigation |
| [User interface](user_interface.md) | Visible and operable experience | LCD physical mapping, driver design, complete state machine |

The documents may reference one another but should not duplicate normative
content.

Related repository roles:

- `docs/specs/` contains structured technical specifications, normalized
  inventories, and engineering inputs.
- `legacy/derived/fmc/use_cases.extraction-v1.yaml` is frozen derived legacy
  evidence, not a current product contract.
- `docs/workflow/fmc_legacy_coverage.md` tracks human review coverage and
  disposition without defining product behavior.
- `docs/specs/lcd/lcd_true_source.yaml` is the technical authority for LCD
  glass elements and physical mapping.
- public headers own implemented module contracts.
- `legacy/` preserves historical evidence, not current authority.
- `WORKING_CONTEXT.md`, when present, owns the active workstream and its single
  active slice; the roadmap owns durable sequencing.

Human learning references:

- [fmc_service tutorial](../../turotial/fmc_service.md);
- [fmc_runtime tutorial](../../turotial/fmc_runtime.md);
- [Phase 7 acquisition tutorial](../../turotial/phase7_acquisition.md).

These protected tutorials explain architecture for human readers. They do not
replace product documents, technical specifications, or public headers.

## Product Overview

**Accepted:** FMC-320U is the product line documented here, and its current
implemented product core models flow measurement through pulse-backed totals,
unit-aware volume and rate calculations, and semantic operator input.

Evidence:
- root `README.md`;
- `src/product/fmc/` public headers.

**Candidate:** The complete field product is a battery-powered flow computer
with configuration, persistence, RTC, alarms, logging, printing,
communications, and optional sensing functions.

Evidence:
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- `legacy/analysis/module_inventory.md`;
- `legacy/analysis/migration_ledger.md`.

## Functional Coverage Map

This map reports documentation coverage, not firmware implementation status.

| Domain | Product aspect | Owner | Depth | Decision state | Evidence | Deepen when |
|---|---|---|---|---|---|---|
| Power, startup, and recovery | Power-on, initialization, restore, low-power concerns | [Behavior](behavior.md) | Surface | Candidate | Roadmap; frozen legacy extraction; legacy `fm_init`/`fmx_lp` inventory | Startup or power slice |
| Pulse and signal acquisition | Conserving counter-backed pulses through bounded deltas | [Requirements](requirements.md) and `docs/specs/fmc/acquisition.md` | Focused Phase 7A | Accepted | Reviewed Phase 7A decisions; runtime/service contracts; legacy acquisition | Phase 7B1 bring-up |
| Flow-rate calculation | Deriving active-unit rate from pulse and time windows | [Requirements](requirements.md) | Focused core | Accepted | `fmc_rate.h`; `fmc_units.h` | Acquisition or display slice |
| Totalization | ACM/TTL accumulation, visible totals, and resets | [Requirements](requirements.md) | Focused core | Accepted | `fmc_model.h`; `fmc_volume.h`; `fmc_service.h` | Operational reset flow |
| Presentation | Startup, live TTL/RATE and ACM/RATE, placeholders, and user-menu POINT | [User interface](user_interface.md) and [Behavior](behavior.md) | Focused Phase 8 contract | Accepted | Reviewed Phase 6A and Phase 8 decisions; presentation and LCD contracts | Functional implementation slices |
| Operator input | Semantic mechanical and external-button actions after hardware translation | [User interface](user_interface.md) and [Behavior](behavior.md) | Focused Phase 8 contract | Accepted | `fmc_input.h`; reviewed Phase 8 decisions | Functional implementation slices |
| Navigation | Context-dependent visible consequences in the five-screen user menu | [User interface](user_interface.md) | Focused Phase 8 contract | Accepted | Reviewed Phase 8 decisions; legacy coverage register | Configuration navigation in Phase 10 |
| Configuration and calibration | Editable measurement and product settings | [Requirements](requirements.md) | Surface | Candidate | Model contract; frozen legacy extraction; legacy inventory | Phase 10 |
| Persistence and defaults | Retention, restore, validation, and factory data | [Requirements](requirements.md) | Surface | Candidate | Roadmap; frozen legacy extraction; legacy inventory | Phases 11 and 12 |
| RTC and time | Current time, editing, validity, and timestamp use | [Behavior](behavior.md) | Surface | Candidate | Roadmap; frozen legacy extraction; legacy `fm_rtc` inventory | Phase 9 |
| Alarms, status, and recovery | Operator-visible abnormal conditions and recovery | [Behavior](behavior.md) | Surface | Candidate | Frozen legacy extraction; roadmap | Selected operational flow |
| Information logging | Event selection, timestamping, retention, and retrieval | [Requirements](requirements.md) | Surface | Deferred | Frozen legacy extraction; legacy log inventory | Logging slice |
| Communications and commands | External protocol and command behavior | [Requirements](requirements.md) | Surface | Deferred | Legacy command/USART inventory | Transport decision |
| Printing | Ticket content and print workflow | [User interface](user_interface.md) | Surface | Deferred | Frozen legacy extraction; legacy `fm_ppt` inventory | Phase 15 |
| Logged-data download | Operator purpose and future transport workflow | [User interface](user_interface.md) | Placeholder only | Deferred | Frozen legacy extraction; legacy `fm_mxc` inventory | Phase 14 |
| Optional sensing | PT100 and other optional product extensions | [Requirements](requirements.md) | Surface | Deferred | Frozen legacy extraction; roadmap | Phase 13 product selection |
| Diagnostics, service, and validation | Product-facing diagnostic or service behavior | [Requirements](requirements.md) | Surface | Unresolved | Current bring-ups; command/debug legacy inventory | Service requirement |

## Technical Authority Boundary

**Accepted:** Product documentation may name LCD fields, symbols, and
capabilities needed by the operator experience. It must reference
`docs/specs/lcd/lcd_true_source.yaml` for physical glass facts and must not copy
COM/SEG tables, controller registers, RAM bits, or routing maps.

Evidence:
- `AGENTS.md`;
- `docs/specs/lcd/lcd_true_source.yaml`.

## Current Focused Depth

**Accepted:** Phase 6A completed the bounded initial presentation slice. The
Phase 8 product contract defines the complete minimum user menu required before
its functional implementation slices.

Evidence:
- `docs/roadmaps/fmc_refactoring.md`.

The accepted Phase 8 contract adds live ACM/RATE, inert PRINT, LOG_DOWNLOAD,
and DATE_TIME positions, complete bounded navigation, ACM reset, user-menu
POINT, external-button semantics, and backlight behavior. Exact content,
timing, transitions, formatting, and deferred boundaries are owned by
[User interface](user_interface.md) and [Behavior](behavior.md). Firmware
release and tag policy is owned by
[Firmware Versioning](../../project/firmware_versioning.md).
