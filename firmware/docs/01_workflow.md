# 01_workflow.md — Guía rápida para trabajo operativo con IA
Versión: v1.0

## Propósito
Usar ChatGPT y Codex de forma consistente durante refactors incrementales: acotar tareas, mantener control técnico y respetar referencias del repo.

## Regla principal
Cada interacción con IA debe atacar **una micro-tarea pequeña, concreta y verificable**.

Antes de abrir un agente, responder:
1. ¿Qué quiero resolver?
2. ¿Dónde debería vivir la responsabilidad?
3. ¿Qué referencias del repo aplican?
4. ¿Qué salida verificable espero?

## Estructura objetivo (separación de responsabilidades)
- `app/` — comportamiento de aplicación y casos de uso
- `bsp/` — detalles de placa y mapping
- `cube/` — código generado / condicionable por CubeMX
- `port/` — adaptación a plataforma/toolchain
- `services/` — lógica reusable y módulos intermedios

## Flujo mínimo por micro-tarea
1. **Delimitar**: objetivo, tipo, carpeta probable, archivos implicados, salida esperable.
2. **Identificar referencias**: `STYLE.md`, `CHANGELOG.md`, `AGENTS.md`, `style-examples/`.
3. **Pensar con ChatGPT**: scope, ubicación, conflictos de responsabilidad, preparar prompt para Codex.
4. **Ejecutar con Codex**: inspección, propuesta localizada, primer paso implementado.
5. **Revisar**: verificación contra `STYLE.md`, ejemplos, `CHANGELOG.md`, `AGENTS.md`.
6. **Cerrar contexto**: qué se resolvió, dónde quedó la responsabilidad, referencias clave, siguiente micro-tarea.

## Golden operational rule
Antes de aceptar una propuesta, responde explícitamente:
- ¿Qué responsabilidad se modifica?
- ¿En qué carpeta debe vivir?
- ¿Qué regla de `STYLE.md` aplica?
- ¿Qué ejemplo de `style-examples/` se usó como referencia?
- ¿Qué contexto de `CHANGELOG.md` se preservó?
- ¿Qué expectativa de `AGENTS.md` aplicó?

## Quick checklist para revisar cambios de Codex
- ¿Vive en la carpeta correcta?
- ¿No mezcla lógica de aplicación con hardware o código generado?
- ¿Respeta `STYLE.md` y naming?
- ¿La implementación coincide con un ejemplo de `style-examples/` en forma?
- ¿No contradice `CHANGELOG.md` o `AGENTS.md`?
- ¿El alcance fue el mínimo necesario?