# 01_quickstart.md

[Ir a la versión en español](#version-en-espanol)

## Purpose

This is the quick operational guide for working in `firmware/`.

Read this file first when starting a task.
It is intended for both humans and agents.

This file explains how to start correctly, how to keep tasks small,
what to inspect before changing code, and how to decide whether a result is acceptable.

For detailed workflow, see `02_workflow.md`.
For naming and style, see `STYLE.md`.

---

## Core principle

Every interaction with an AI assistant or code agent should be framed as a small,
concrete, verifiable task.

Good tasks are narrow enough to review easily and specific enough to validate quickly.

Examples of good task scope:

- add one missing init function
- split one mixed-responsibility module
- rename one public API to match naming rules
- create one service module following an existing example
- review whether one file belongs in `app/` or `services/`

Examples of bad task scope:

- rewrite the whole architecture
- clean up the entire codebase
- redesign all naming
- refactor everything related to initialization
- reorganize all folders in one pass

---

## Start sequence

Before asking an agent to generate or modify code:

1. define the exact task
2. identify the expected owning folder
3. identify the affected module or file
4. read `STYLE.md`
5. inspect the closest matching example in `style-examples/`
6. confirm whether the task is additive, corrective, or refactor-only
7. define what "done" means

If any of these are unclear, reduce the scope before proceeding.

---

## How to define a good task

A good task should answer:

- What needs to change?
- Where should it change?
- Why does that file or module own the change?
- What existing pattern should be followed?
- What should remain untouched?

Recommended task statement format:

> Update `<target file or module>` to `<specific change>`, following `STYLE.md`
> and the closest relevant file under `style-examples/`.
> Keep the change minimal and do not expand scope beyond `<boundary>`.

---

## How to choose the owning folder

Use existing repository responsibility boundaries.

Typical rule of thumb:

- choose `app/` for orchestration and product behavior
- choose `bsp/` for board-level integration
- choose `cube/` only when aligned with vendor-generated or low-level MCU startup or config
- choose `port/` for adaptation boundaries
- choose `services/` for reusable internal modules not owned by `app/` or hardware specifics

Do not place code based only on convenience.
Place code where future maintenance will be most obvious.

---

## What to inspect before editing

Before generating changes, inspect:

- the target file itself
- neighboring files in the same module
- the naming rules in `STYLE.md`
- an existing example with similar public and private split
- whether the change introduces a new responsibility into a file that already has another purpose

If a file already mixes concerns, do not expand that mistake automatically.
Call it out and keep the change as contained as possible.

---

## What to use from `style-examples/`

Use examples to learn:

- file layout
- function naming
- public versus private symbol split
- include ordering
- internal helper placement
- callback naming
- wrapper naming
- minimal module structure

Do not use examples as final architecture templates.
They are style and module-form references, not product design templates.

---

## What to avoid

Avoid these common failures:

- creating new naming styles
- mixing public API and internal orchestration without structure
- exposing helpers that should stay private
- creating folders or layers without repository precedent
- adding abstractions "for later"
- broad cleanup inside a task that was supposed to be narrow
- using examples mechanically without checking current repository structure

---

## Definition of done

A task is in good shape when:

- the target folder is justified
- the module responsibility is clear
- naming matches `STYLE.md`
- public and private split is consistent
- no extra architectural change was introduced unintentionally
- the patch is easy to review
- the result matches an existing repository pattern where possible

---

## Minimal completion checklist

Before finalizing:

- [ ] task is concrete and bounded
- [ ] target folder is correct
- [ ] naming was checked
- [ ] file split is coherent
- [ ] closest example was reviewed
- [ ] no unnecessary scope expansion happened
- [ ] result is reviewable in isolation

---

## Related documents

- Full process: `02_workflow.md`
- Style and naming: `STYLE.md`
- Agent behavior: `AGENTS.md`
- Examples index: `style-examples/README.md`
- Live continuity: `05_working_context.md`

---

## Version en espanol

## Proposito

Esta es la guia operativa rapida para trabajar en `firmware/`.

Lee este archivo primero al comenzar una tarea.
Esta pensada tanto para humanos como para agentes.

Este archivo explica como empezar correctamente, como mantener las tareas pequenas,
que inspeccionar antes de cambiar codigo, y como decidir si un resultado es aceptable.

Para el workflow detallado, consulta `02_workflow.md`.
Para naming y estilo, consulta `STYLE.md`.

---

## Principio central

Toda interaccion con un asistente de IA o un agente de codigo deberia plantearse como una tarea pequena,
concreta y verificable.

Las buenas tareas son lo bastante acotadas como para revisarse facilmente y lo bastante especificas como para validarse rapido.

Ejemplos de buen alcance de tarea:

- agregar una unica funcion de inicializacion faltante
- dividir un unico modulo con responsabilidades mezcladas
- renombrar una unica API publica para que coincida con las reglas de naming
- crear un unico modulo de servicio siguiendo un ejemplo existente
- revisar si un archivo pertenece a `app/` o a `services/`

Ejemplos de mal alcance de tarea:

- reescribir toda la arquitectura
- limpiar toda la base de codigo
- redisenar todo el naming
- refactorizar todo lo relacionado con la inicializacion
- reorganizar todas las carpetas en una sola pasada

---

## Secuencia de inicio

Antes de pedirle a un agente que genere o modifique codigo:

1. define la tarea exacta
2. identifica la carpeta que deberia ser la dueña del cambio
3. identifica el modulo o archivo afectado
4. lee `STYLE.md`
5. inspecciona el ejemplo mas cercano en `style-examples/`
6. confirma si la tarea es aditiva, correctiva o solo refactor
7. define que significa "terminado"

Si alguno de estos puntos no esta claro, reduce el alcance antes de continuar.

---

## Como definir una buena tarea

Una buena tarea deberia responder:

- Que necesita cambiar?
- Donde deberia cambiar?
- Por que ese archivo o modulo es el dueño del cambio?
- Que patron existente deberia seguirse?
- Que deberia permanecer intacto?

Formato recomendado para describir una tarea:

> Actualiza `<archivo o modulo objetivo>` para `<cambio especifico>`, siguiendo `STYLE.md`
> y el archivo mas cercano bajo `style-examples/`.
> Manten el cambio minimo y no expandas el alcance mas alla de `<limite>`.

---

## Como elegir la carpeta duena

Usa los limites de responsabilidad ya existentes en el repositorio.

Regla practica tipica:

- elige `app/` para orquestacion y comportamiento de producto
- elige `bsp/` para integracion a nivel placa
- elige `cube/` solo cuando este alineado con codigo generado por vendor o con startup/configuracion MCU de bajo nivel
- elige `port/` para capas de adaptacion
- elige `services/` para modulos internos reutilizables que no pertenezcan a `app/` ni a hardware especifico

No ubiques codigo solo por conveniencia.
Ubicalo donde el mantenimiento futuro resulte mas obvio.

---

## Que inspeccionar antes de editar

Antes de generar cambios, inspecciona:

- el archivo objetivo
- los archivos vecinos del mismo modulo
- las reglas de naming en `STYLE.md`
- un ejemplo existente con una division similar entre publico y privado
- si el cambio introduce una responsabilidad nueva en un archivo que ya tiene otro proposito

Si un archivo ya mezcla responsabilidades, no expandas ese error automaticamente.
Senalalo y manten el cambio lo mas contenido posible.

---

## Que tomar de `style-examples/`

Usa los ejemplos para aprender:

- organizacion del archivo
- naming de funciones
- separacion entre simbolos publicos y privados
- orden de includes
- ubicacion de helpers internos
- naming de callbacks
- naming de wrappers
- estructura minima de un modulo

No uses los ejemplos como plantillas finales de arquitectura.
Son referencias de estilo y de forma de modulo, no disenos de producto.

---

## Que evitar

Evita estos errores comunes:

- crear estilos nuevos de naming
- mezclar API publica y orquestacion interna sin estructura
- exponer helpers que deberian seguir siendo privados
- crear carpetas o capas sin precedente en el repositorio
- agregar abstracciones "para mas adelante"
- hacer limpieza amplia dentro de una tarea que deberia ser acotada
- usar ejemplos mecanicamente sin revisar la estructura actual del repositorio

---

## Definicion de terminado

Una tarea esta en buen estado cuando:

- la carpeta objetivo esta justificada
- la responsabilidad del modulo es clara
- el naming coincide con `STYLE.md`
- la division entre publico y privado es consistente
- no se introdujo sin querer un cambio arquitectonico extra
- el patch es facil de revisar
- el resultado coincide, cuando es posible, con un patron existente del repositorio

---

## Checklist minima de cierre

Antes de finalizar:

- [ ] la tarea es concreta y acotada
- [ ] la carpeta objetivo es correcta
- [ ] el naming fue verificado
- [ ] la division del archivo es coherente
- [ ] se reviso el ejemplo mas cercano
- [ ] no hubo expansion innecesaria del alcance
- [ ] el resultado es revisable de forma aislada

---

## Documentos relacionados

- Proceso completo: `02_workflow.md`
- Estilo y naming: `STYLE.md`
- Comportamiento de agentes: `AGENTS.md`
- Indice de ejemplos: `style-examples/README.md`
- Continuidad viva: `05_working_context.md`
