# WORKING_CONTEXT.md

## Current Work

Stage:
- validation

Active pass:
- contracts

Operational context:

Area:
- `bsp/devices/lcd/`
- `bsp/devices/lcd/pcf8553/`
- `apps/lcd_bringup/`

Core structure:
- the redesigned LCD stack is the active path
- `fm_lcd.*` owns LCD-visible semantics and desired-state behavior
- `fm_lcd_map.*` remains pure mapping and must stay free of timing or runtime policy
- `fm_lcd_blink.*` owns timer-neutral blink sequencing outside `fm_lcd.*`
- the most important active facts for this step live in both:
  - `WORKING_CONTEXT.md`
  - `docs/contexts/lcd_redesign.md`

Current focus:
- the last completed step was implementing the public logical-blink contracts
- the next useful step is composing visible blink masking inside `fm_lcd.c`
- keep bring-up untouched until the visible blink behavior exists

Constraints:
- keep `fm_lcd_map.*` unchanged during the next blink step
- keep timer ownership outside `fm_lcd.*`
- do not edit bring-up until visible blink behavior exists in the LCD core
- do not spend this iteration on old-path compatibility, broad cleanup, or the higher-level variable editing module

## Next Step

- implement visible blink masking in `bsp/devices/lcd/fm_lcd.c`
- apply the stored blink ranges and current blink phase over the stable desired LCD content before `FM_LCD_Flush()`
- validate by build first, then extend bring-up only after the masking behavior is stable

## References

- `STYLE.md`

## Invocation Rule

Every time this file is used:
- confirm stage, active pass, current focus, constraints, and next step
- for immediate execution, `Constraints` and `Next Step` are authoritative
- if temporary drift exists with the referenced `docs/contexts/` file, this
  file wins for the current step and the referenced file remains the extended
  rationale and backlog
- if the real work drifted, update this file and its referenced `docs/contexts/` file before continuing
- if the request falls outside this context, reframe instead of forcing it into the current track
- load only the references needed for the current step
- if a pass becomes explicit, treat that pass as dominant for the task
- after any material progress, refresh current focus, constraints, and next step

## Maintenance Rule

Keep this file updated when any of these change:
- stage
- active pass
- operational focus
- constraints
- next step
- referenced active context file

Keep the referenced file under `docs/contexts/` updated when any of these change:
- extended technical rationale
- detailed scope
- validated or pending state
- remaining work
- active technical decisions
- contract details that no longer fit in this file

Update both together when:
- the short execution state and the extended context would otherwise diverge
- this file starts pointing to a different active context file
- the active line of work changes materially

Refresh this file immediately after:
- completing the current step
- a verification result changes the next step or constraints
- a blocker or direction change redefines the active line of work

## Strong Rule

This is execution state, not documentation.
No history.
No long explanation.
Detailed rationale stays in `docs/contexts/lcd_redesign.md`.
Keep `WORKING_CONTEXT.md` and its referenced `docs/contexts/` file aligned.
Repeat here only the most important active facts needed to execute the current
step safely.
Do not let this file gradually absorb extended design detail that belongs in
`docs/contexts/lcd_redesign.md`.
