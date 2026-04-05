# From “It Builds in the IDE” to a Canonical Firmware Build: Enabling Real AI-Assisted Development

[Ir a la versión en español](#version-en-espanol)

## Executive summary (promotional)

Most embedded firmware projects unknowingly depend on an invisible assumption: *“if it builds in the IDE, the system is correct.”*
This assumption breaks immediately when introducing automation, CI, or AI agents.

By making the STM32 build **explicit, reproducible, and canonical outside the IDE**, this project eliminates that hidden dependency and unlocks a new capability:

> The firmware can now be **understood, modified, built, and validated consistently by both humans and AI agents**.

This is not a tooling improvement.
It is a **shift in the development contract** of the firmware.

---

# 1. The real problem: IDE-dependent builds

In STM32 + VS Code environments, the build pipeline is not fully visible:

* `cube`, `cube-cmake`, toolchains, and bundles are resolved implicitly
* environment variables are injected by the extension
* paths are versioned and not exported to the shell

As a result:

* The project builds inside VS Code
* The same project **fails from a clean terminal**
* An AI agent **cannot reproduce the build**

This is not a missing `cmake` issue.
It is a **hidden environment problem**.

---

# 2. Why this matters for AI-assisted development

Without a reproducible build, an agent can only:

* suggest code
* refactor structure
* apply naming rules
* generate plausible implementations

But it **cannot**:

* verify compilation
* distinguish environment vs code failures
* iterate safely
* close the engineering loop

This creates a fundamental limitation:

> The agent operates in a **speculative mode**, not an **engineering mode**.

---

# 3. What was implemented: canonical build + bootstrap

## 3.1 Explicit bootstrap of STM32 environment

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
```

This script:

* discovers STM32 extensions dynamically
* resolves environment via `cube`
* exports toolchain and build variables
* reconstructs `PATH` deterministically

Key property:

> The environment is **discovered, not assumed**.

---

## 3.2 Canonical build entrypoint

```powershell
cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

Selected based on:

* identical generator
* identical toolchain
* identical cache
* identical artifact

---

## 3.3 Agent-aware workflow

```powershell
where.exe cube
echo $env:CUBE_BUNDLE_PATH

.\tools\stm32cube-env.ps1

cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

---

# 4. Impact: from assistance to operational capability

Before:

* No build verification
* No environment introspection
* No reliable iteration

After:

* Deterministic build
* Shared human/agent contract
* Closed engineering loop

> From **AI suggesting code** → to **AI participating in development**

---

# 5. Why the bootstrap is the central piece

It eliminates:

* IDE dependency
* environment ambiguity
* toolchain opacity
* version fragility

Without it:

> The system is non-reproducible → the agent is unreliable

---

# 6. What this does *not* solve

Canonical build is necessary, but not sufficient.

Missing layers:

* functional validation
* runtime verification
* integration confidence

---

# 7. What remains to be solved

This is the critical next phase.

## 7.1 Post-build validation contract

Currently:

> “build succeeded” is the only success criterion

This is insufficient.

Required next step:

* define **canonical post-build checks**, for example:

  * `.elf` presence and size range
  * expected sections / symbols
  * linker map sanity
  * memory usage constraints

Without this:

> The agent can produce **compiling but broken firmware**

---

## 7.2 Minimal runtime / bring-up validation

Build correctness ≠ system correctness.

Missing:

* startup sanity (vector table, reset handler)
* peripheral init expectations
* minimal “system alive” signal (e.g., log, GPIO toggle)

Recommended:

* define a **baseline runtime contract**:

  * what must be observable after boot
  * what defines “firmware alive”

---

## 7.3 Task-level validation patterns

Today tasks are well-scoped, but validation is implicit.

Next step:

* each task type should define:

  * what must compile
  * what must not break
  * what must be verified

Example:

* “new service module” → builds + no new dependencies upward
* “HAL wrapper” → builds + no leakage of HAL types

---

## 7.4 Structural invariants enforcement

The repo defines:

* folder ownership
* layering rules

But enforcement is manual.

Missing:

* lightweight checks:

  * forbidden includes (e.g., `app` including `cube`)
  * dependency direction validation
  * public/private API constraints

Without this:

> The agent may slowly degrade architecture while still passing build

---

## 7.5 CI or automated verification

Current flow is local.

Next logical step:

* minimal CI pipeline that:

  * bootstraps environment
  * runs canonical build
  * runs post-build checks

This would:

* validate reproducibility
* prevent regressions
* make agent output trustable at scale

---

## 7.6 Failure classification clarity

One subtle but important gap:

The system still relies on interpretation when build fails.

Needed:

* clear classification of failures:

  * environment not loaded
  * toolchain mismatch
  * code error
  * configuration error

This improves:

* agent diagnostics
* debugging speed
* prompt quality

---

## 7.7 Reduction of “implicit correctness”

Even with canonical build, some assumptions remain implicit:

* correct preset selection
* correct board configuration
* correct startup code alignment

Future improvement:

* make these explicit in documentation or checks

---

# 8. Key insight

The major achievement is not the script.

It is this:

> The build is now part of the **explicit contract of the repository**

What remains:

> Extending that contract beyond build → into **validation and correctness**

---

# 9. Roadmap summary

Current state:

* reproducible environment
* canonical build
* agent-compatible workflow

Next phase:

1. post-build validation
2. runtime sanity checks
3. structural enforcement
4. CI integration
5. failure classification

---

# 10. Conclusion

This project solved the hardest first problem:

> Making firmware build reproducible outside the IDE

That alone transforms AI usefulness.

But the full transformation requires one more step:

> Turning “it builds” into “it is correct”

When that layer is added:

* the agent will not only build firmware
* it will **validate, diagnose, and evolve it reliably**

That is the real end state.

---

## Version en espanol

# De “Compila en el IDE” a un Build Canonico de Firmware: habilitando desarrollo real asistido por IA

## Resumen ejecutivo (promocional)

Muchos proyectos de firmware embedded dependen sin notarlo de un supuesto invisible: *“si compila en el IDE, el sistema esta correcto.”*
Ese supuesto se rompe de inmediato cuando se introduce automatizacion, CI o agentes de IA.

Al hacer el build STM32 **explicito, reproducible y canonico fuera del IDE**, este proyecto elimina esa dependencia oculta y habilita una nueva capacidad:

> El firmware ahora puede ser **entendido, modificado, compilado y validado de forma consistente tanto por personas como por agentes de IA**.

Esto no es solo una mejora de tooling.
Es un **cambio en el contrato de desarrollo** del firmware.

---

# 1. El problema real: builds dependientes del IDE

En entornos STM32 + VS Code, el pipeline de build no es completamente visible:

* `cube`, `cube-cmake`, toolchains y bundles se resuelven implicitamente
* variables de entorno son inyectadas por la extension
* los paths estan versionados y no se exportan a la shell

Como resultado:

* El proyecto compila dentro de VS Code
* El mismo proyecto **falla desde una terminal limpia**
* Un agente de IA **no puede reproducir el build**

Esto no es un problema de `cmake` faltante.
Es un **problema de entorno oculto**.

---

# 2. Por que esto importa para desarrollo asistido por IA

Sin un build reproducible, un agente solo puede:

* sugerir codigo
* refactorizar estructura
* aplicar reglas de naming
* generar implementaciones plausibles

Pero **no puede**:

* verificar compilacion
* distinguir fallas de entorno vs fallas de codigo
* iterar con seguridad
* cerrar el loop de ingenieria

Eso crea una limitacion fundamental:

> El agente opera en **modo especulativo**, no en **modo ingenieria**.

---

# 3. Que se implemento: build canonico + bootstrap

## 3.1 Bootstrap explicito del entorno STM32

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
```

Este script:

* descubre dinamicamente las extensiones STM32
* resuelve el entorno via `cube`
* exporta variables del toolchain y del build
* reconstruye `PATH` de forma deterministica

Propiedad clave:

> El entorno se **descubre, no se asume**.

---

## 3.2 Entrada canonica del build

```powershell
cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

Seleccionada en base a:

* mismo generator
* mismo toolchain
* mismo cache
* mismo artefacto

---

## 3.3 Flujo consciente de agentes

```powershell
where.exe cube
echo $env:CUBE_BUNDLE_PATH

.\tools\stm32cube-env.ps1

cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

---

# 4. Impacto: de asistencia a capacidad operativa

Antes:

* Sin verificacion de build
* Sin introspeccion de entorno
* Sin iteracion confiable

Despues:

* Build deterministico
* Contrato compartido entre humanos y agentes
* Loop de ingenieria cerrado

> De **IA sugiriendo codigo** a **IA participando del desarrollo**

---

# 5. Por que el bootstrap es la pieza central

Elimina:

* dependencia del IDE
* ambiguedad del entorno
* opacidad del toolchain
* fragilidad por versionado

Sin eso:

> El sistema no es reproducible y el agente no es confiable

---

# 6. Que esto *no* resuelve

El build canonico es necesario, pero no suficiente.

Capas que aun faltan:

* validacion funcional
* verificacion runtime
* confianza de integracion

---

# 7. Que queda por resolver

Esta es la siguiente fase critica.

## 7.1 Contrato de validacion post-build

Hoy:

> “el build termino bien” es el unico criterio de exito

Eso es insuficiente.

Siguiente paso requerido:

* definir **checks post-build canonicos**, por ejemplo:

  * presencia del `.elf` y rango de tamano
  * secciones / simbolos esperados
  * sanidad del linker map
  * restricciones de uso de memoria

Sin eso:

> El agente puede producir **firmware que compila pero esta roto**

---

## 7.2 Validacion runtime minima / de bring-up

Que el build sea correcto no implica que el sistema lo sea.

Falta:

* sanidad de startup (vector table, reset handler)
* expectativas de init de perifericos
* senal minima de “sistema vivo” (por ejemplo log o toggle GPIO)

Recomendado:

* definir un **contrato runtime base**:

  * que debe poder observarse luego del boot
  * que define “firmware vivo”

---

## 7.3 Patrones de validacion por tipo de tarea

Hoy las tareas estan bien acotadas, pero la validacion es implicita.

Siguiente paso:

* cada tipo de tarea deberia definir:

  * que debe compilar
  * que no debe romperse
  * que debe verificarse

Ejemplo:

* “nuevo modulo de servicio” -> compila + no introduce dependencias hacia arriba
* “wrapper HAL” -> compila + no filtra tipos HAL

---

## 7.4 Enforcements de invariantes estructurales

El repo define:

* ownership por carpeta
* reglas de layering

Pero el enforcement hoy es manual.

Falta:

* checks livianos:

  * includes prohibidos (por ejemplo `app` incluyendo `cube`)
  * validacion de direccion de dependencias
  * restricciones de API publica/privada

Sin eso:

> El agente puede degradar lentamente la arquitectura aunque el build siga pasando

---

## 7.5 CI o verificacion automatizada

El flujo actual es local.

Siguiente paso logico:

* pipeline minimo de CI que:

  * bootstrappee el entorno
  * ejecute el build canonico
  * corra checks post-build

Esto:

* valida reproducibilidad
* previene regresiones
* vuelve confiable la salida del agente a escala

---

## 7.6 Claridad en clasificacion de fallas

Hay una brecha sutil pero importante:

El sistema aun depende de interpretacion cuando el build falla.

Hace falta:

* clasificacion clara de fallas:

  * entorno no cargado
  * toolchain incorrecto
  * error de codigo
  * error de configuracion

Esto mejora:

* diagnostico del agente
* velocidad de debug
* calidad de prompts

---

## 7.7 Reduccion de “correccion implicita”

Aun con build canonico, algunas cosas siguen siendo implicitas:

* seleccion correcta del preset
* configuracion correcta de board
* alineacion correcta del startup code

Mejora futura:

* volver eso explicito en documentacion o checks

---

# 8. Insight clave

El mayor logro no es el script.

Es esto:

> El build ahora forma parte del **contrato explicito del repositorio**

Lo que falta:

> Extender ese contrato mas alla del build, hacia **validacion y correccion**

---

# 9. Resumen del roadmap

Estado actual:

* entorno reproducible
* build canonico
* flujo compatible con agentes

Siguiente fase:

1. validacion post-build
2. checks runtime basicos
3. enforcement estructural
4. integracion con CI
5. clasificacion de fallas

---

# 10. Conclusion

Este proyecto resolvio el problema inicial mas dificil:

> Hacer que el firmware compile de forma reproducible fuera del IDE

Eso por si solo transforma la utilidad de la IA.

Pero la transformacion completa requiere un paso mas:

> Convertir “compila” en “es correcto”

Cuando esa capa exista:

* el agente no solo compilara firmware
* lo **validara, diagnosticara y evolucionara de forma confiable**

Ese es el estado final real.
