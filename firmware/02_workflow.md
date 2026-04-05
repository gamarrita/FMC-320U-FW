# 02_workflow.md

[Ir a la versión en español](#version-en-espanol)

## Purpose

This document describes the working workflow for `firmware/`
when collaborating with AI assistants and code agents.

This is the full workflow document.
It is more detailed than `01_quickstart.md`.

Use this file to understand roles, sequencing, task framing,
handoff boundaries, and continuity management.

---

## Workflow goals

The workflow is designed to:

- keep tasks small and verifiable
- reduce accidental architecture drift
- improve consistency of generated code
- separate thinking from execution
- make progress easy to review
- preserve continuity across sessions

---

## Main workflow roles

### ChatGPT or planning assistant

Use for:

- clarifying problem framing
- comparing options
- refining naming or module boundaries
- reviewing whether a change belongs in `app/`, `services/`, or another area
- drafting prompts for execution agents
- reviewing documentation structure
- turning a large task into smaller tasks

This role should help think before editing.

### Codex or execution-oriented agent

Use for:

- inspecting target files
- implementing small scoped changes
- generating code following repository style
- adjusting file structure
- performing narrow refactors
- applying a clearly described change set

This role should execute against a concrete task.

---

## Standard task cycle

### Step 1: define the task

Describe the task in a small and testable way.

The task should identify:

- target files or module
- expected owning folder
- boundaries of what should not change
- reference docs
- whether the task is implementation, cleanup, naming correction, or split

### Step 2: identify repository references

Before editing, identify the governing references:

- `STYLE.md` for naming and structure
- `AGENTS.md` for agent constraints
- `style-examples/` for the nearest valid pattern
- `CHANGELOG.md` if continuity from earlier work matters
- `05_working_context.md` if the session depends on current live status

### Step 3: inspect current code

Read the target code and nearby files.

Confirm:

- current naming style
- current public and private split
- dependency direction
- whether the file already contains mixed responsibilities
- whether an existing local pattern should be preserved

### Step 4: perform one bounded change

Prefer one of these per iteration:

- add one missing piece
- rename one API surface
- split one module responsibility
- create one small module
- update one file pair (`.h` plus `.c`)
- document one stable rule

Avoid mixed-purpose iterations.

### Step 5: review the result

Check:

- naming consistency
- ownership correctness
- module boundaries
- minimal scope
- similarity to accepted patterns in the repo

### Step 6: record continuity if needed

If the work is incomplete or staged across sessions:

- update `05_working_context.md`
- update `CHANGELOG.md` only if the change is meaningful project history
- do not place temporary planning notes into stable docs

---

## Recommended document roles

### Stable documents

Use these for durable rules:

- `AGENT_ENTRY.md`
- `01_quickstart.md`
- `02_workflow.md`
- `STYLE.md`
- `AGENTS.md`
- `style-examples/README.md`

### Operational prompt libraries

Use these for tool-specific prompting:

- `03_chatgpt_prompts.md`
- `04_codex_prompts.md`

### Mutable continuity document

Use this for live state:

- `05_working_context.md`

### Historical log

Use this for significant milestones:

- `CHANGELOG.md`

---

## Task sizing guidance

A task is usually well sized if:

- it affects one module or one narrow behavior
- it can be reviewed without reconstructing unrelated context
- success criteria are obvious
- style checks are local and concrete

A task is probably too large if:

- it changes several ownership boundaries at once
- it introduces a new pattern and refactors old ones together
- it spans unrelated folders
- it mixes naming cleanup with architecture changes

When in doubt, split the task.

---

## Handoff guidance

When preparing an execution prompt for an agent, include:

- the exact target
- the intended folder ownership
- the governing references
- the no-go boundaries
- the expectation of minimal change

Good handoff pattern:

> Update `<target>` to `<specific result>`.  
> Follow `AGENT_ENTRY.md` and `STYLE.md`.  
> Use the closest relevant example under `style-examples/`.  
> Keep the patch minimal and do not expand scope beyond `<boundary>`.

---

## Continuity guidance

Use `05_working_context.md` only for live session continuity.

It may include:

- current focus
- current assumptions
- known next step
- incomplete tasks
- temporary decisions awaiting confirmation

It must not become the source of truth for style, architecture, or permanent workflow.

---

## Anti-patterns

Avoid these workflow failures:

- asking the agent to "fix everything"
- skipping `STYLE.md`
- using examples as architecture instead of format guidance
- relying on a mutable context file as the main source of truth
- combining planning, implementation, and cleanup in one oversized prompt
- making undocumented naming exceptions inside generated code

---

## Related documents

- Entry point: `AGENT_ENTRY.md`
- Quick start: `01_quickstart.md`
- Style and naming: `STYLE.md`
- ChatGPT prompts: `03_chatgpt_prompts.md`
- Codex prompts: `04_codex_prompts.md`
- Live context: `05_working_context.md`

---

## Version en espanol

## Proposito

Este documento describe el workflow de trabajo para `firmware/`
cuando se colabora con asistentes de IA y agentes de codigo.

Este es el documento completo del workflow.
Es mas detallado que `01_quickstart.md`.

Usa este archivo para entender roles, secuencias, forma de plantear tareas,
limites de handoff y manejo de continuidad.

---

## Objetivos del workflow

El workflow esta disenado para:

- mantener las tareas pequenas y verificables
- reducir deriva arquitectonica accidental
- mejorar la consistencia del codigo generado
- separar pensamiento de ejecucion
- hacer que el progreso sea facil de revisar
- preservar continuidad entre sesiones

---

## Roles principales del workflow

### ChatGPT o asistente de planificacion

Usar para:

- aclarar el planteo del problema
- comparar opciones
- refinar naming o limites de modulos
- revisar si un cambio pertenece a `app/`, `services/` u otra area
- redactar prompts para agentes de ejecucion
- revisar la estructura de la documentacion
- convertir una tarea grande en tareas mas chicas

Este rol deberia ayudar a pensar antes de editar.

### Codex o agente orientado a ejecucion

Usar para:

- inspeccionar archivos objetivo
- implementar cambios pequenos y acotados
- generar codigo siguiendo el estilo del repositorio
- ajustar estructura de archivos
- realizar refactors estrechos
- aplicar un conjunto de cambios claramente descrito

Este rol deberia ejecutar sobre una tarea concreta.

---

## Ciclo estandar de una tarea

### Paso 1: definir la tarea

Describe la tarea de forma pequena y testeable.

La tarea deberia identificar:

- archivos o modulo objetivo
- carpeta duena esperada
- limites de lo que no deberia cambiar
- documentos de referencia
- si la tarea es de implementacion, limpieza, correccion de naming o split

### Paso 2: identificar referencias del repositorio

Antes de editar, identifica las referencias que gobiernan la tarea:

- `STYLE.md` para naming y estructura
- `AGENTS.md` para restricciones de agentes
- `style-examples/` para el patron valido mas cercano
- `CHANGELOG.md` si importa la continuidad con trabajo anterior
- `05_working_context.md` si la sesion depende del estado vivo actual

### Paso 3: inspeccionar el codigo actual

Lee el codigo objetivo y los archivos cercanos.

Confirma:

- el estilo de naming actual
- la division actual entre publico y privado
- la direccion de dependencias
- si el archivo ya contiene responsabilidades mezcladas
- si un patron local existente deberia preservarse

### Paso 4: realizar un unico cambio acotado

Prefiere una de estas opciones por iteracion:

- agregar una pieza faltante
- renombrar una unica superficie de API
- dividir una unica responsabilidad de modulo
- crear un modulo pequeno
- actualizar un unico par de archivos (`.h` y `.c`)
- documentar una unica regla estable

Evita iteraciones de proposito mezclado.

### Paso 5: revisar el resultado

Verifica:

- consistencia de naming
- correccion de ownership
- limites del modulo
- alcance minimo
- similitud con patrones aceptados del repo

### Paso 6: registrar continuidad si hace falta

Si el trabajo esta incompleto o repartido entre sesiones:

- actualiza `05_working_context.md`
- actualiza `CHANGELOG.md` solo si el cambio forma parte de la historia significativa del proyecto
- no pongas notas temporales de planificacion dentro de documentacion estable

---

## Roles recomendados para los documentos

### Documentos estables

Usa estos para reglas duraderas:

- `AGENT_ENTRY.md`
- `01_quickstart.md`
- `02_workflow.md`
- `STYLE.md`
- `AGENTS.md`
- `style-examples/README.md`

### Librerias operativas de prompts

Usa estos para prompting especifico de herramienta:

- `03_chatgpt_prompts.md`
- `04_codex_prompts.md`

### Documento mutable de continuidad

Usa este para estado vivo:

- `05_working_context.md`

### Registro historico

Usa este para hitos significativos:

- `CHANGELOG.md`

---

## Guia para dimensionar tareas

Una tarea suele estar bien dimensionada si:

- afecta un modulo o un comportamiento estrecho
- puede revisarse sin reconstruir contexto no relacionado
- los criterios de exito son obvios
- los chequeos de estilo son locales y concretos

Una tarea probablemente es demasiado grande si:

- cambia varios limites de ownership a la vez
- introduce un patron nuevo y refactoriza patrones viejos en la misma pasada
- se extiende por carpetas no relacionadas
- mezcla limpieza de naming con cambios de arquitectura

Ante la duda, divide la tarea.

---

## Guia de handoff

Cuando prepares un prompt de ejecucion para un agente, incluye:

- el objetivo exacto
- el ownership de carpeta esperado
- las referencias que gobiernan la tarea
- los limites de lo que no debe tocarse
- la expectativa de cambio minimo

Buen patron de handoff:

> Actualiza `<objetivo>` para lograr `<resultado especifico>`.  
> Sigue `AGENT_ENTRY.md` y `STYLE.md`.  
> Usa el ejemplo mas cercano bajo `style-examples/`.  
> Manten el patch minimo y no expandas el alcance mas alla de `<limite>`.

---

## Guia de continuidad

Usa `05_working_context.md` solo para continuidad viva de la sesion.

Puede incluir:

- foco actual
- supuestos actuales
- siguiente paso conocido
- tareas incompletas
- decisiones temporales pendientes de confirmacion

No debe convertirse en la fuente de verdad de estilo, arquitectura o workflow permanente.

---

## Anti-patrones

Evita estos fallos de workflow:

- pedirle al agente que "arregle todo"
- saltearse `STYLE.md`
- usar ejemplos como arquitectura en lugar de usarlos como guia de formato
- depender de un archivo de contexto mutable como fuente principal de verdad
- combinar planificacion, implementacion y limpieza en un solo prompt sobredimensionado
- hacer excepciones de naming no documentadas dentro del codigo generado

---

## Documentos relacionados

- Punto de entrada: `AGENT_ENTRY.md`
- Inicio rapido: `01_quickstart.md`
- Estilo y naming: `STYLE.md`
- Prompts para ChatGPT: `03_chatgpt_prompts.md`
- Prompts para Codex: `04_codex_prompts.md`
- Contexto vivo: `05_working_context.md`
