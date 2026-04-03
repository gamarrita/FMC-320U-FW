# Working Context

## Objetivo general actual
Refactor incremental del firmware legacy hacia una estructura más clara, mantenible y alineada con la separación de responsabilidades del repo.

## Orden de trabajo
1. leer `01_workflow.md`
2. preparar interacción con `02_chatgpt.md`
3. preparar interacción con `03_codex.md`
4. actualizar `04_context.md`

## Referencias activas del repo
- `STYLE.md`
- `CHANGELOG.md`
- `AGENTS.md`
- `style-examples/`

## Estructura objetivo de firmware
- `app/`: comportamiento de aplicación y casos de uso
- `bsp/`: detalles de placa y hardware concreto
- `cube/`: código generado o condicionado por CubeMX
- `port/`: adaptación a plataforma / MCU / toolchain / entorno
- `services/`: servicios reutilizables y lógica intermedia delimitada

## Restricciones generales
- Evitar reescrituras masivas sin validación.
- No mezclar lógica de aplicación con hardware de placa.
- No meter lógica manual en código regenerable.
- Contrastar cambios con `STYLE.md`.
- Usar `style-examples/` como patrón mínimo de forma y simplicidad.
- Revisar `CHANGELOG.md` si el cambio depende de continuidad técnica.
- Considerar `AGENTS.md` si la tarea será delegada.
- No asumir naming sin verificar.

## Decisiones vigentes
- [decisión 1]
- [decisión 2]
- [decisión 3]

## Módulos bajo análisis
- [ruta / módulo]
- [ruta / módulo]

## Tareas recientes
- [tarea] -> [resultado]
- [tarea] -> [resultado]

## Dudas abiertas
- [duda]
- [duda]

## Siguiente micro-tarea sugerida
- [paso siguiente]

## Notas sobre ubicación de responsabilidades
- [responsabilidad] debería vivir en [carpeta] porque [...]
- [responsabilidad] no debería vivir en [carpeta] porque [...]