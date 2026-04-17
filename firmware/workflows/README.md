# Workflow Stages

[Ir a la versión en español](#version-en-español)


## Purpose
This directory defines how work is split into small, focused stages.

The goal is not to force every task through the same full process.
The goal is to make it clear what kind of pass is being done at a given moment,
what that pass is allowed to change, and what "done" means for that pass.

This is especially important when working with agents such as ChatGPT and Codex:
- first-pass outputs are often functionally useful but not fully aligned with naming, comments, cleanup, or validation expectations
- asking for everything at once usually reduces consistency
- different stages need different instructions, context, and review criteria

This workflow exists to reduce mixed goals inside a single request.

---

## Core idea
A task may require one or more stages.

Typical examples:
- analysis only
- analysis and plan
- implement a small change
- run a comment pass
- run a validation pass

Not every task needs every stage.

A stage should have:
- a clear goal
- a limited scope
- explicit non-goals
- a visible output that can be reviewed

---

## Recommended stage sequence
When needed, work should normally be split in this order:

1. Analysis
2. Plan
3. Implementation
4. Comment pass
5. Validation

This is the default sequence, not a rigid rule.

For small tasks, analysis and plan may be combined into a single pass.

Examples:
- a small bugfix may use only analysis, implementation, validation
- a larger change may use analysis, plan, implementation, comment pass, validation
- a documentation cleanup may use only comment pass

---

## Stage definitions

### 1. Analysis
Use this stage to understand the current code before changing it.

This stage should answer:
- how the module currently works
- what nearby files already do
- what local naming and structure patterns exist
- what risks exist for the requested change
- what should remain untouched

Typical output:
- short technical findings
- recommended insertion point
- list of relevant files or patterns
- warnings about coupling, ownership, ISR context, init order, or hardware assumptions

Analysis should not drift into large speculative redesign unless explicitly requested.

---

### 2. Plan
Use this stage to define how the change should be approached after analysis is complete.

This stage should define:
- the target files or module
- the exact intended change
- what is out of scope
- whether behavior is allowed to change
- whether later passes are expected
- the recommended sequence of edits

Typical output:
- a narrow task statement
- affected files
- acceptance criteria
- a short implementation strategy

For small tasks, this stage may be merged with analysis.

---

### 3. Implementation
Use this stage to make the functional or structural change.

This stage may include:
- adding code
- changing behavior
- moving code
- splitting functions
- improving naming
- removing dead code
- restructuring internals
- refactoring code without changing intended behavior

This stage should stay focused on the requested code change.

Unless explicitly requested, this stage should not try to solve all of the following at once:
- full comment cleanup
- full naming normalization across the module
- broad architectural redesign
- exhaustive validation
- unrelated style corrections

Typical output:
- the requested code change
- short summary of what changed
- explicit note of what remains for later passes, if anything

---

### 4. Comment pass
Use this stage after implementation or refactor, when the code structure is already stable enough to document clearly.

This stage exists because comments are easier to do well after behavior and structure are settled.

This stage may include:
- adding missing API comments
- removing redundant comments
- fixing inaccurate comments
- making comment style more consistent
- documenting constraints that are not obvious from the code

This stage should not be used as a substitute for poor naming or poor structure.

Typical output:
- public API comments aligned with actual behavior
- private comments only where they add real technical value
- removal of stale or misleading comments

A dedicated document for this stage should define the detailed comment rules.

---

### 5. Validation
Use this stage to check that the result is acceptable for integration.

Validation may include:
- build verification
- targeted testing
- debug checks
- runtime sanity checks
- confirmation that behavior did not change unexpectedly
- confirmation that the requested change is actually complete

This stage should make clear what was and was not verified.

Typical output:
- what was validated
- how it was validated
- what remains unverified

Validation is not optional by principle, but the required depth depends on the task.

---

## How to use this directory
This directory should contain short stage-specific documents.

Each stage document should answer:
- when to use it
- what it is allowed to change
- what it must not change
- what "done" means for that stage

Stage documents should be operational.
They should help humans and agents perform a pass.
They should not become long general manuals.

---

## Relationship with root-level documents

### Root-level documents should keep:
- stable project-wide rules
- coding style rules
- naming rules
- architecture rules that apply broadly
- repository entrypoints
- long-lived context that should remain easy to find

Examples:
- `AGENT_ENTRY.md`
- `STYLE.md`
- `AGENTS.md`
- `README.md`

### Workflow documents should contain:
- stage-specific instructions
- stage scope and non-goals
- pass-specific completion criteria
- examples of how to request or review that pass

In short:
- root documents define enduring project rules
- workflow documents define when and how a stage is applied

---

## Migration approach
This repository does not need a full documentation rewrite before using stage-based workflow.

Migration should be incremental.

Recommended order:
1. Define this stage model
2. Add one stage document with immediate value
3. Apply it on a real module
4. Refine based on actual use
5. Only then extract or rewrite other workflow material

The first recommended stage document is:
- `comment_pass.md`

Reason:
- comment consistency is currently uneven across the repo
- comments are often better handled after implementation stabilizes
- this stage can be applied with low risk
- this stage can be reviewed clearly

---

## Practical rule
Do not assume that a first pass must solve everything.

When needed, request the next pass explicitly.

Examples:
- "analyze this module first"
- "analyze and propose a plan"
- "implement the refactor only"
- "run a comment pass on these two files"
- "run a validation pass for this change"

This should be preferred over combining multiple goals in one vague request.

---

## Initial convention
Until more stage documents exist, use these default meanings:

- analysis: understand current code and local constraints
- plan: define the intended change and recommended sequence
- implementation: change code
- comment pass: improve comments after code stabilizes
- validation: verify integration readiness

Future stage documents may refine these meanings, but should not contradict them without good reason.

# Version en Español

# Etapas del workflow

## Propósito
Este directorio define cómo dividir el trabajo en etapas pequeñas y enfocadas.

El objetivo no es forzar que todas las tareas pasen por el mismo proceso completo.
El objetivo es dejar claro qué tipo de pasada se está haciendo en cada momento,
qué se puede cambiar en esa pasada, y qué significa que esa pasada esté "terminada".

Esto es especialmente importante cuando se trabaja con agentes como ChatGPT y Codex:
- una primera pasada suele ser funcionalmente útil, pero no necesariamente alineada con naming, comentarios, limpieza o validación
- pedir todo junto normalmente reduce la consistencia
- distintas etapas necesitan distintas instrucciones, distinto contexto y distintos criterios de revisión

Este workflow existe para evitar mezclar objetivos dentro de un mismo pedido.

---

## Idea principal
Una tarea puede requerir una o más etapas.

Ejemplos típicos:
- solo análisis
- análisis y plan
- implementar un cambio chico
- hacer una pasada de comentarios
- hacer una pasada de validación

No todas las tareas necesitan todas las etapas.

Cada etapa debería tener:
- un objetivo claro
- un alcance limitado
- no objetivos explícitos
- una salida visible que se pueda revisar

---

## Secuencia recomendada
Cuando haga falta, el trabajo debería dividirse normalmente en este orden:

1. Análisis
2. Plan
3. Implementación
4. Pasada de comentarios
5. Validación

Esta es la secuencia por defecto, no una regla rígida.

Para tareas chicas, análisis y plan pueden combinarse en una sola pasada.

Ejemplos:
- un bugfix chico puede usar solo análisis, implementación y validación
- un cambio más grande puede usar análisis, plan, implementación, pasada de comentarios y validación
- una mejora documental puede usar solo pasada de comentarios

---

## Definición de etapas

### 1. Análisis
Usar esta etapa para entender el código actual antes de cambiarlo.

Esta etapa debería responder:
- cómo funciona actualmente el módulo
- qué hacen archivos cercanos similares
- qué patrones locales de naming y estructura ya existen
- qué riesgos tiene el cambio pedido
- qué conviene no tocar

Salida típica:
- hallazgos técnicos breves
- punto recomendado de inserción o cambio
- lista de archivos o patrones relevantes
- advertencias sobre acoplamiento, ownership, contexto ISR, orden de init o supuestos de hardware

El análisis no debería derivar en un rediseño amplio especulativo salvo que se pida explícitamente.

---

### 2. Plan
Usar esta etapa para definir cómo conviene abordar el cambio una vez terminado el análisis.

Esta etapa debería definir:
- archivos o módulo objetivo
- cambio exacto buscado
- qué queda fuera de alcance
- si se permite cambiar comportamiento
- si se esperan pasadas posteriores
- secuencia recomendada de edición

Salida típica:
- una definición acotada de la tarea
- archivos afectados
- criterio de aceptación
- una estrategia breve de implementación

Para tareas chicas, esta etapa puede fusionarse con análisis.

---

### 3. Implementación
Usar esta etapa para hacer el cambio funcional o estructural.

Esta etapa puede incluir:
- agregar código
- cambiar comportamiento
- mover código
- dividir funciones
- mejorar naming
- remover código muerto
- reestructurar internals
- refactorizar código sin cambiar el comportamiento esperado

Esta etapa debería mantenerse enfocada en el cambio pedido.

Salvo que se pida explícitamente, esta etapa no debería intentar resolver todo esto al mismo tiempo:
- limpieza completa de comentarios
- normalización completa de naming en todo el módulo
- rediseño arquitectónico amplio
- validación exhaustiva
- correcciones de estilo no relacionadas

Salida típica:
- el cambio de código pedido
- resumen breve de qué cambió
- nota explícita de qué queda para pasadas posteriores, si aplica

---

### 4. Pasada de comentarios
Usar esta etapa después de implementar o refactorizar, cuando la estructura del código ya está lo suficientemente estable como para documentarla con claridad.

Esta etapa existe porque los comentarios suelen salir mejor cuando el comportamiento y la estructura ya están asentados.

Esta etapa puede incluir:
- agregar comentarios faltantes en la API pública
- remover comentarios redundantes
- corregir comentarios inexactos
- volver más consistente el estilo de comentarios
- documentar restricciones que no son obvias desde el código

Esta etapa no debería usarse como reemplazo de mal naming o mala estructura.

Salida típica:
- comentarios de API pública alineados con el comportamiento real
- comentarios privados solo donde aportan valor técnico real
- eliminación de comentarios viejos, engañosos o inconsistentes

Un documento específico de esta etapa debería definir las reglas detalladas de comentarios.

---

### 5. Validación
Usar esta etapa para verificar que el resultado es aceptable para integrar.

La validación puede incluir:
- verificación de build
- testing dirigido
- chequeos de debug
- sanity checks en runtime
- confirmación de que el comportamiento no cambió inesperadamente
- confirmación de que el cambio pedido quedó realmente completo

Esta etapa debería dejar claro qué fue validado y qué no.

Salida típica:
- qué se validó
- cómo se validó
- qué queda sin validar

La validación no es opcional por principio, pero la profundidad requerida depende del tipo de tarea.

---

## Cómo usar este directorio
Este directorio debería contener documentos cortos y específicos por etapa.

Cada documento de etapa debería responder:
- cuándo usarlo
- qué se puede cambiar en esa etapa
- qué no se debe cambiar
- qué significa que esa etapa está terminada

Los documentos de etapa deberían ser operativos.
Deben ayudar a humanos y agentes a ejecutar una pasada.
No deberían convertirse en manuales generales largos.

---

## Relación con los documentos en raíz

### Los documentos en raíz deberían conservar:
- reglas estables de todo el proyecto
- reglas de estilo de código
- reglas de naming
- reglas de arquitectura de aplicación amplia
- puntos de entrada al repositorio
- contexto duradero que convenga encontrar rápido

Ejemplos:
- `AGENT_ENTRY.md`
- `STYLE.md`
- `AGENTS.md`
- `README.md`

### Los documentos de workflow deberían contener:
- instrucciones específicas por etapa
- alcance y no objetivos de cada etapa
- criterio de cierre específico de la pasada
- ejemplos de cómo pedir o revisar esa pasada

En resumen:
- los documentos en raíz definen reglas duraderas del proyecto
- los documentos de workflow definen cuándo y cómo se aplica una etapa

---

## Enfoque de migración
Este repositorio no necesita una reescritura completa de documentación antes de empezar a usar un workflow por etapas.

La migración debería ser incremental.

Orden recomendado:
1. Definir este modelo de etapas
2. Agregar un documento de etapa con valor inmediato
3. Aplicarlo sobre un módulo real
4. Ajustarlo según uso real
5. Recién después extraer o reescribir el resto del material de workflow

El primer documento de etapa recomendado es:
- `comment_pass.md`

Motivos:
- la consistencia de comentarios hoy es despareja en el repo
- los comentarios suelen resolverse mejor una vez estabilizada la implementación
- esta etapa se puede aplicar con bajo riesgo
- esta etapa se puede revisar con claridad

---

## Regla práctica
No asumir que una primera pasada tiene que resolver todo.

Cuando haga falta, pedir explícitamente la siguiente pasada.

Ejemplos:
- "analizá primero este módulo"
- "analizá y proponé un plan"
- "hacé solo la implementación"
- "hacé una pasada de comentarios sobre estos dos archivos"
- "hacé una pasada de validación sobre este cambio"

Esto debería preferirse por sobre mezclar múltiples objetivos en un único pedido vago.

---

## Convención inicial
Hasta que existan más documentos de etapa, usar estos significados por defecto:

- análisis: entender el código actual y sus restricciones locales
- plan: definir el cambio buscado y la secuencia recomendada
- implementación: cambiar código
- pasada de comentarios: mejorar comentarios una vez estabilizado el código
- validación: verificar que el cambio está en condiciones de integrarse

Los futuros documentos de etapa pueden refinar estos significados, pero no deberían contradecirlos sin una buena razón.
