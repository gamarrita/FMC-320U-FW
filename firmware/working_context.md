# WORKING_CONTEXT.md

## Use This File First

Current working document:
- `docs/contexts/lcd_redesign.md`

Operate there for work touching:
- `bsp/devices/lcd/`
- `bsp/devices/lcd/pcf8553/`
- `apps/lcd_bringup/`
- `legacy_backup/` only when backup material must be inspected

---

## Current Stage

Current workflow stage:
- validation

Use from `docs/workflow/README.md` only what matters for this stage:
- verify the implemented result
- use observed evidence to judge correctness
- keep clear what was validated and what was not

Specific implication for the current LCD work:
- treat numeric rows, decimal points, and indicators as validated base behavior
- focus the next validation on alpha support only
- extend the bring-up only as much as needed to observe alpha behavior
- leave closure passes such as comment cleanup for later unless they are the explicit task

---

## Inspect These References When Needed

- `STYLE.md`
- `AGENTS.md`
- `docs/workflow/README.md`
- `docs/workflow/passes/README.md`
- `CONTRIBUTING.md`

Inspect the file that applies.
Do not expect `WORKING_CONTEXT.md` to restate their content.

---

## Current Priority

- use `docs/contexts/lcd_redesign.md` as the active source of truth for the LCD refactor
- validate the newly implemented alpha support on hardware
- keep the validated numeric and indicator path untouched unless alpha validation exposes a real issue

Expected next options after this validation:
- alpha mapping correction
- alpha bring-up refinement
- blink or resume-policy work

---

## Rule

`WORKING_CONTEXT.md` should stay short.

If it starts carrying details that belong to the active task,
move them into the active context file.
