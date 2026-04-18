# WORKING_CONTEXT.md

## Purpose

This file is the active working contract for the LCD redesign path.

Use it first for tasks under:
- `bsp/devices/lcd/`
- `bsp/devices/lcd/pcf8553/`

This document is temporary and operational.
It is not the final API contract and it is not the repository-wide style guide.

---

## Reading Order

For LCD redesign work, use this order:

1. `WORKING_CONTEXT.md`
2. `AGENT_ENTRY.md`
3. `workflows/README.md`
4. `STYLE.md` only when naming or file/module creation decisions are involved
5. `workflows/comment_pass.md` only for a dedicated comment pass

Current priority:
- keep the LCD redesign direction clear
- keep module boundaries clean
- postpone broad style/comment cleanup unless a pass is explicitly about that

---

## Current Intent

The LCD stack is being rebuilt as a new path.
It is not a compatibility-first refactor of the existing LCD path.

Design priorities:
1. semantic clarity
2. explicit ownership
3. reviewable implementation
4. low-power-aware write behavior
5. later optimization after behavior is clear

Memory size is not the main constraint in this redesign.
Explicit tables are preferred over opaque arithmetic when they improve clarity.

---

## Architecture Contract

The working target remains a four-module LCD stack:

1. `fm_lcd.*`
- public LCD behavior and state
- dirty tracking
- visible-state composition
- flush policy
- logical blink behavior

2. `fm_lcd_layout.*`
- semantic model of the glass
- stable names for rows, alpha digits, and indicators
- datasheet traceability where useful

3. `fm_lcd_map.*`
- pure glass-to-RAM mapping
- explicit mapping tables
- character encoding/font tables
- caller-provided RAM buffers only
- no hardware I/O

4. `pcf8553/fm_pcf8553.*`
- controller startup/reset/resume behavior
- RAM write behavior
- partial write support
- controller-specific details only
- no glass semantics

Additional shared header allowed by current design:
- `fm_lcd_types.h`
  - only for small public types shared by the new LCD modules
  - must stay narrow
  - must not become a dump of unrelated definitions

---

## Contract Artifacts Already Created

The following new-path headers already exist:
- `bsp/devices/lcd/fm_lcd_layout.h`
- `bsp/devices/lcd/fm_lcd_types.h`
- `bsp/devices/lcd/fm_lcd.h`
- `bsp/devices/lcd/fm_lcd_map.h`
- `bsp/devices/lcd/pcf8553/fm_pcf8553.h`

What this means now:
- the contract surface is already far enough along
- the main work is no longer “invent the headers”
- the main work is now implementation and validation in the correct order

---

## Legacy Separation Rule

The legacy LCD path must not condition the new LCD path.

This rule is active and non-negotiable while the redesign is in progress.

Hard separation rules:
- do not make the new path call legacy symbols through wrappers
- do not share mutable globals between legacy and new paths
- do not make new modules include legacy headers except in explicitly legacy
  translation units
- if a useful legacy sequence is reused, copy it consciously into the new path
  with new ownership instead of delegating behavior back to the legacy path

The point of the legacy path is:
- backup
- reference
- known-working behavior source

The point of the legacy path is not:
- architectural dependency
- helper layer for the new path
- hidden implementation substrate for the new path

---

## Current Legacy/New Split

New-path contracts:
- `bsp/devices/lcd/fm_lcd.h`
- `bsp/devices/lcd/fm_lcd_layout.h`
- `bsp/devices/lcd/fm_lcd_types.h`
- `bsp/devices/lcd/fm_lcd_map.h`
- `bsp/devices/lcd/pcf8553/fm_pcf8553.h`

Frozen legacy path:
- `bsp/devices/lcd/fm_lcd.c`
- `bsp/devices/lcd/fm_lcd_legacy.h`
- `bsp/devices/lcd/fm_lcd_ll.c`
- `bsp/devices/lcd/fm_lcd_ll.h`
- `bsp/devices/lcd/pcf8553/fm_pcf8553_legacy.c`
- `bsp/devices/lcd/pcf8553/fm_pcf8553_legacy.h`

Current backend split:
- `fm_pcf8553_legacy.c/.h` preserve the historical backend path
- `fm_pcf8553.c/.h` are reserved for the redesigned backend path

Current bring-up apps are not architecture constraints for the redesign path.

---

## Semantic Rules

The LCD should be modeled as:
- one top numeric row with 8 visible positions
- one bottom numeric row with 7 visible positions
- one set of standalone indicators and product-specific legends
- one pair of 14-segment alphanumeric characters

Public naming should prefer visible meaning over raw datasheet labels.
Datasheet labels may still appear in comments or traceability notes.

Decimal points belong semantically to numeric rows, not to the standalone
indicator set.

Blink is expected to be logical display behavior owned by `fm_lcd.*`, not the
primary semantic model of the PCF8553 hardware blink.

---

## Current Boundary Notes

`fm_lcd_map.h` no longer depends on `fm_lcd.h` for alignment types.
That boundary correction is already applied through `fm_lcd_types.h`.

Current contract quality assessment:
- no major legacy contamination is expected in the new headers
- the new headers no longer depend on legacy headers
- the backend new path and backend legacy path now have separate public symbols

This does not mean the implementation is finished.
It means the structural direction is now acceptable to continue from.

---

## Backend Notes

The new backend must own:
- controller initialization
- reset sequencing
- resume behavior
- display RAM range writes
- controller-specific startup quirks from the datasheet

The new backend must not own:
- glass semantics
- higher-level desired LCD state
- public blink semantics
- legacy framebuffer globals

Known controller facts still relevant:
- the PCF8553 supports sequential RAM writes with auto-incremented addressing
- hardware blink exists but should not define the LCD public API
- after reset or POR, controller configuration must be restored before normal
  operation

---

## Current Phase

The redesign is no longer in pure contract-definition mode.

The active phase is:
1. harden `pcf8553/fm_pcf8553.c`
2. implement `fm_lcd_map.c`
3. implement `fm_lcd.c`

This order is intentional.
Do not skip upward while the lower boundary is still unstable.

---

## Immediate Next Work

Current recommended next step:
- finish hardening `bsp/devices/lcd/pcf8553/fm_pcf8553.c`

That means:
- validate startup behavior
- validate reset/resume expectations
- validate RAM range write behavior
- keep the implementation independent from the legacy backend path

Only after that:
- implement `bsp/devices/lcd/fm_lcd_map.c`

Only after that:
- implement `bsp/devices/lcd/fm_lcd.c`

---

## Out Of Scope For Now

Do not let the redesign get blocked by:
- preserving compatibility with the old `fm_lcd.*` API
- preserving compatibility with the old `fm_lcd_ll.*` API
- reusing current bring-up apps as architectural anchors
- broad cleanup outside the LCD redesign path
- final comment polish everywhere
- broad naming cleanup unrelated to the current pass

---

## Decision Rule

When a choice is unclear, prefer:
1. semantic clarity
2. explicit ownership
3. clean separation from legacy
4. reviewable implementation
5. small forward progress

If a choice would make the new path depend structurally on the legacy path,
stop and choose a different approach.
