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
- validate on hardware before reopening architecture
- prefer correction passes driven by LCD and UART evidence
- leave closure passes such as comment cleanup for later unless they are the explicit task

---

## Inspect These References When Needed

- `STYLE.md`
- `AGENTS.md`
- `docs/workflow/README.md`
- `docs/workflow/comment_pass.md`
- `CONTRIBUTING.md`

Inspect the file that applies.
Do not expect `WORKING_CONTEXT.md` to restate their content.

---

## Current Priority

- validate `apps/lcd_bringup/` on hardware
- compare the LCD and UART behavior against the expected scene and milestones
- use that evidence to decide the next correction pass

Expected next options after validation:
- mapping correction
- alpha support
- blink or resume-policy work

---

## Rule

`WORKING_CONTEXT.md` should stay short.

If it starts carrying details that belong to the active task,
move them into the active context file.
