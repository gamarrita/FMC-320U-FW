# WORKING_CONTEXT.md

## Current Work

Stage:
- analysis

Active pass:
- refactor

Operational context:

Area:
- `legacy_backup/libs/fm_fmc.c`
- `legacy_backup/libs/fm_fmc.h`
- `legacy_backup/libs/fm_user.c`
- `legacy_backup/libs/fm_setup.c`
- `libs/`
- `libs/fm_fmc_model.h`
- `bsp/devices/lcd/`

Core structure:
- `legacy_backup/libs/fm_fmc.*` is legacy behavior reference, not a direct
  port target
- the closed LCD stack under `bsp/devices/lcd/` is the display foundation to
  preserve
- this refactor now targets:
  - an FMC model layer
  - then an FMC presentation-semantics layer above it
  - then a later LCD adapter over the validated LCD stack
- the first slice excludes pulse math, capture logic, and legacy UI flow

Current focus:
- extract the FMC semantic elements that the instrument actually exposes:
  ACM, TTL, RATE, pulse counts, K/calibration, units, and time base
- run an item-by-item clarification loop over the legacy inventory before
  accepting more contract in `libs/fm_fmc_model.h`
- separate:
  - FMC model semantics
  - FMC presentation semantics
  - LCD adapter concerns
- incorporate the latest domain decisions:
  - TTL is resettable, but by a more privileged mechanism than ACM
  - TTL remains non-user-resettable in normal operation
  - ACM and TTL should stay simple and share the same volume-unit ownership
  - RATE should share the same volume unit and only vary by time base
  - pulse counters remain relevant as the backing state behind totals
- clarify semantic assumptions before accepting any `.h` as a contract:
  - ACM, TTL, and RATE are runtime-derived state, not editable configuration
  - configuration and runtime ownership must be reviewed explicitly
  - interpretation problems found in the header should be treated as blockers,
    not cosmetic issues
- use the working loop:
  - inspect one legacy item
  - ask until intent and ownership are clear
  - record the clarified meaning in the working artifacts
  - only then move to the next item
- keep the first slice oriented to a flow-computer instrument while improving
  semantics and coherence instead of copying legacy structure or locking the
  contract to `ufp3_t`

Constraints:
- do not port `legacy_backup/` modules directly into `libs/`
- do not include pulse/capture math in the first slice
- do not pull in `FM_LCD_LL_*`, setup screens, user menu flow, Bluetooth, RTC,
  ticketing, or backup persistence
- use `legacy_backup/libs/fm_fmc.*`, `fm_user.c`, and `fm_setup.c` only as
  references for domain discovery and contrast, not as semantics to preserve
- use scaled/fixed-point only if it remains an implementation detail, not as
  the semantic boundary of the new module
- do not keep `flow_active` or `pulse_activity` as core model state unless a
  later requirement proves they are first-class FMC elements
- keep the model shape copyable so the product can later hold:
  - active environment
  - edit buffer
  - factory/default copies

## Next Step

- continue the semantic clarification loop using:
  - `docs/specs/fmc/fm_fmc_legacy_field_inventory.md`
  - `docs/specs/fmc/use_cases.yaml`
  - `docs/specs/math/fm_numeric_library_candidate.md`
- keep `libs/fm_fmc_model.h` in candidate status until this loop closes the
  current group of doubts
- for each item under review:
  - clarify intended product meaning
  - separate canonical/config/runtime/presentation roles
  - record whether it survives and in which layer
- only after this loop stabilizes:
  - refine the minimum contract for `libs/fm_fmc_model.h`
- define the seam between:
  - FMC model semantics
  - FMC presentation semantics
  - LCD adapter usage of the validated LCD stack

## References

- `AGENTS.md`
- `docs/contexts/fmc_presentation_refactor.md`
- `docs/specs/fmc/use_cases.yaml`
- `docs/specs/fmc/fm_fmc_legacy_field_inventory.md`
- `docs/specs/math/fm_numeric_library_candidate.md`
- `STYLE.md`

## Invocation Rule

Every time this file is used:
- confirm stage, active pass, current focus, constraints, and next step
- for immediate execution, `Constraints` and `Next Step` are authoritative
- use `legacy_backup/` only as behavior reference, not as direct architecture
- if the real work drifted, update this file and its referenced
  `docs/contexts/` file before continuing

## Maintenance Rule

Keep this file updated when any of these change:
- stage
- active pass
- operational focus
- constraints
- next step
- referenced active context file

Keep the referenced file under `docs/contexts/` updated when any of these
change:
- extended technical rationale
- detailed scope
- validated or pending state
- remaining work
- active technical decisions

Update both together when:
- the short execution state and the extended context would otherwise diverge
- the active line of work changes materially

## Strong Rule

This is execution state, not documentation.
No history.
No long explanation.
Detailed rationale stays in `docs/contexts/fmc_presentation_refactor.md`.
Keep `WORKING_CONTEXT.md` and its referenced `docs/contexts/` file aligned.
Do not let this file absorb extended design detail that belongs in the
referenced context file.
