Trabaja en `firmware/`.

Tipo de prompt:
- support prompt
- opera sobre el contexto existente
- no crea ni reemplaza `WORKING_CONTEXT.md`

Usá como referencia:
- `firmware/WORKING_CONTEXT.md`
- el archivo activo referenciado por `firmware/WORKING_CONTEXT.md`
- `firmware/docs/workflow/README.md`
- `firmware/docs/workflow/passes/new_feature/README.md`

Tarea:
Hacé una pasada de validación sobre el cambio implementado dentro del track de
nueva funcionalidad.

Objetivo:
verificar que el cambio quedó correcto usando el flujo canónico del repo y,
cuando aplique, validarlo también observando la salida de debug por el Virtual
COM del ST-Link.

Si la validación depende de observabilidad adicional que el cambio actual no
expone, trátalo como un límite explícito de validación y repórtalo sin
convertir esta pasada en una implementación nueva.

Respondé con esta estructura:

Build canónico:
- ...

Flash / ejecución:
- ...

Validación por ST-Link VCP:
- ...

Resultado:
- validado / no validado / validado parcialmente

Problemas encontrados:
- ...

Qué quedó sin validar:
- ...
