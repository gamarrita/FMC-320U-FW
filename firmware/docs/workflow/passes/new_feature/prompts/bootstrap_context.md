Trabaja en `firmware/`.

Tipo de prompt:
- bootstrap
- puede crear o reencuadrar `WORKING_CONTEXT.md`
- debe dejar un contexto activo nuevo o actualizado bajo `docs/contexts/`

Tarea:
Iniciá o reencuadrá una línea de trabajo de nueva funcionalidad sobre
<feature, módulo o flujo objetivo>.

Entradas a completar:
- feature area o caso de uso a entender
- flujo de request o interacción a seguir
- módulos donde hoy viven lógica, transporte, persistencia o UI si aplica
- lugares donde hoy ocurren validación, side effects o cambios de estado

Objetivo:
dejar estado operativo claro para una implementación chica, clara y revisable.

Usá como referencia:
- `firmware/STYLE.md`
- `firmware/docs/workflow/README.md`
- `firmware/docs/workflow/passes/README.md`
- `firmware/docs/workflow/passes/new_feature/README.md`

Restricciones:
- no hacer una explicación larga
- no implementar todavía
- no expandir el feature más allá del pedido real
- no inventar nueva arquitectura sin necesidad
- mantener ownership claro
- mantener naming consistente con `STYLE.md`

Tenés que:
1. identificar objetivo de la feature, puntos de integración y módulos afectados
2. crear o actualizar `WORKING_CONTEXT.md`
3. crear o actualizar un contexto activo bajo `docs/contexts/<slug>.md`

Respondé usando exactamente esta estructura:

WORKING_CONTEXT.md:
- Current Work
- Stage = new_feature
- Active pass = new_feature
- Operational context:
  - Area
  - Core structure
  - Current focus
  - Constraints
- Next Step
- References:
  - `docs/contexts/<slug>.md`
  - `STYLE.md`
- Maintenance Rule:
  - update `WORKING_CONTEXT.md` when execution state changes
  - update `docs/contexts/<slug>.md` when extended context changes
  - update both together when they would otherwise diverge
- Strong Rule

docs/contexts/<slug>.md:
- objetivo de la feature
- punto de integración real
- comportamiento que debe preservarse
- ownership propuesto
- archivos candidatos
- riesgos
- plan mínimo

Resumen corto:
- ...

Do NOT:
- rediseñar el sistema
- referenciar otros passes
- generar arquitectura especulativa
- escribir análisis largo en `WORKING_CONTEXT.md`
- olvidar embebir la regla de mantener alineados `WORKING_CONTEXT.md` y `docs/contexts/<slug>.md`
