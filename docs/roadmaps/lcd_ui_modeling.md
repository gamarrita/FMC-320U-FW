# LCD UI Modeling Roadmap

## Purpose

Establish a human- and agent-reviewable source for segmented-LCD screens and
semantic button interactions before adding more product screens. The tooling
must complement, not replace, product requirements or the physical LCD
authority.

`docs/specs/lcd/lcd_true_source.yaml` remains the authority for glass fields,
glyphs, indicators, COM/SEG routing, and validated physical capabilities. An
instrument UI project owns the modeled screen composition, example variables,
and semantic interactions. Its declarative details become authoritative only
through the promotion rule in `docs/product/fmc/user_interface.md`. Generated
previews and fixtures are derived artifacts.

## UI-MODEL-1: LCD Simulator MVP

Status:
- completed and human-accepted on 2026-09-01 as an executable prototyping and
  contract-initiation tool;
- does not by itself own accepted product behavior or authorize generated
  firmware code;
- individual modeled screens require explicit review and promotion before
  their declarative details become authoritative.

Objective:
- provide a local/web simulator of the FMC segmented LCD and semantic buttons;
- load a versioned, reviewable YAML UI project;
- reproduce the accepted Phase 8 screens and navigation;
- export the project and a human-readable visual catalog;
- establish a deterministic foundation for later fixtures and code generation.

Dependencies:
- accepted Phase 8 screen and navigation contracts;
- `docs/specs/lcd/lcd_true_source.yaml`;
- current semantic `fmc_ui_frame_t`, `fmc_input_event_t`, and LCD layout
  contracts as implementation evidence.

MVP route:

`M1 schema -> M2 LCD renderer -> M3 semantic buttons/navigation -> M4 YAML import/export -> M5 Phase 8 equivalence`

Completion outcome (2026-09-01):
- M1 through M4 are implemented under `tools/lcd-ui-studio/`;
- M5 has automated frame/navigation equivalence checks, a visual catalog, and
  human visual acceptance;
- the physical renderer is calibrated from the FMC-160LDC vector artwork and
  the confirmed V1R1R2 front-panel photograph, while accepted product behavior
  remains owned by product documentation;
- focused tests, lint, the production build, and the production-dependency
  security audit pass;
- the review deployment is owner-only;
- the current schema models presentation and deterministic transitions but is
  not yet a complete contract for indicator capabilities or operator editing;
  those semantics must be added from concrete product cases, not inferred from
  simulator controls.

Included scope:
- one fixed segmented-LCD device profile matching the current glass;
- screen IDs, literals, example variables, alignment, indicators, and variants;
- SHORT/LONG DOWN, UP, ENTER, ESC, EXT_1, and EXT_2 interactions;
- deterministic transition simulation and current-state inspection;
- YAML import/export with schema version and stable IDs;
- visual catalog or snapshots derived from the same project;
- accepted Phase 8 startup and user-menu examples.

Deferred:
- parameter editing, authorization, drafts, apply/cancel, or persistence;
- RTC, acquisition, ThreadX, HAL, target access, or hardware configuration;
- automatic generation or replacement of firmware source;
- arbitrary display-device authoring or a general-purpose GUI builder;
- hosted collaborative storage, authentication, and multi-user editing.

Architecture gates:
- the UI project must not contain COM/SEG or controller-register details;
- variables use stable semantic IDs, never C expressions, addresses, or raw
  pointers;
- the simulator and future generators must be deterministic from the same
  versioned project;
- generated outputs remain separate from authored firmware and documentation;
- complex product rules remain authored behind typed bindings or requests.

Satisfied exit criteria:
- a human can inspect and operate the current screen path visually;
- YAML round-trip is stable and schema errors are reported clearly;
- the modeled Phase 8 screens and semantic transitions match accepted
  documentation and regression expectations;
- changing a screen produces a reviewable project diff and regenerated visual
  result;
- no firmware, generated CubeMX source, or hardware behavior changes;
- the human accepted the MVP before Phase 9 resumed.

## Later Candidates

- `UI-MODEL-2`: generate deterministic frame and transition fixtures;
- `UI-MODEL-3`: generate bounded firmware descriptors after equivalence is
  proven;
- `UI-MODEL-4`: add declarative editing mechanics when Phase 10 provides real
  editor cases.

These candidates are not authorized by UI-MODEL-1. Phase work may introduce
only the smallest candidate capability required by an accepted concrete screen
case.
