# WORKING_CONTEXT.md

## Current Work

Stage:
- validation

Active pass:
- none

Operational context:

Area:
- `bsp/devices/lcd/`
- `bsp/devices/lcd/pcf8553/`
- `apps/lcd_bringup/`

Core structure:
- the new LCD stack already exists
- alpha support already exists in the public contract and implementation
- validated numeric rows, decimal points, and indicators should be treated as stable base behavior
- the alpha bring-up now includes:
  - left alpha full-on validation
  - right alpha full-on validation
  - sequential sweep of the currently encoded alpha character set
- lowercase letters are now accepted by the alpha mapping layer
- `'#'` is currently reserved as a validation glyph meaning "all 14 alpha
  segments on"

Current focus:
- the last completed step was extending alpha validation in `apps/lcd_bringup/`
- the next useful check is hardware validation of the new alpha scenes
- validated numeric behavior should stay untouched unless alpha validation reveals a real coupling

Constraints:
- keep the validated numeric and indicator path untouched unless alpha validation exposes a real issue
- extend the bring-up only as much as needed to observe alpha behavior
- do not spend this iteration on old-path compatibility, broad cleanup, or blink/resume work

## Next Step

- validate the new alpha scenes on hardware using LCD and UART evidence
- confirm:
  - left/right alpha full-on scenes
  - sequential sweep of digits, uppercase, and lowercase
  - that `'#'` behaves only as a validation glyph and not as an application symbol
- apply the smallest correction only if that validation exposes a real issue

## References

- `WORKING_CONTEXT.md`
- `docs/contexts/lcd_redesign.md`
- `STYLE.md`

## Invocation Rule

Every time this file is used:
- confirm stage, active pass, current focus, constraints, and next step
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

## Strong Rule

This is execution state, not documentation.
No history.
No long explanation.
Detailed rationale stays in `docs/contexts/lcd_redesign.md`.
Keep `WORKING_CONTEXT.md` and its referenced `docs/contexts/` file aligned.
