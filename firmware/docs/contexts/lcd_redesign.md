# LCD Redesign Working Document

## Purpose

This is the active working document for the LCD redesign.

It is intentionally dynamic.
It should preserve:
- current state
- already completed work
- remaining work
- the contract pieces still needed to continue safely

---

## Current State

Current stage:
- validation

Current situation:
- the new LCD stack already exists
- the canonical bring-up already exists
- the next useful progress depends on real LCD and UART evidence

Current validation target:
- `apps/lcd_bringup/`

Expected UART milestones:
- `LCD_BRINGUP:START`
- `LCD_BRINGUP:LCD_INIT_OK`
- `LCD_BRINGUP:ROW_TOP_OK`
- `LCD_BRINGUP:INDICATOR_OK`
- `LCD_BRINGUP:FLUSH_OK`
- `LCD_BRINGUP:IDLE`

Current expected interaction:
- the agent keeps the stack and bring-up coherent
- the human runs the hardware and reports what the LCD and UART actually do

---

## Work Already Completed

### Contracts already created
- `bsp/devices/lcd/fm_lcd_layout.h`
- `bsp/devices/lcd/fm_lcd_types.h`
- `bsp/devices/lcd/fm_lcd.h`
- `bsp/devices/lcd/fm_lcd_map.h`
- `bsp/devices/lcd/pcf8553/fm_pcf8553.h`

### Implementation already completed
- `bsp/devices/lcd/pcf8553/fm_pcf8553.c`
  - first backend hardening pass completed
- `bsp/devices/lcd/fm_lcd_map.c`
  - first pure mapping pass completed for numeric rows and indicators
- `bsp/devices/lcd/fm_lcd.c`
  - first public stateful LCD V1 completed over `fm_lcd_map.*` and `fm_pcf8553.*`
- `apps/lcd_bringup/`
  - first unified bring-up for the new stack completed

### Structural decisions already taken
- the redesign is not compatibility-first
- legacy must not structurally condition the new path
- explicit tables are preferred over opaque arithmetic when they improve clarity
- the canonical active bring-up is the new one, not the legacy ones

---

## Contract Still In Force

Keep this contract active while the redesign continues.

### Architecture target

The current architecture target remains:
- `fm_lcd.*`
- `fm_lcd_layout.*`
- `fm_lcd_map.*`
- `pcf8553/fm_pcf8553.*`

`fm_lcd_types.h` is allowed only as a narrow shared helper header.

### Legacy separation

The legacy LCD path must not condition the new LCD path.

Keep these rules active:
- do not make the new path call legacy symbols through wrappers
- do not share mutable globals between legacy and new paths
- do not include legacy headers from the new path
- if a useful legacy sequence is reused, copy it consciously with new ownership

Frozen backup path:
- `legacy_backup/apps/lcd_ll_bringup`
- `legacy_backup/apps/pcf8553_bringup`
- `legacy_backup/bsp/devices/lcd/fm_lcd_legacy.c`
- `legacy_backup/bsp/devices/lcd/fm_lcd_legacy.h`
- `legacy_backup/bsp/devices/lcd/fm_lcd_ll.c`
- `legacy_backup/bsp/devices/lcd/fm_lcd_ll.h`
- `legacy_backup/bsp/devices/lcd/pcf8553/fm_pcf8553_legacy.c`
- `legacy_backup/bsp/devices/lcd/pcf8553/fm_pcf8553_legacy.h`

### Semantic rules

The LCD is modeled as:
- one top numeric row with 8 visible positions
- one bottom numeric row with 7 visible positions
- one set of standalone indicators and product-specific legends
- one pair of 14-segment alphanumeric characters

Public naming should prefer visible meaning over raw datasheet labels.
Decimal points belong to numeric rows, not to the standalone indicator set.
Blink is expected to be logical display behavior owned by `fm_lcd.*`.

### Backend facts still relevant

Board assumptions explicit in this redesign:
- `PORE` is tied to `VDD`
- `IFS` is tied to `VDD`

Relevant controller facts:
- the PCF8553 supports sequential RAM writes with auto-incremented addressing
- hardware blink exists but should not define the LCD public API
- after reset or POR, controller configuration must be restored before normal operation

---

## Remaining Work

Immediate remaining work:
1. validate `apps/lcd_bringup/` end to end on hardware
2. compare observed UART and LCD behavior against the expected milestones and scene
3. apply the smallest correction pass supported by that evidence

Likely next work after that:
1. mapping correction if visible segments or indicators are wrong
2. alpha support if validation shows the base path is stable enough
3. blink or resume-policy work once the base behavior is trusted

---

## Out Of Scope For Now

Do not let this refactor get blocked by:
- preserving compatibility with the old `fm_lcd.*` API
- preserving compatibility with the old `fm_lcd_ll.*` API
- preserving backup material inside the active build path
- broad cleanup outside the LCD redesign path
- final comment polish everywhere
- broad naming cleanup unrelated to the active validation-driven pass
