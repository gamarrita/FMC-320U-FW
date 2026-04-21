# WORKING_CONTEXT.md

## Current Work

Stage:
- implementation

Active pass:
- bringup-blink-scenes

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
- the last completed step was implementing visible blink masking in `fm_lcd.c`
  using the new visible-cell mapping helpers
- the next useful step is extending `apps/lcd_bringup/` with simple blink
  scenes for top row, bottom row, and alpha
- use bring-up next to validate the new visible blink behavior on hardware

Constraints:
- keep visible-cell helpers generic and semantic, not blink-specific
- avoid further `fm_lcd_map.*` changes unless `fm_lcd.c` exposes a real gap in
  the helper contract
- keep timer ownership outside `fm_lcd.*`
- do not spend this iteration on old-path compatibility, broad cleanup, or the higher-level variable editing module

## Next Step

- extend `apps/lcd_bringup/` with simple blink scenes that exercise:
  - top-row visible cell blink
  - bottom-row visible cell blink
  - left and right alpha digit blink
- keep the scenes human-readable from UART and visible LCD output
- validate by canonical build first, then hardware observation

## References

- `docs/contexts/lcd_redesign.md`
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
