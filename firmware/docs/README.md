# Firmware docs — QuickStart

**Propósito**  
Documentación operativa para refactor de firmware con apoyo de agentes (ChatGPT / Codex). Este directorio agrupa la guía de trabajo, plantillas y el contexto activo.

**Orden de lectura (obligatorio)**
1. `01_workflow.md` — Guía rápida y checklist.
2. `02_chatgpt.md` — Plantillas para pensamiento y scoping.
3. `03_codex.md` — Plantillas para ejecutar cambios y diffs.
4. `04_context.md` — Estado vivo del trabajo.
5. `agent_prompt.md` — Entrada canónica para agentes (usar siempre cuando se delega ejecución).

**Referencias obligatorias (leer antes de cambiar código)**
- `../STYLE.md` — reglas de naming, estructura y seguridad.
- `../CHANGELOG.md` — continuidad técnica.
- `../AGENTS.md` — reglas operativas para delegación.
- `../style-examples/` — ejemplos mínimos de forma (NO arquitectura).

**Checklist rápida antes de aceptar un cambio**
- ¿La responsabilidad vive en `app/`, `bsp/`, `cube/`, `port/` o `services/`? (justificar)
- ¿Respeta `STYLE.md` (naming, visibilidad, límites de ISR/ISR wrappers)?
- ¿El cambio es una micro-tarea acotada y verificable?
- ¿Se revisó `CHANGELOG.md` por continuidad?
- ¿Se siguió `agent_prompt.md` si la tarea fue delegada a un agente?

**Notas**
- Mantener nombres `01_...` → `04_...` para preservar orden de lectura.
- `AGENTS.md` queda como política; `agent_prompt.md` es el condicionador reutilizable.