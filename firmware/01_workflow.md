# Workflow operativo con IA para refactor de firmware embebido
Versión: v0.5

## Propósito
Usar ChatGPT y Codex de forma consistente durante el refactor del firmware, manteniendo foco, contexto y control técnico, con baja fricción y respetando las referencias del repositorio.

Este workflow no reemplaza criterio técnico ni revisión de código.
Su objetivo es mejorar la calidad de las interacciones con IA para:
- analizar código,
- proponer cambios,
- revisar trabajos de Codex,
- mantener coherencia entre sesiones,
- reducir refactors mal delimitados.

---

## Orden de uso
Este conjunto de documentos se usa en este orden:

1. `01_workflow.md`
   - guía general de trabajo
2. `02_chatgpt.md`
   - prompts y patrones para ChatGPT
3. `03_codex.md`
   - prompts y patrones para Codex
4. `04_context.md`
   - estado vivo del trabajo actual

---

## Referencias del repo

### `STYLE.md`
Documento normativo principal para:
- convenciones de estilo,
- organización de módulos,
- naming,
- forma de archivos,
- expectativas de consistencia del repo.

Toda propuesta de implementación o refactor debe contrastarse con `STYLE.md`.

### `CHANGELOG.md`
Referencia de continuidad técnica.
Debe usarse para:
- entender dirección reciente del repo,
- revisar decisiones o cambios ya incorporados,
- evitar contradicciones con el camino de refactor ya tomado.

### `AGENTS.md`
Referencia operativa para trabajo con agentes.
Debe usarse cuando la tarea implique interacción repetida, delegación de trabajo o expectativas de comportamiento del agente.

### `style-examples/`
Referencia práctica de ejemplos mínimos de estilo.

No debe tomarse como arquitectura final del producto.
Debe usarse como referencia de:
- estructura de archivos,
- naming,
- separación API pública / privados,
- simplicidad de implementación,
- ejemplos mínimos por contexto baremetal / RTOS / ISR / drivers.

Ejemplos disponibles:
- `style-examples/baremetal/fm_main_simple.c/.h`
- `style-examples/baremetal/fm_gpio_poll.c/.h`
- `style-examples/rtos/fm_kernel_basic.c/.h`
- `style-examples/rtos/fm_task_simple.c/.h`
- `style-examples/interrupts/fm_exti_flag.c/.h`
- `style-examples/drivers/fm_adc_basic.c/.h`

---

## Regla principal
Cada interacción con IA debe atacar una tarea pequeña, concreta y verificable.

Antes de abrir ChatGPT o Codex, responder:
1. qué quiero resolver,
2. en qué carpeta debería vivir esa responsabilidad,
3. qué referencias del repo aplican,
4. qué salida verificable espero.

---

## La estructura del repo como marco de diseño

La estructura de carpetas dentro de este directorio debe usarse como separador de responsabilidades:

- `port/`
- `bsp/`
- `services/`
- `app/`
- `cube/`

Toda tarea debería intentar responder:
- ¿en qué carpeta debería vivir esta responsabilidad?
- ¿qué carpetas no deberían absorberla?
- ¿qué dependencias son razonables?
- ¿qué ejemplo de `style-examples/` se parece en forma, aunque no necesariamente en función?

---

## Uso correcto de referencias

### Cuándo usar `STYLE.md`
- antes de escribir código nuevo,
- antes de aceptar una propuesta de Codex,
- cuando haya dudas sobre naming, estructura o forma de módulo,
- cuando se proponga crear o renombrar archivos.

### Cuándo usar `CHANGELOG.md`
- cuando una tarea continúe un refactor previo,
- cuando una propuesta toque convenciones o estructura general,
- cuando haya que entender si una dirección ya fue tomada.

### Cuándo usar `AGENTS.md`
- cuando se delegue una tarea larga al agente,
- cuando haya que fijar reglas de comportamiento del agente,
- cuando se quiera alinear expectativas de ejecución.

### Cuándo usar `style-examples/`
- cuando se necesite un ejemplo mínimo de forma,
- cuando Codex necesite una referencia de módulo simple `.c/.h`,
- cuando haya duda sobre separación pública/privada,
- cuando convenga comparar simplicidad de la solución propuesta.

---

## Regla especial sobre naming y forma de archivos
No asumir naming por intuición.

Antes de proponer archivos nuevos o renombrar módulos:
1. revisar `STYLE.md`,
2. contrastar con nombres reales en `style-examples/`,
3. no “corregir” naming de ejemplos por cuenta propia,
4. explicitar cualquier inconsistencia observada en vez de ocultarla.

---

## Flujo mínimo por tarea

### Paso 1. Delimitar la micro-tarea
Definir:
- objetivo concreto,
- tipo de tarea,
- carpeta probable,
- archivos involucrados,
- resultado esperado.

### Paso 2. Identificar referencias del repo
Antes de pedir ayuda a la IA, decidir si hace falta consultar:
- `STYLE.md`
- `CHANGELOG.md`
- `AGENTS.md`
- `style-examples/`

### Paso 3. Usar ChatGPT para pensar
Usar ChatGPT para:
- recortar la tarea,
- decidir en qué carpeta debería vivir,
- detectar conflictos de responsabilidad,
- interpretar cómo usar las referencias del repo,
- preparar un prompt para Codex.

### Paso 4. Usar Codex para inspeccionar o ejecutar
Usar Codex para:
- inspeccionar archivos,
- resumir estado actual,
- proponer cambios localizados,
- implementar un primer paso,
- contrastar propuesta con `STYLE.md`, `CHANGELOG.md`, `AGENTS.md` y `style-examples/`.

### Paso 5. Revisar el resultado
Toda revisión debe comprobar:
- si el cambio vive en la carpeta correcta,
- si respeta `STYLE.md`,
- si usa `style-examples/` como referencia de forma y simplicidad,
- si contradice o ignora contexto relevante de `CHANGELOG.md`,
- si contradice expectativas operativas de `AGENTS.md`.

### Paso 6. Cerrar contexto
Al final dejar claro:
- qué se resolvió,
- en qué carpeta quedó la responsabilidad,
- qué referencia del repo fue clave,
- cuál es la siguiente micro-tarea.

---

## Checklist para revisar cambios de Codex

### Sobre estructura
- ¿La responsabilidad quedó en la carpeta correcta?
- ¿Se mezcló lógica de aplicación con hardware o con código generado?
- ¿Se usaron los ejemplos como referencia de forma y no como arquitectura completa?

### Sobre consistencia
- ¿Respeta `STYLE.md`?
- ¿El naming parece consistente con el repo?
- ¿La forma del módulo se parece a ejemplos mínimos comparables?
- ¿La implementación sigue siendo simple y revisable?

### Sobre continuidad
- ¿Hay algo en `CHANGELOG.md` que debería haberse considerado?
- ¿El cambio sigue la dirección del refactor o la contradice?

### Sobre operación con agentes
- ¿El cambio o la forma de trabajo contradice `AGENTS.md`?
- ¿La tarea fue suficientemente delimitada para un agente?

### Sobre alcance
- ¿Tocó más de lo necesario?
- ¿Rediseñó demasiado para una tarea chica?
- ¿Facilita el siguiente paso?

---

## Regla operativa de oro
Antes de aceptar una propuesta, responder explícitamente:
1. ¿Qué responsabilidad se está tocando?
2. ¿En qué carpeta debería vivir?
3. ¿Qué regla de `STYLE.md` aplica?
4. ¿Qué ejemplo de `style-examples/` se parece al caso?
5. ¿Qué contexto de `CHANGELOG.md` conviene preservar?
6. ¿Qué expectativa de `AGENTS.md` conviene respetar?