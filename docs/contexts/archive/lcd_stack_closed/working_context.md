# WORKING_CONTEXT.md

## Current Work

Stage:
- closure

Active pass:
- stack-closure-docs

Operational context:

Area:
- `bsp/devices/lcd/`
- `bsp/devices/lcd/pcf8553/`
- `apps/lcd_bringup/`
- `apps/lcd_blink_bringup/`

Core structure:
- the redesigned LCD stack is the active path
- `fm_lcd.*` owns LCD-visible semantics and desired-state behavior
- `fm_lcd_map.*` remains pure mapping and must stay free of timing or runtime policy
- `fm_lcd_blink.*` owns timer-neutral blink sequencing outside `fm_lcd.*`
- the most important active facts for this step live in both:
  - `WORKING_CONTEXT.md`
  - `docs/contexts/archive/lcd_stack_closed/lcd_redesign.md`

Current focus:
- the LCD stack is now considered complete for its intended scope
- `apps/lcd_bringup/` remains the static base-validation app
- `apps/lcd_blink_bringup/` remains the temporal validation app for logical
  blink behavior
- the current task is closing the documentation around the finished stack state

Constraints:
- keep visible-cell helpers generic and semantic, not blink-specific
- do not reopen the base LCD stack unless a real visual defect or a new
  product requirement appears
- keep timer ownership outside `fm_lcd.*`
- keep blink timing outside `fm_lcd.*` even in bring-up
- allow a simple blocking delay inside `lcd_blink_bringup` for this validation
  phase
- keep `apps/lcd_bringup/` focused on static LCD validation
- prefer one case-insensitive visible result per 7-seg letter intent, not
  separate uppercase/lowercase runtime behavior
- do not spend this iteration on old-path compatibility, broad cleanup, or the higher-level variable editing module

## Next Step

- keep the stack closed unless new evidence requires a targeted correction
- move future work to higher-level modules that consume the LCD primitives

## References

- `docs/contexts/archive/lcd_stack_closed/lcd_redesign.md`
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
Detailed rationale stays in `docs/contexts/archive/lcd_stack_closed/lcd_redesign.md`.
Keep `WORKING_CONTEXT.md` and its referenced `docs/contexts/` file aligned.
Repeat here only the most important active facts needed to execute the current
step safely.
Do not let this file gradually absorb extended design detail that belongs in
`docs/contexts/archive/lcd_stack_closed/lcd_redesign.md`.
