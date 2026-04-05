# 04_codex_prompts.md

[Ir a la versión en español](#version-en-espanol)

## Purpose

This file contains reusable prompt templates for Codex or other execution-oriented agents.

Use this file when the task is already clear and the goal is to inspect, modify,
or generate code with minimal ambiguity.

This file is for direct code inspection and modification prompts.

This file is not the source of truth for style or workflow.
Stable rules live in:

- `AGENT_ENTRY.md`
- `01_quickstart.md`
- `02_workflow.md`
- `STYLE.md`
- `AGENTS.md`

For canonical STM32 build verification, the operational source of truth lives in:

- `docs/stm32cube-build-env.md`

---

## General guidance

Execution prompts should be:

- narrow
- explicit
- bounded
- linked to the correct references
- clear about what must not change

Prefer prompts that target one module, one responsibility, or one local refactor at a time.

When a task changes code that should be validated, prefer one of these two workflows:

- two-step workflow: one prompt to implement, one prompt to verify with the canonical build
- single prompt with explicit final verification only when the task is still small and bounded

Do not duplicate the build contract in each prompt.
Reference `docs/stm32cube-build-env.md` instead.

---

## Prompt template: small implementation

```text
Work in `firmware/`.

Task:
Implement <specific change> in <target file or module>.

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md
- closest relevant example under style-examples/

Constraints:
- keep the change minimal
- do not expand scope
- do not introduce new architecture
- keep public and private split consistent
- follow existing repository naming

Expected result:
- only the necessary files changed
- consistent naming
- reviewable patch
```

---

## Prompt template: small refactor

```text
Work in `firmware/`.

Task:
Perform a small refactor in <target file or module> to achieve <specific goal>.

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md

Constraints:
- refactor only
- no behavior expansion
- no unrelated cleanup
- keep file ownership clear
- do not rename unrelated symbols

Expected result:
- minimal patch
- improved structure
- unchanged intended behavior
```

---

## Prompt template: split mixed-responsibility module

```text
Work in `firmware/`.

Task:
Analyze and split mixed responsibilities in <target module> only if a minimal split is justified.

References:
- firmware/AGENT_ENTRY.md
- firmware/01_quickstart.md
- firmware/STYLE.md
- firmware/style-examples/README.md

Constraints:
- preserve behavior
- prefer minimal movement
- do not redesign the architecture
- keep naming repository-consistent
- explain the new ownership boundary

Expected output:
- proposed split
- affected files
- rationale for ownership
- minimal implementation
```

---

## Prompt template: create new module

```text
Work in `firmware/`.

Task:
Create a new module for <specific responsibility> in the appropriate folder.

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md
- closest relevant example under style-examples/

Constraints:
- choose the correct owning folder
- create only the minimum necessary public API
- keep helpers private in the `.c` file
- follow repository naming exactly
- do not add extra abstraction layers

Expected result:
- new `.h` and `.c` pair if justified
- clear responsibility
- minimal API surface
- consistent structure
```

---

## Prompt template: naming cleanup

```text
Work in `firmware/`.

Task:
Adjust naming in <target file or module> to match repository conventions.

References:
- firmware/STYLE.md
- firmware/style-examples/README.md

Constraints:
- rename only what is necessary
- keep scope local
- do not combine with architecture changes
- preserve behavior

Expected result:
- repository-consistent names
- minimal patch
- no unrelated edits
```

---

## Prompt template: review before edit

```text
Work in `firmware/`.

Task:
Inspect <target file or module> and report:
1. main responsibility
2. public API
3. private helpers
4. naming mismatches against STYLE.md
5. whether the file appears to mix concerns
6. minimal recommended next step

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md

Do not modify code yet.
Only inspect and report.
```

---

## Prompt template: documentation-aligned code update

```text
Work in `firmware/`.

Task:
Update code in <target> so it aligns with the documented naming and module conventions.

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/AGENTS.md
- firmware/style-examples/README.md

Constraints:
- no broad cleanup
- no speculative improvements
- keep the patch reviewable
- use the existing local pattern whenever valid

Expected result:
- code aligned with docs
- minimal necessary edits
```

---

## Prompt template: implementation with canonical build verification

```text
Work in `firmware/`.

Task:
Implement <specific change> in <target file or module>, then verify the result with the canonical STM32 build of the repo.

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md
- firmware/docs/stm32cube-build-env.md
- closest relevant example under style-examples/

Constraints:
- keep the change minimal
- do not expand scope
- do not introduce new architecture
- keep public and private split consistent
- do not invent a parallel build flow
- use the canonical build workflow documented in `docs/stm32cube-build-env.md`

Validation:
- after editing, verify the change with the canonical build of the repo
- if the shell is not ready, bootstrap it following `docs/stm32cube-build-env.md`
- if build verification is not possible, explain exactly why

Expected result:
- only the necessary files changed
- summary of the code change
- result of the canonical build verification
- explicit note if the final limitation is from environment or code
```

---

## Prompt template: canonical build verification after changes

```text
Work in `firmware/`.

Task:
Verify the current state of the repo with the canonical STM32 build workflow, without making unrelated code changes.

References:
- firmware/02_workflow.md
- firmware/AGENTS.md
- firmware/docs/stm32cube-build-env.md

Instructions:
- first check whether the shell already has the repo bootstrap loaded or is still clean
- follow `docs/stm32cube-build-env.md` as the source of truth for the build workflow
- if needed, load the STM32 bootstrap for the repo
- run the canonical configure/build entrypoint documented by the repo
- do not replace it with an arbitrary system `cmake`

Report:
- whether the shell needed bootstrap
- which canonical command path was used
- whether configure succeeded
- whether build succeeded
- resulting artifact or the relevant failure
- whether the problem appears to be environment-related or code-related
```

---

## Standard closing clause for execution prompts

When validation matters, append a clause like:

```text
Before closing, verify the result with the canonical build of the repo according to `firmware/docs/stm32cube-build-env.md`. If you cannot validate it, explain exactly why.
```

Use this clause:
- before closing an implementation task
- before asking for commit-ready confirmation
- before claiming a functional fix is complete

It is not required for pure inspection prompts or very small exploratory steps.

---

## Notes

Use this file after the task has already been clarified.

Do not use execution prompts as a substitute for:
- architecture decisions
- workflow definitions
- stable style rules
- the canonical STM32 build contract

If the prompt needs to explain repository rules in full,
the stable documentation is incomplete and should be improved.

---

## Version en espanol

## Proposito

Este archivo contiene plantillas de prompts reutilizables para Codex u otros agentes orientados a ejecucion.

Usa este archivo cuando la tarea ya este clara y el objetivo sea inspeccionar, modificar
o generar codigo con la menor ambiguedad posible.

Este archivo esta pensado para prompts directos de inspeccion y modificacion de codigo.

Este archivo no es la fuente de verdad de estilo ni de workflow.
Las reglas estables viven en:

- `AGENT_ENTRY.md`
- `01_quickstart.md`
- `02_workflow.md`
- `STYLE.md`
- `AGENTS.md`

Para verificacion canonica del build STM32, la fuente de verdad operativa vive en:

- `docs/stm32cube-build-env.md`

---

## Guia general

Los prompts de ejecucion deberian ser:

- acotados
- explicitos
- delimitados
- enlazados con las referencias correctas
- claros respecto de lo que no debe cambiar

Prefiere prompts que apunten a un modulo, una responsabilidad o un refactor local por vez.

Cuando una tarea cambia codigo que deberia validarse, prefiere uno de estos dos workflows:

- workflow en dos pasos: un prompt para implementar y otro para verificar con el build canonico
- un solo prompt con verificacion final explicita solo cuando la tarea sigue siendo pequena y acotada

No dupliques el contrato del build en cada prompt.
En su lugar, referencia `docs/stm32cube-build-env.md`.

---

## Plantilla de prompt: implementacion pequena

```text
Work in `firmware/`.

Task:
Implement <specific change> in <target file or module>.

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md
- closest relevant example under style-examples/

Constraints:
- keep the change minimal
- do not expand scope
- do not introduce new architecture
- keep public and private split consistent
- follow existing repository naming

Expected result:
- only the necessary files changed
- consistent naming
- reviewable patch
```

---

## Plantilla de prompt: refactor pequeno

```text
Work in `firmware/`.

Task:
Perform a small refactor in <target file or module> to achieve <specific goal>.

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md

Constraints:
- refactor only
- no behavior expansion
- no unrelated cleanup
- keep file ownership clear
- do not rename unrelated symbols

Expected result:
- minimal patch
- improved structure
- unchanged intended behavior
```

---

## Plantilla de prompt: dividir modulo con responsabilidades mezcladas

```text
Work in `firmware/`.

Task:
Analyze and split mixed responsibilities in <target module> only if a minimal split is justified.

References:
- firmware/AGENT_ENTRY.md
- firmware/01_quickstart.md
- firmware/STYLE.md
- firmware/style-examples/README.md

Constraints:
- preserve behavior
- prefer minimal movement
- do not redesign the architecture
- keep naming repository-consistent
- explain the new ownership boundary

Expected output:
- proposed split
- affected files
- rationale for ownership
- minimal implementation
```

---

## Plantilla de prompt: crear modulo nuevo

```text
Work in `firmware/`.

Task:
Create a new module for <specific responsibility> in the appropriate folder.

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md
- closest relevant example under style-examples/

Constraints:
- choose the correct owning folder
- create only the minimum necessary public API
- keep helpers private in the `.c` file
- follow repository naming exactly
- do not add extra abstraction layers

Expected result:
- new `.h` and `.c` pair if justified
- clear responsibility
- minimal API surface
- consistent structure
```

---

## Plantilla de prompt: limpieza de naming

```text
Work in `firmware/`.

Task:
Adjust naming in <target file or module> to match repository conventions.

References:
- firmware/STYLE.md
- firmware/style-examples/README.md

Constraints:
- rename only what is necessary
- keep scope local
- do not combine with architecture changes
- preserve behavior

Expected result:
- repository-consistent names
- minimal patch
- no unrelated edits
```

---

## Plantilla de prompt: review antes de editar

```text
Work in `firmware/`.

Task:
Inspect <target file or module> and report:
1. main responsibility
2. public API
3. private helpers
4. naming mismatches against STYLE.md
5. whether the file appears to mix concerns
6. minimal recommended next step

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md

Do not modify code yet.
Only inspect and report.
```

---

## Plantilla de prompt: actualizacion de codigo alineada con documentacion

```text
Work in `firmware/`.

Task:
Update code in <target> so it aligns with the documented naming and module conventions.

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/AGENTS.md
- firmware/style-examples/README.md

Constraints:
- no broad cleanup
- no speculative improvements
- keep the patch reviewable
- use the existing local pattern whenever valid

Expected result:
- code aligned with docs
- minimal necessary edits
```

---

## Plantilla de prompt: implementacion con verificacion de build canonico

```text
Work in `firmware/`.

Task:
Implement <specific change> in <target file or module>, then verify the result with the canonical STM32 build of the repo.

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md
- firmware/docs/stm32cube-build-env.md
- closest relevant example under style-examples/

Constraints:
- keep the change minimal
- do not expand scope
- do not introduce new architecture
- keep public and private split consistent
- do not invent a parallel build flow
- use the canonical build workflow documented in `docs/stm32cube-build-env.md`

Validation:
- after editing, verify the change with the canonical build of the repo
- if the shell is not ready, bootstrap it following `docs/stm32cube-build-env.md`
- if build verification is not possible, explain exactly why

Expected result:
- only the necessary files changed
- summary of the code change
- result of the canonical build verification
- explicit note if the final limitation is from environment or code
```

---

## Plantilla de prompt: verificacion canonica de build despues de cambios

```text
Work in `firmware/`.

Task:
Verify the current state of the repo with the canonical STM32 build workflow, without making unrelated code changes.

References:
- firmware/02_workflow.md
- firmware/AGENTS.md
- firmware/docs/stm32cube-build-env.md

Instructions:
- first check whether the shell already has the repo bootstrap loaded or is still clean
- follow `docs/stm32cube-build-env.md` as the source of truth for the build workflow
- if needed, load the STM32 bootstrap for the repo
- run the canonical configure/build entrypoint documented by the repo
- do not replace it with an arbitrary system `cmake`

Report:
- whether the shell needed bootstrap
- which canonical command path was used
- whether configure succeeded
- whether build succeeded
- resulting artifact or the relevant failure
- whether the problem appears to be environment-related or code-related
```

---

## Clausula estandar de cierre para prompts de ejecucion

Cuando la validacion importa, agrega una clausula como esta:

```text
Before closing, verify the result with the canonical build of the repo according to `firmware/docs/stm32cube-build-env.md`. If you cannot validate it, explain exactly why.
```

Usa esta clausula:

- antes de cerrar una tarea de implementacion
- antes de pedir confirmacion de que esta listo para commit
- antes de afirmar que una correccion funcional esta terminada

No es necesaria para prompts puramente de inspeccion o para pasos exploratorios muy pequenos.

---

## Notas

Usa este archivo despues de que la tarea ya fue aclarada.

No uses prompts de ejecucion como sustituto de:

- decisiones de arquitectura
- definiciones de workflow
- reglas estables de estilo
- el contrato canonico del build STM32

Si el prompt necesita explicar las reglas del repositorio en detalle,
entonces la documentacion estable esta incompleta y deberia mejorarse.
