> Human reference only.
>
> This note preserves a historical prompt/context window for human review.
> It is not part of the active agent route, does not define the current
> workstream, and should not be used as source of truth for implementation.
>
> For agent work, start with `AGENTS.md` and `WORKING_CONTEXT.md`, then follow
> the applicable roadmap and specifications referenced there.
>
> Do not apply `docs/workflow/doc_closure.md` to delete or compress this file
> unless the user explicitly asks to curate human tutorial/reference material.

Estamos trabajando en `D:\githubs\FMC-320U_V2_VSCODE`.

Primero leer:

1. `AGENTS.md`
2. `WORKING_CONTEXT.md`
3. el contexto activo indicado por `WORKING_CONTEXT.md`

El repo acaba de pasar por una limpieza documental. Respetar especialmente:

- el agente debe empezar por `AGENTS.md` y `WORKING_CONTEXT.md`, y luego seguir
  el roadmap y las especificaciones aplicables
- solo `WORKING_CONTEXT.md` define la tarea/workstream actual
- no conservar contextos cerrados como respaldo historico; Git cumple ese rol
- aplicar `docs/workflow/doc_closure.md` antes de commits o cambios de contexto

Estado de referencia historica:

- la linea de trabajo era definir la frontera `fmc_service` / `fmc_runtime`
- el roadmap aplicable fue absorbido por `docs/roadmaps/fmc_refactoring.md`
- keyboard short-press bring-up ya estaba cerrado
- `main` estaba pusheado y limpio hasta el commit `2b8c586`

Intencion original:

```text
Continuar desde ahi, eligiendo el primer sub-slice de fmc_service /
fmc_runtime sin agrandar documentacion innecesariamente.
```
