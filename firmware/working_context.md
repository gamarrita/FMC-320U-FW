# WORKING_CONTEXT.md

## Propósito

Contexto de trabajo vivo para tareas actuales de firmware.
Preserva continuidad de sesión.
Mutable; no es fuente de reglas estables.

## Foco Actual

Área activa:
- `bsp/devices/lcd/`
- `apps/lcd_ll_bringup/`

Objetivo vigente:
- Consolidar el camino LCD nuevo sin dependencias legacy.
- Mantener `fm_lcd.*` como helper de nivel más alto para uso de producto.
- Cerrar validaciones pendientes del mapping documentado y limpiar remanentes viejos.

Archivos hoy relevantes:
- `bsp/devices/lcd/fm_lcd_ll.h`
- `bsp/devices/lcd/fm_lcd_ll.c`
- `bsp/devices/lcd/fm_lcd.h`
- `bsp/devices/lcd/fm_lcd.c`
- `apps/lcd_ll_bringup/fm_lcd_ll_bringup.c`
- `bsp/devices/lcd/docs/lcd_true_source.yaml`
- `bsp/devices/lcd/pcf8553/fm_pcf8553.h`
- `bsp/devices/lcd/pcf8553/fm_pcf8553.c`

Nota de build:
- El `CMakeLists.txt` principal compila hoy `fm_lcd.c`, `fm_lcd_ll.c` y `pcf8553/fm_pcf8553.c`.
- El cierre del refactor ya no depende de archivos legacy dentro del árbol.
- Los archivos `fm_lcd_legacy.*` y `fm_lcd_ll_legacy.*` ya fueron retirados del repositorio tras validar `ACM_1`.

---

## Estado Actual

`fm_lcd_ll.c/h` ya cubre el camino principal nuevo para:
- init, clear, fill y refresh del controlador
- escritura de caracteres de 7 segmentos en fila 1 y fila 2
- escritura de puntos decimales por fila y columna
- escritura de la mayoría de los símbolos independientes
- tamaño de filas (`8` arriba, `7` abajo)
- escritura de los dos caracteres alfanuméricos de 14 segmentos mediante `FM_LCD_LL_AlphaPutChar()`

`fm_lcd.c/h` ya está alineado con el camino nuevo:
- envuelve a `fm_lcd_ll.*`
- expone helpers de nivel más alto para strings, enteros, decimal points y símbolos
- ya no depende de remanentes legacy

`apps/lcd_ll_bringup/fm_lcd_ll_bringup.c` ya valida visualmente:
- clear
- fila 1 completa con `8`
- puntos decimales de fila 1
- fila 2 completa con `8`
- puntos decimales de fila 2
- símbolos confirmados
- `ACM_1`
- patrón combinado con todos los elementos confirmados
- patrones alpha `A`, `H`, `N`, `M`, `X`, `2` y `8`

La migración del soporte alfanumérico de 14 segmentos debe considerarse funcionalmente avanzada:
- el mapping base de ambos dígitos ya está en `fm_lcd_ll.c`
- las tablas de encoding por caracter ya existen en el driver nuevo
- el bring-up ya consume esa API nueva y no el camino legacy para esos dígitos

El documento `lcd_true_source.yaml` sigue siendo la fuente técnica de referencia para mapping y gaps conocidos.

---

## Pendientes Reales

`ACM_1` ya quedó cerrado en el camino nuevo:
- `FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_ACM_1, ...)` escribe `reg: 5`, `bit: 3`
- el dato quedó incorporado a `lcd_true_source.yaml`
- la validación visual en hardware fue confirmada con `lcd_ll_bringup`

Blink queda explícitamente fuera del refactor actual:
- el comportamiento legacy de blink no se migra a `fm_lcd_ll.*`
- si el producto vuelve a necesitar blink, conviene rediseñarlo desde cero con ownership claro en una pasada futura
- la API pública `FM_PCF8553_Blink()` del camino nuevo puede considerarse código muerto si no reaparece un consumidor real

Las APIs legacy `FM_LCD_LL_PutChar_1()` y `FM_LCD_LL_PutChar_2()` no existen en el camino nuevo:
- no son necesarias para el build activo porque el nuevo driver usa `FM_LCD_LL_AlphaPutChar()` y el bring-up ya prueba ese camino
- ya no son criterio para retener archivos legacy en el repo

El naming geométrico fino de los 14 segmentos sigue deliberadamente encapsulado dentro de `fm_lcd_ll.c`.
La API pública nueva evita exponer nombres A..N del vidrio mientras esa nomenclatura no quede cerrada con evidencia de hardware.

No hay todavía una pasada de validación que confirme:
- exactitud física final de todos los patrones alpha fuera del set ya usado en bring-up

---

## Restricciones Actuales

No mezclar en el mismo patch:
- resolución de mapping faltante
- rediseño amplio de API
- limpieza general del módulo

Mantener ownership claro:
- `fm_lcd_ll.*` para mapping del glass y acceso elemento por elemento
- `fm_lcd.*` para helpers LCD de más alto nivel
- `apps/lcd_ll_bringup/*` para validación runtime por etapas

Seguir usando `lcd_true_source.yaml` como source of truth para mapping y trazabilidad; no volver a inferir el layout desde el código legacy salvo como apoyo puntual.

---

## Próximo Paso Recomendado

Hacer la próxima pasada chica enfocada en limpieza post-migración:
- eliminar remanentes legacy del árbol
- correr build y grep de referencias para confirmar que el camino nuevo quedó autosuficiente

---

## Notas para la Próxima Sesión

No volver a tratar la migración alpha como pendiente principal: ya hay implementación nueva y bring-up dedicado.
La discrepancia principal ya no es `ACM_1`, sino cuánto endurecer la validación alpha y cuánta limpieza adicional conviene hacer alrededor del módulo.

Si se hace una próxima pasada de análisis, concentrarla en:
- validación física adicional de patrones alpha si hace falta
- limpieza residual del módulo LCD que no cambie comportamiento
