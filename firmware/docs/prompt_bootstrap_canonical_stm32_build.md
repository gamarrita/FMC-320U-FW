# Technical Prompt to Bootstrap a Canonical STM32 Build

[Go to Spanish version](#version-en-espanol)

Use this prompt in a new STM32 project when you want Codex to leave a reproducible build flow for both humans and agents, aligned as closely as possible with the STM32 tooling used by VS Code.

Optional background, only as examples of the expected outcome:
- `docs/stm32cube-build-env.md`
- `docs/build_canonical_new_projects.md`

Do not treat those documents as mandatory dependencies of the new project. Use them only as references if they exist or if they were intentionally copied.

## Prompt

You are Codex working on an STM32 project that must end up with a reproducible build environment outside the IDE, using the same tooling resolved by the STM32 VS Code extension or the closest mechanism that the workspace actually uses.

Your goal is not just to make "some cmake" compile. Your goal is to make the project's canonical build explicit and verifiable, usable by:
- a human from the terminal
- an agent from the terminal
- the same VS Code workspace without degrading its current behavior

## Expected result

You must leave resolved, with evidence:
- what the canonical build entrypoint is in this project
- how to bootstrap the environment outside the IDE
- how a human builds it
- how an agent builds it
- which part of the environment actually depends on STM32 rather than only on `PATH`

## Restrictions

- Do not assume from the start that `cube-cmake`, `cube cmake`, or direct `cmake` are equivalent.
- Do not assume from the start that `cube ...` is canonical.
- Do not assume that the problem is solved by adding `cmake.exe` to `PATH`.
- Do not modify `.vscode/settings.json`, presets, clangd, or existing scripts in the first pass unless you can prove it is strictly necessary for real reproducibility.
- Do not stop at "it configures"; validate the build too whenever feasible.
- Keep the change minimal and explicit.

## Phase 1. Mandatory discovery

Before proposing changes:

1. Inspect the current build contract:
   - `CMakePresets.json`
   - `CMakeLists.txt`
   - `.vscode/settings.json`
   - existing scripts in the repo
   - relevant documentation and workflows

2. Reproduce the failure from a clean shell when possible, with concrete evidence. At minimum verify:
   - `where.exe cube`
   - `where.exe cube-cmake`
   - `where.exe cmake`
   - `where.exe ninja`
   - `where.exe arm-none-eabi-gcc`
   - `echo $env:CUBE_BUNDLE_PATH` in PowerShell
   - `cmake --preset <preset>` using the real preset of the project

3. Discover the real STM32 environment using `cube` when it is accessible. At minimum verify:
   - `cube --get-current-value cube_bundle_path`
   - `cube --get-current-value cmsis_pack_root`
   - `cube --resolve cmake`
   - if it applies, `cube --resolve starm-clangd`

4. Explain with evidence what actually depends on the STM32 environment:
   - effective location of `cube.exe`
   - effective location of `cube-cmake.exe`
   - `CUBE_BUNDLE_PATH`
   - `CMSIS_PACK_ROOT`
   - `cmake`
   - `ninja`
   - ARM toolchain
   - `starm-clangd` if it participates in the real workspace contract

## Phase 2. Decide canonicality

You must compare explicitly, using the real preset of the project, these three possibilities:

1. `cube-cmake --preset ...` and `cube-cmake --build --preset ...`
2. `cube cmake --preset ...` and `cube cmake --build --preset ...`
3. `cmake --preset ...` and `cmake --build --preset ...` after loading the bootstrapped environment

Do not make the decision by intuition. You must justify which one becomes the canonical entrypoint using observable evidence.

## Minimum equivalence criteria

Treat two entrypoints as equivalent only if you verify, at minimum:
- the same effective preset
- the same generator
- the same toolchain file
- the same `CMAKE_COMMAND`
- the same `CMAKE_MAKE_PROGRAM`
- the same effective C compiler
- the same effective CXX compiler if applicable
- a comparable final artifact, or at least the same effective build phase if the project has a problem independent of the environment

If configure and build diverge, separate them clearly.

## Phase 3. Minimum expected implementation

If the project does not yet have reproducible bootstrap outside the IDE, implement the minimum needed to leave it ready. Minimum deliverables:

- PowerShell bootstrap
- Git Bash bootstrap if justified by the project
- short operational documentation
- explanatory documentation if needed to understand the problem
- explicit usage criteria for humans and agents

Bootstrap scripts must:
- discover STM32 extensions without hardcoding exact versions
- export the required STM32 environment variables
- add only the necessary entries to `PATH`
- fail with a clear diagnostic if they cannot resolve the expected tooling

## What must be documented

The final documentation must cover, with real commands:
- how to reproduce the failure from a clean shell
- how to discover the real absolute paths
- how to load the repo bootstrap
- how a human builds the project
- how an agent should build the project
- how to repeat the same pattern in new STM32 projects

The documentation must distinguish between:
- stable behavior of the mechanism
- local examples of paths and versions that may change

## What you must not do

- Do not invent a parallel flow if the repo already has useful scripts or partial contracts.
- Do not declare an entrypoint canonical only because "it worked once".
- Do not mix cosmetic improvements with the reproducible-build solution.
- Do not treat configure as sufficient; validate build whenever feasible.
- Do not hide ambiguities between `cube-cmake` and `cube cmake`; compare them.

## Expected final response format

Your final response must include:

1. Summary of the real problem found.
2. What changes you implemented.
3. What evidence you used to decide the canonical entrypoint.
4. How a human should build the project.
5. How an agent should build the project.
6. What limits or risks remain.
7. What optional improvements you would leave for a second pass.

## Completion criterion

Do not consider the job done unless you can state, with evidence:
- that a documented and repeatable way exists to bootstrap the environment
- that the project has a justified canonical build entrypoint
- that a human and an agent can follow that flow without depending on implicit IDE state
- that you did not confuse "some possible build" with "the same build STM32 actually resolves"

---

## Version en espanol

# Prompt técnico para bootstrappear un build STM32 canónico

Usa este prompt en un proyecto STM32 nuevo cuando quieras que Codex deje un flujo de build reproducible por humanos y agentes, alineado lo mejor posible con el tooling STM32 de VS Code.

Antecedentes opcionales, solo como ejemplos del resultado esperado:
- `docs/stm32cube-build-env.md`
- `docs/build_canonical_new_projects.md`

No tomes esos documentos como dependencias obligatorias del nuevo proyecto. Úsalos solo como referencia si existen o si fueron copiados intencionalmente.

## Prompt

Eres Codex trabajando sobre un proyecto STM32 que debe quedar con un entorno de build reproducible fuera del IDE, usando el mismo tooling que resuelve el complemento STM32 de VS Code o el mecanismo más fiel que realmente use este workspace.

Tu objetivo no es solamente hacer que "algún cmake" compile. Tu objetivo es dejar explícito y verificable un build canónico del proyecto, usable por:
- una persona desde terminal
- un agente desde terminal
- el mismo workspace de VS Code sin degradar su comportamiento actual

## Resultado esperado

Debes dejar resuelto, con evidencia:
- cuál es la entrada canónica del build en este proyecto
- cómo bootstrappear el entorno fuera del IDE
- cómo compila un humano
- cómo compila un agente
- qué parte del entorno depende realmente de STM32 y no solo del `PATH`

## Restricciones

- No asumas de entrada que `cube-cmake`, `cube cmake` o `cmake` directo son equivalentes.
- No asumas de entrada que `cube ...` es canónico.
- No asumas que el problema se resuelve agregando `cmake.exe` al `PATH`.
- No modifiques `.vscode/settings.json`, presets, clangd ni scripts existentes en la primera pasada salvo que demuestres que es estrictamente necesario para reproducibilidad real.
- No cierres el trabajo en "configura"; debes validar también el build cuando sea factible.
- Mantén el cambio mínimo y explícito.

## Fase 1. Descubrimiento obligatorio

Antes de proponer cambios:

1. Inspecciona el contrato actual del build:
   - `CMakePresets.json`
   - `CMakeLists.txt`
   - `.vscode/settings.json`
   - scripts existentes en el repo
   - documentación y workflows relevantes

2. Reproduce el fallo desde una shell limpia cuando sea posible, con evidencia concreta. Como mínimo verifica:
   - `where.exe cube`
   - `where.exe cube-cmake`
   - `where.exe cmake`
   - `where.exe ninja`
   - `where.exe arm-none-eabi-gcc`
   - `echo $env:CUBE_BUNDLE_PATH` en PowerShell
   - `cmake --preset <preset>` usando el preset real del proyecto

3. Descubre el entorno STM32 real usando `cube` cuando sea accesible. Como mínimo verifica:
   - `cube --get-current-value cube_bundle_path`
   - `cube --get-current-value cmsis_pack_root`
   - `cube --resolve cmake`
   - si aplica al caso, `cube --resolve starm-clangd`

4. Explica con evidencia qué depende realmente del entorno STM32:
   - ubicación efectiva de `cube.exe`
   - ubicación efectiva de `cube-cmake.exe`
   - `CUBE_BUNDLE_PATH`
   - `CMSIS_PACK_ROOT`
   - `cmake`
   - `ninja`
   - toolchain ARM
   - `starm-clangd` si participa del contrato real del workspace

## Fase 2. Decidir la canonicalidad

Debes comparar explícitamente, usando el preset real del proyecto, estas tres posibilidades:

1. `cube-cmake --preset ...` y `cube-cmake --build --preset ...`
2. `cube cmake --preset ...` y `cube cmake --build --preset ...`
3. `cmake --preset ...` y `cmake --build --preset ...` después de cargar el entorno bootstrappeado

No tomes la decisión por intuición. Debes justificar cuál queda como entrada canónica usando evidencia observable.

## Criterio de equivalencia mínimo

Considera equivalentes dos entrypoints solo si verificas, como mínimo:
- mismo preset efectivo
- mismo generator
- mismo toolchain file
- mismo `CMAKE_COMMAND`
- mismo `CMAKE_MAKE_PROGRAM`
- mismo compilador C efectivo
- mismo compilador CXX efectivo si aplica
- artefacto final comparable, o al menos la misma fase efectiva del build si el proyecto tiene un problema independiente del entorno

Si hay diferencias entre configure y build, sepáralas con claridad.

## Fase 3. Implementación mínima esperada

Si el proyecto no tiene bootstrap reproducible fuera del IDE, implementa lo mínimo necesario para dejarlo listo. Como entregables mínimos:

- bootstrap PowerShell
- bootstrap Git Bash si el proyecto lo justifica
- documentación operativa corta
- documentación explicativa si hace falta para entender el problema
- criterio explícito de uso para humano y para agente

Los scripts de bootstrap deben:
- descubrir extensiones STM32 sin hardcodear versiones exactas
- exportar las variables necesarias del entorno STM32
- agregar a `PATH` solo lo necesario
- fallar con diagnóstico claro si no pueden resolver el tooling esperado

## Qué debe quedar documentado

La documentación final debe cubrir, con comandos reales:
- cómo reproducir el fallo en una shell limpia
- cómo descubrir los paths absolutos reales
- cómo cargar el bootstrap del repo
- cómo builda un humano
- cómo debe buildar un agente
- cómo repetir el mismo patrón en proyectos STM32 nuevos

La documentación debe distinguir entre:
- comportamiento estable del mecanismo
- ejemplos locales de paths y versiones que pueden cambiar

## Qué no debes hacer

- No inventes un flujo paralelo si el repo ya tiene scripts o contratos parciales útiles.
- No declares como canónico un entrypoint solo porque "funcionó una vez".
- No mezcles mejoras cosméticas con la solución del build reproducible.
- No tomes como suficiente que configure; valida build cuando sea factible.
- No ocultes ambigüedades entre `cube-cmake` y `cube cmake`; compáralas.

## Formato de salida esperado

Tu respuesta final debe incluir:

1. Resumen del problema real encontrado.
2. Qué cambios implementaste.
3. Qué evidencia usaste para decidir el entrypoint canónico.
4. Cómo debe compilar un humano.
5. Cómo debe compilar un agente.
6. Qué límites o riesgos quedaron.
7. Qué mejoras opcionales dejarías para una segunda pasada.

## Criterio de cierre

Da el trabajo por terminado solo si puedes afirmar, con evidencia:
- que existe una forma documentada y repetible de bootstrappear el entorno
- que el proyecto tiene una entrada canónica de build justificada
- que un humano y un agente pueden seguir ese flujo sin depender del estado implícito del IDE
- que no confundiste "algún build posible" con "el mismo build que resuelve STM32"
