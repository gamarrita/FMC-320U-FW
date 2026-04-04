# Codex Agent Prompt

## Purpose
This file is the single entrypoint for Codex execution prompts in this repository.

When preparing a new Codex task, reference only this file plus the task-specific request.

## Mandatory Instruction Chain
Codex must follow this order:
1. Read `AGENTS.md` for persistent repository rules.
2. Read `docs/current_milestone.md` for active scope and done criteria.
3. Read `STYLE.md` before proposing or editing code.
4. Read `CHANGELOG.md` when continuity or release impact may matter.
5. Use `style-examples/` only as minimal shape and naming references.
6. Use `docs/milestone_design.md` only when milestone reasoning or tradeoffs are needed.

## Repository Layout To Respect
- `01_app/`: application behavior and top-level orchestration.
- `bsp/`: board-specific details and hardware composition.
- `port/`: MCU or peripheral adaptation.
- `services/`: reusable services and intermediate logic.
- `libs/`: small shared utilities.
- `Core/`, `Drivers/`, `cmake/stm32cubemx/`: generated or vendor integration. Do not move manual app logic into these areas.

## Execution Rules
- Keep scope narrow and reviewable.
- Inspect code before editing.
- Do not assume naming or module placement without checking real repository usage.
- Justify why the responsibility belongs in the chosen folder.
- Preserve existing behavior unless the task explicitly asks to change it.
- When working near CubeMX-generated code, keep manual logic outside regenerable sections.

## Build Tools And Commands
- Primary build system: CMake presets with Ninja.
- Standard configure: `cmake --preset Debug`
- Standard build: `cmake --build --preset Debug`
- Release build: `cmake --preset Release` and `cmake --build --preset Release`
- Build artifacts are expected under `build/Debug` or `build/Release`.
- If `cmake` is not on `PATH`, use the executable already resolved by the local environment or read `build/Debug/CMakeCache.txt` to locate the STM32Cube bundle binaries.
- If the toolchain is not on `PATH`, expect it under the STM32Cube bundle directory referenced by `CMakeCache.txt`.

## Changelog Rule
Update `CHANGELOG.md` when the accepted change affects:
- runtime behavior,
- public or integration-facing interfaces,
- safety-relevant handling,
- developer-visible build or usage workflow.

Do not add changelog entries for discarded ideas, exploration notes, or prompt-only drafts.

## Expected Response Style
- State what was inspected.
- State the intended change and why it stays in scope.
- State whether the code was built or otherwise verified.
- State whether `CHANGELOG.md` was updated or why it was not needed.

## Minimal Prompt Template
Use this as the human-facing task wrapper:

```text
Usa `docs/agent_prompt.md` como instruccion operativa principal.

Tarea:
[describir objetivo concreto]

Alcance:
[archivos o modulos permitidos]

Restricciones:
[compatibilidad, capas a no tocar, validaciones requeridas]

Resultado verificable esperado:
[build, diff, interfaz, analisis, plan, etc.]
```
