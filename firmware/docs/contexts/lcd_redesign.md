# LCD Redesign Working Document

## Purpose

This is the active working document for the LCD redesign.

It is intentionally dynamic.
It should preserve:
- current state
- already completed work
- remaining work
- the contract pieces still needed to continue safely

For immediate execution, `WORKING_CONTEXT.md` is authoritative.
This file is the extended technical basis, backlog, and redesign rationale.

---

## Current State

Current stage:
- validation

Current situation:
- the new LCD stack already exists
- the canonical bring-up already passed on hardware for the validated segment scope
- alpha support now exists in the public contract and in the new stack
- the validated segment path is now treated as closed unless future visual
  evidence exposes a real defect
- logical blink contracts now exist in the public LCD headers
- a timer-neutral companion blink-policy module now exists outside `fm_lcd.*`
- the next useful progress is implementing visible blink masking in `fm_lcd.c`
  before any bring-up extension

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
- `bsp/devices/lcd/fm_lcd_blink.h`
- `bsp/devices/lcd/fm_lcd_map.h`
- `bsp/devices/lcd/pcf8553/fm_pcf8553.h`

### Implementation already completed
- `bsp/devices/lcd/pcf8553/fm_pcf8553.c`
  - first backend hardening pass completed
- `bsp/devices/lcd/fm_lcd_map.c`
  - first pure mapping pass completed for numeric rows, indicators, and alpha
  - lowercase alpha inputs are now explicitly accepted
  - `'#'` is currently encoded as a validation glyph that turns on all 14 alpha
    segments
- `bsp/devices/lcd/fm_lcd.c`
  - first public stateful LCD V1 completed over `fm_lcd_map.*` and `fm_pcf8553.*`
  - alpha support now exposed through the public contract
  - blink selection and phase state are now stored in the LCD core
- `bsp/devices/lcd/fm_lcd_blink.c`
  - timer-neutral blink state machine now exists for on/off phase sequencing
- `apps/lcd_bringup/`
  - unified human-validation bring-up sequence completed
  - alpha scenes now include:
    - left alpha full-on
    - right alpha full-on
    - sequential sweep of the currently encoded alpha character set

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
1. implement visible blink masking in `fm_lcd.c`
2. compose the visible LCD image from:
   - stable desired content
   - configured blink ranges
   - current blink phase
3. keep `fm_lcd_map.*` unchanged and keep timer ownership outside `fm_lcd.*`

Likely next work after that:
1. extend `apps/lcd_bringup/` only after the visible blink behavior is stable
2. validate top-row, bottom-row, and alpha blink scenes on hardware
3. build the higher-level editing module on top of the LCD blink primitives

Current validated coverage:
- top numeric row
- bottom numeric row
- decimal points
- standalone indicators
- current supported alpha rendering behavior

Explicitly not validated yet:
- logical blink behavior
- richer resume or recovery policy

Current alpha string policy:
- application text enters alpha through normal C strings
- printable ASCII is the intended input domain
- lowercase is accepted by the mapping table
- unsupported characters are rendered as blank
- non-printable control-byte encodings are not part of the intended public
  string contract
- `'#'` is a validation glyph for bring-up use, not yet a recommended
  application symbol

Current blink contract direction:
- keep blink selection ownership inside `fm_lcd.*`
- keep timer ownership outside `fm_lcd.*`
- keep the higher-level variable editing flow outside the LCD stack
- keep V1 blink limited to text cells:
  - top row
  - bottom row
  - alpha pair

---

## Out Of Scope For Now

Do not let this refactor get blocked by:
- preserving compatibility with the old `fm_lcd.*` API
- preserving compatibility with the old `fm_lcd_ll.*` API
- preserving backup material inside the active build path
- broad cleanup outside the LCD redesign path
- final comment polish everywhere
- broad naming cleanup unrelated to the active validation-driven pass
- implementing the higher-level variable editing module itself inside `fm_lcd.*`
