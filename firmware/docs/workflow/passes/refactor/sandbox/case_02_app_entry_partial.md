# Case 02: Partial Fresh Refactor

## User-Style Request

Necesito un refactor chico alrededor de `apps/main/` y `apps/template/`.
Los entry points y los módulos app-locales se están volviendo difíciles de
mantener parejos.

## Safe Repo Evidence

- `apps/README.md` expects `app_entry.c` to stay thin and delegate to an
  app-local module
- `apps/main/app_entry.c` and `apps/template/app_entry.c` both look like thin
  delegates already
- the target area is plausible and narrow enough to inspect

## Blocking Gaps

Still missing before bootstrap:
- preserved behavior
  - what must remain correct besides "keep them similar"
- real structural driver
  - duplication? naming drift? ownership confusion?
- explicit out-of-scope boundary
- concrete next step anchor

## Expected Wizard Questions

The wizard should ask only for the missing blockers, for example:

1. ¿Qué comportamiento querés preservar explícitamente en `main` y `template`?
2. ¿Cuál es el problema estructural real: duplicación, drift de naming o
   ownership poco claro?
3. ¿Qué querés dejar fuera de alcance en este refactor?
4. ¿Querés que el siguiente paso sea inspeccionar sólo `app_entry.*` o también
   los módulos `fm_main.*` y `fm_template.*`?

## Wizard Decision

Do not bootstrap yet.

Ask the blocking questions first.

## Checklist-Oriented Outcome

Expected verdict if the wizard bootstraps too early:
- not approved

Reason:
- preserved behavior would still be vague
- next step would still need interpretation
- out-of-scope would still be weak

## Lesson

The wizard needs an explicit question-only mode before bootstrap.

If the prompt always forces direct output of the context pair, this case will
push it to invent details too early.
