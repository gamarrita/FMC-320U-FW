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
- the canonical bring-up already passed on hardware for the validated scope
- alpha support now exists in the public contract and in the new stack
- the next useful progress is to expose alpha in the bring-up and validate it
  on hardware without destabilizing the validated numeric base

Validated hardware target:
- `apps/lcd_bringup/`

Validated UART milestones:
- `LCD_BRINGUP:START`
- `LCD_BRINGUP:LCD_INIT_OK`
- `LCD_BRINGUP:LOOP_RESTART`
- `LCD_BRINGUP:SCENE_CLEAR`
- `LCD_BRINGUP:SCENE_ALL_DIGITS_ON TOP=88888888 BOT=8888888`
- `LCD_BRINGUP:SCENE_ALL_DECIMALS_ON`
- `LCD_BRINGUP:SCENE_DIGIT_PATTERN TOP=12345678 BOT=1234567`
- `LCD_BRINGUP:SCENE_BASIC_ICONS POINT+BATT`
- `LCD_BRINGUP:SCENE_UPPER_LEGENDS`
- `LCD_BRINGUP:SCENE_LOWER_RIGHT_LEGENDS`
- `LCD_BRINGUP:SCENE_ALL_INDICATORS`
- `LCD_BRINGUP:SCENE_NOMINAL_USE TOP=12.34 BOT=56.7`
- `LCD_BRINGUP:IDLE`

Observed validation result:
- the expected UART sequence matched the visible LCD behavior
- no correction pass is currently needed for:
  - top numeric row
  - bottom numeric row
  - decimal points
  - standalone indicators

Current expected interaction:
- the agent keeps the next increment small and coherent
- the human returns to hardware validation when alpha support is exposed by the
  bring-up

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
  - first pure mapping pass completed for numeric rows, indicators, and alpha
- `bsp/devices/lcd/fm_lcd.c`
  - first public stateful LCD V1 completed over `fm_lcd_map.*` and `fm_pcf8553.*`
  - alpha support now exposed through the public contract
- `apps/lcd_bringup/`
  - unified human-validation bring-up sequence completed

### Validation already completed
- `apps/lcd_bringup/`
  - hardware validation completed for:
    - top numeric row
    - bottom numeric row
    - decimal points
    - standalone indicators

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
1. extend `apps/lcd_bringup/` with alpha scenes
2. validate alpha behavior on hardware using LCD + UART evidence
3. apply the smallest correction pass supported by that evidence

Likely next work after that:
1. mapping correction only if alpha validation exposes issues in the currently
   unvalidated area
2. blink or resume-policy work once alpha behavior is trusted

Current validated coverage:
- top numeric row
- bottom numeric row
- decimal points
- standalone indicators

Explicitly not validated yet:
- 14-segment alpha pair
- logical blink behavior
- richer resume or recovery policy

---

## Out Of Scope For Now

Do not let this refactor get blocked by:
- preserving compatibility with the old `fm_lcd.*` API
- preserving compatibility with the old `fm_lcd_ll.*` API
- preserving backup material inside the active build path
- broad cleanup outside the LCD redesign path
- final comment polish everywhere
- broad naming cleanup unrelated to the active validation-driven pass
