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
The operative public contract still lives in the active LCD headers.
If this document drifts from `bsp/devices/lcd/*.h`, the headers win.
This file should preserve the initial contract intent, composition model, and
implementation rationale even when the public surface evolves incrementally.

---

## Current State

Current stage:
- implementation

Current situation:
- the new LCD stack already exists
- the canonical bring-up already passed on hardware for the validated segment scope
- alpha support now exists in the public contract and in the new stack
- the validated segment path is now treated as closed unless future visual
  evidence exposes a real defect
- logical blink contracts now exist in the public LCD headers
- a timer-neutral companion blink-policy module now exists outside `fm_lcd.*`
- minimal pure visible-cell helper contracts now exist in `fm_lcd_map.h`
- the minimal pure visible-cell helpers are now implemented in `fm_lcd_map.c`
- visible blink masking is now implemented in `fm_lcd.c`
- `apps/lcd_bringup/` should remain the canonical static validation app
- the next useful progress is creating `apps/lcd_blink_bringup/` for blink
  validation with blocking timing outside `fm_lcd.*`

Canonical human-validation target:
- `apps/lcd_bringup/fm_lcd_bringup.c`

Dedicated blink-validation target:
- `apps/lcd_blink_bringup/`

Validation evidence rule:
- the canonical UART scene labels and expected visible behavior should live in
  the bring-up app, not be duplicated here
- this document should only track which behavior is already considered
  validated for the redesign path

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
  - pure visible-cell clearing helpers now exist for:
    - one numeric row cell
    - one alpha digit
- `bsp/devices/lcd/fm_lcd.c`
  - first public stateful LCD V1 completed over `fm_lcd_map.*` and `fm_pcf8553.*`
  - alpha support now exposed through the public contract
  - blink selection and phase state are now stored in the LCD core
  - visible LCD composition now uses:
    - stable desired RAM
    - composed visible RAM
    - last flushed visible RAM
  - blink `OFF` phase now hides selected visible text cells through the
    mapping helpers before `FM_LCD_Flush()`
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
1. create `apps/lcd_blink_bringup/`
2. validate top-row, bottom-row, and alpha blink behavior there with a simple
   blocking delay that toggles logical blink phase
3. keep timer ownership outside `fm_lcd.*`

Likely next work after that:
1. validate that the dedicated blink bring-up app matches visible hardware
   behavior
2. decide whether a richer scheduler-driven blink harness is needed later
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
- allow minimal pure visible-cell helpers in `fm_lcd_map.*` when they improve
  semantic clarity for visible-cell operations
- keep V1 blink limited to text cells:
  - top row
  - bottom row
  - alpha pair
- keep validation of static LCD content separate from validation of temporal
  blink behavior

Current visible-cell helper direction:
- keep the helpers generic and semantic, not blink-specific
- allow clearing one visible numeric cell without touching its decimal point
- allow clearing one visible alpha digit without touching the other one

### Blink composition model

The intended visible blink behavior is based on three different notions of LCD
state:
- stable desired content
- current logical blink state
- last flushed visible image

The model should be understood this way:
- `desired_ram` is the stable LCD content requested by the application
- blink selection, enabled state, and current phase are separate logical state
  owned by `fm_lcd.*`
- the visible LCD image should be composed from `desired_ram` plus the current
  blink state before hardware I/O happens
- `flushed_ram` should represent the last visible image actually written to the
  controller, not just the stable desired content

Current implementation fact before visible blink masking:
- `FM_LCD_Flush()` still writes `desired_ram` directly to hardware
- dirty tracking still compares `desired_ram` against `flushed_ram`
- this is sufficient for the already validated non-blink path, but it is not
  yet the final intended model for logical blink

Intended behavior once visible blink masking is implemented:
- text-writing APIs update `desired_ram`, not hardware
- blink-selection APIs update logical blink state, not base content
- phase changes should affect only the composed visible image
- `FM_LCD_Flush()` should write the composed visible image, not raw
  `desired_ram`, when blink affects visibility
- dirty tracking should be driven by the visible composed image versus the last
  flushed visible image

Capabilities this model is meant to provide:
- blink can hide and restore text without destroying the underlying content
- stopping blink or forcing phase `ON` should reveal the original desired text
  without requiring the caller to rewrite it
- multiple configured ranges behave as one visible union under the same global
  blink program
- the same logical blink selection can be driven by any external timing source
  because timer ownership stays outside `fm_lcd.*`

Limitations intentionally implied by this model for V1:
- blink is a logical show/hide effect, not hardware-defined blink semantics
- no timer primitive is owned by `fm_lcd.*`
- no independent simultaneous blink programs are planned in V1
- standalone indicators and decimal points are outside the current V1 blink
  scope
- a blink phase change is not visible until the caller reaches the next
  `FM_LCD_Flush()`
- early blink bring-up may use a blocking delay in the app layer without
  implying any timing ownership inside the LCD stack

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
