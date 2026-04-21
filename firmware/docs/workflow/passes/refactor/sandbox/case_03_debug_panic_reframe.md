# Case 03: Reframe Request

## User-Style Request

Quiero reencuadrar un refactor sobre `libs/fm_debug.*` y `apps/panic_demo/`.
Me preocupa mezclar calibración de panic con los servicios debug.
Quiero preservar el comportamiento visible actual del demo y de los helpers de
debug, pero cambiar el foco del track.

## Safe Repo Evidence

- `libs/fm_debug.c` already mixes panic paths, UART helpers, LED control and
  event queue behavior
- `apps/panic_demo/fm_panic_demo.c` is a tutorial app focused on panic/fault
  calibration
- the request explicitly says "reencuadrar", not just "iniciar"

## Blocking Gaps

Still missing before safe reframing:
- which active context pair is being reframed
- what remains in scope after the reframe
- what is explicitly out of scope in the new narrowed track
- what immediate next analysis step should replace the old one

## Expected Wizard Questions

The wizard should ask short reframing questions, for example:

1. ¿Qué `working_context.md` / `docs/contexts/<slug>.md` querés reencuadrar?
2. En el nuevo foco, ¿qué parte querés dejar fuera de alcance: el demo,
   `fm_debug`, o la relación entre ambos?
3. ¿Cuál sería el siguiente paso concreto después del reencuadre?

## Wizard Decision

Do not create a fresh context pair blindly.

First determine whether the operation is:
- reframe of an existing active pair
- or fresh bootstrap of a new refactor track

## Checklist-Oriented Outcome

Expected verdict if the wizard ignores reframe mode:
- approved with risk at best

Reason:
- the resulting pair may lose continuity
- ownership of the active track would remain unclear
- the wizard could create unnecessary duplicate context files

## Lesson

Create-versus-reframe behavior is not optional.

The wizard must branch explicitly when the user says the task is a reframe.
