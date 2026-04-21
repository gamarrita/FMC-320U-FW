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

Current focus:
- validate alpha behavior on hardware without destabilizing the validated numeric base

Constraints:
- keep the validated numeric and indicator path untouched unless alpha validation exposes a real issue
- extend the bring-up only as much as needed to observe alpha behavior
- do not spend this iteration on old-path compatibility, broad cleanup, or blink/resume work

## Next Step

- extend `apps/lcd_bringup/` with alpha scenes only as much as needed
- validate alpha behavior using LCD and UART evidence
- apply the smallest correction only if that validation exposes a real issue

## References

- `docs/contexts/lcd_redesign.md`
- `STYLE.md`

## Strong Rule

This is execution state, not documentation.
No history.
No long explanation.
Detailed rationale stays in `docs/contexts/lcd_redesign.md`.
