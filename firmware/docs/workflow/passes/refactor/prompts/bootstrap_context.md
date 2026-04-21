Trabaja en `firmware/`.

Tipo de prompt:
- bootstrap
- puede crear o reencuadrar `WORKING_CONTEXT.md`
- debe dejar un contexto activo nuevo o actualizado bajo `docs/contexts/`

Tarea:
Iniciá o reencuadrá una tarea de refactor sobre <módulo, carpeta o línea de trabajo>.

Entradas a completar:
- archivos, carpetas o feature area a inspeccionar
- flujo o request path a seguir
- módulos donde hoy viven lógica, transporte, persistencia o UI si aplica
- lugares donde ocurren validación, side effects o cambios de estado

Objetivo:
dejar estado operativo claro para un refactor chico, claro y revisable, sin
expandir el alcance innecesariamente.

Usá como referencia:
- `firmware/STYLE.md`
- `firmware/docs/workflow/README.md`
- `firmware/docs/workflow/passes/README.md`
- `firmware/docs/workflow/passes/refactor/README.md`

Restricciones:
- no hacer una explicación larga
- no modificar código todavía
- no proponer cambios de comportamiento salvo inconsistencia real
- no inventar nueva arquitectura
- mantener ownership claro
- mantener naming consistente con `STYLE.md`

Tenés que:
1. identificar módulos involucrados, comportamiento actual y intención estructural del refactor
2. crear o actualizar `WORKING_CONTEXT.md`
3. crear o actualizar un contexto activo bajo `docs/contexts/<slug>.md`

Respondé usando exactamente esta estructura:

WORKING_CONTEXT.md:
- Current Work
- Stage = refactor
- Active pass = refactor
- Operational context:
  - Area
  - Core structure
  - Current focus
  - Constraints
- Next Step
- References:
  - `docs/contexts/<slug>.md`
  - `STYLE.md`
- Strong Rule

docs/contexts/<slug>.md:
- propósito del refactor
- comportamiento actual
- problema estructural real
- límites de alcance
- archivos candidatos
- riesgos
- plan mínimo

Resumen corto:
- ...

Do NOT:
- escribir análisis largo en `WORKING_CONTEXT.md`
- copiar completo el archivo de contexto extendido
- referenciar otros passes
