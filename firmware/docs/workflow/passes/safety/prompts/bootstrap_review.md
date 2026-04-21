Trabaja en `firmware/`.

Tipo de prompt:
- support prompt
- bootstrap de revisión safety dentro del contexto existente
- no crea ni reemplaza `WORKING_CONTEXT.md`
- no convierte `docs/workflow/passes/safety/` en una pasada bootstrap formal

Usá como referencia:
- `firmware/WORKING_CONTEXT.md`
- el archivo de contexto activo referenciado por `firmware/WORKING_CONTEXT.md`, si aplica
- `firmware/docs/workflow/README.md`
- `firmware/docs/workflow/passes/README.md`
- `firmware/docs/workflow/passes/safety/README.md`
- `firmware/docs/workflow/passes/safety/CONTRACT.md`
- `firmware/STYLE.md` solo cuando naming o estructura importen para el análisis

Tarea:
Iniciá una revisión safety-like sobre <scope objetivo> sin expandir innecesariamente el alcance.

Objetivo:
determinar el riesgo visible, los límites de validación y el hardening mínimo
revisable que tendría sentido dentro del contexto actual.

Restricciones:
- no crear ni reencuadrar `WORKING_CONTEXT.md`
- no convertir esta revisión en una pasada de rediseño general
- no mezclar naming o style cleanup salvo cuando impacten un riesgo real
- mantener ownership y límites explícitos

Respondé usando exactamente esta estructura:

Scope real:
- ...

Riesgos safety-like visibles:
- ...

Límites de validación:
- ...

Archivos candidatos:
- ...

Hardening mínimo revisable:
- ...

Fuera de alcance:
- ...
