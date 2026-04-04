# agent_prompt.md — Entrada canónica para agentes
Versión: v1.0

## Propósito
Condicionar la ejecución de agentes (Codex o ChatGPT que actúe como ejecutor) sobre reglas de programación, estilo y separación de responsabilidades del repo FMC-320U.

## Lectura obligatoria (antes de actuar)
1. `firmware/STYLE.md`
2. `firmware/CHANGELOG.md`
3. `firmware/AGENTS.md`
4. `firmware/style-examples/`

## Reglas estrictas
- **Micro-scope**: actuar solo en una micro-tarea acotada. Si la petición supera 3 archivos o un módulo grande, pedir reducción de scope.
- **Carpeta y justificación**: antes de crear/mover archivos, justificar en 1–2 líneas por qué la responsabilidad pertenece a la carpeta elegida (`app/`, `bsp/`, `cube/`, `port/`, `services/`).
- **Naming & visibilidad**: respetar `STYLE.md` (p. ej. `FM_MODULE_Action()` para API pública; `static void fm_module_action_()` para privados).
- **No modificar `cube/`** salvo en puntos de integración permitidos.
- **No autocorregir naming**: si hay inconsistencias, informar y proponer alternativas justificadas.
- **Salida verificable**: git patch/diff + checklist de validación + plan de smoke-test.

## Formato de salida requerido
1. **Resumen** (1–3 líneas): qué se hizo y por qué.  
2. **Justificación de carpeta** (1–2 líneas).  
3. **Lista de archivos** creados/modificados/renombrados.  
4. **Patch git** (o PR) aplicable.  
5. **Checklist** (marcado) con:
   - [ ] Vive en la carpeta correcta (justificación)
   - [ ] Respeta `STYLE.md` (enumerar reglas clave aplicadas)
   - [ ] No toca zonas regeneradas `cube/`
   - [ ] Cambios acotados, listos para revisión humana
6. **Comandos de verificación**: build / tests / pasos de smoke-test

## Comportamiento operacional
- Si no puedes justificar la carpeta o el scope, **no procedas**: pide clarificación.
- Si detectas riesgo de regresión (API pública, ISR, seguridad), señalarlo y detener ejecución propuesta.
- Documentar cualquier excepción al estilo con referencia a `STYLE.md`.

## Ejemplo mínimo de prompt para Codex (usar como plantilla)

You are Codex. Task: {una frase muy corta con objetivo}.
Scope: only files under {ruta(s)}.
Before changing, read: firmware/STYLE.md, firmware/CHANGELOG.md, firmware/AGENTS.md, firmware/style-examples/.
Produce:

One-paragraph justification of why the responsibility belongs in {carpeta}.
A git patch implementing the minimal first-step change (only files in scope).
A checklist of validations and a short smoke-test plan.
Do not touch files outside the scope. Reference file: firmware/docs/agent_prompt.md


## Nota final
Siempre devolver la salida en el formato requerido. No completes cambios que no puedas justificar frente a las referencias obligatorias.