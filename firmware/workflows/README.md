# Workflow Stages - Etapas del workflow

## Propósito
Este directorio define cómo dividir el trabajo en etapas pequeñas y enfocadas.

El objetivo no es forzar que todas las tareas pasen por el mismo proceso completo.
El objetivo es dejar claro qué tipo de pasada se está haciendo en cada momento,
qué se puede cambiar en esa pasada, y qué significa que esa pasada esté "terminada".

Esto es especialmente importante cuando se trabaja con agentes como ChatGPT y Codex:
- una primera pasada suele ser funcionalmente útil, pero no necesariamente alineada con naming, comentarios, limpieza o validación
- pedir todo junto normalmente reduce la consistencia
- distintas etapas necesitan distintas instrucciones, distinto contexto y distintos criterios de revisión

Este workflow existe para evitar mezclar objetivos dentro de un mismo pedido.

---

## Idea principal
Una tarea puede requerir una o más etapas.

Ejemplos típicos:
- solo análisis
- análisis y plan
- implementar un cambio chico
- hacer una pasada de comentarios
- hacer una pasada de validación

No todas las tareas necesitan todas las etapas.

Cada etapa debería tener:
- un objetivo claro
- un alcance limitado
- no objetivos explícitos
- una salida visible que se pueda revisar

---

## Secuencia recomendada
Cuando haga falta, el trabajo debería dividirse normalmente en este orden:

1. Análisis
2. Plan
3. Implementación
4. Pasada de comentarios
5. Validación

Esta es la secuencia por defecto, no una regla rígida.

Para tareas chicas, análisis y plan pueden combinarse en una sola pasada.

Ejemplos:
- un bugfix chico puede usar solo análisis, implementación y validación
- un cambio más grande puede usar análisis, plan, implementación, pasada de comentarios y validación
- una mejora documental puede usar solo pasada de comentarios

---

## Definición de etapas

### 1. Análisis
Usar esta etapa para entender el código actual antes de cambiarlo.

Esta etapa debería responder:
- cómo funciona actualmente el módulo
- qué hacen archivos cercanos similares
- qué patrones locales de naming y estructura ya existen
- qué riesgos tiene el cambio pedido
- qué conviene no tocar

Salida típica:
- hallazgos técnicos breves
- punto recomendado de inserción o cambio
- lista de archivos o patrones relevantes
- advertencias sobre acoplamiento, ownership, contexto ISR, orden de init o supuestos de hardware

El análisis no debería derivar en un rediseño amplio especulativo salvo que se pida explícitamente.

---

### 2. Plan
Usar esta etapa para definir cómo conviene abordar el cambio una vez terminado el análisis.

Esta etapa debería definir:
- archivos o módulo objetivo
- cambio exacto buscado
- qué queda fuera de alcance
- si se permite cambiar comportamiento
- si se esperan pasadas posteriores
- secuencia recomendada de edición

Salida típica:
- una definición acotada de la tarea
- archivos afectados
- criterio de aceptación
- una estrategia breve de implementación

Para tareas chicas, esta etapa puede fusionarse con análisis.

---

### 3. Implementación
Usar esta etapa para hacer el cambio funcional o estructural.

Esta etapa puede incluir:
- agregar código
- cambiar comportamiento
- mover código
- dividir funciones
- mejorar naming
- remover código muerto
- reestructurar internals
- refactorizar código sin cambiar el comportamiento esperado

Esta etapa debería mantenerse enfocada en el cambio pedido.

Salvo que se pida explícitamente, esta etapa no debería intentar resolver todo esto al mismo tiempo:
- limpieza completa de comentarios
- normalización completa de naming en todo el módulo
- rediseño arquitectónico amplio
- validación exhaustiva
- correcciones de estilo no relacionadas

Salida típica:
- el cambio de código pedido
- resumen breve de qué cambió
- nota explícita de qué queda para pasadas posteriores, si aplica

---

### 4. Pasada de comentarios
Usar esta etapa después de implementar o refactorizar, cuando la estructura del código ya está lo suficientemente estable como para documentarla con claridad.

Esta etapa existe porque los comentarios suelen salir mejor cuando el comportamiento y la estructura ya están asentados.

Esta etapa puede incluir:
- agregar comentarios faltantes en la API pública
- remover comentarios redundantes
- corregir comentarios inexactos
- volver más consistente el estilo de comentarios
- documentar restricciones que no son obvias desde el código

Esta etapa no debería usarse como reemplazo de mal naming o mala estructura.

Salida típica:
- comentarios de API pública alineados con el comportamiento real
- comentarios privados solo donde aportan valor técnico real
- eliminación de comentarios viejos, engañosos o inconsistentes

Un documento específico de esta etapa debería definir las reglas detalladas de comentarios.

---

### 5. Validación
Usar esta etapa para verificar que el resultado es aceptable para integrar.

La validación puede incluir:
- verificación de build
- testing dirigido
- chequeos de debug
- sanity checks en runtime
- confirmación de que el comportamiento no cambió inesperadamente
- confirmación de que el cambio pedido quedó realmente completo

Esta etapa debería dejar claro qué fue validado y qué no.

Salida típica:
- qué se validó
- cómo se validó
- qué queda sin validar

La validación no es opcional por principio, pero la profundidad requerida depende del tipo de tarea.

---

## Cómo usar este directorio
Este directorio debería contener documentos cortos y específicos por etapa.

Cada documento de etapa debería responder:
- cuándo usarlo
- qué se puede cambiar en esa etapa
- qué no se debe cambiar
- qué significa que esa etapa está terminada

Los documentos de etapa deberían ser operativos.
Deben ayudar a humanos y agentes a ejecutar una pasada.
No deberían convertirse en manuales generales largos.

---

## Relación con los documentos en raíz

### Los documentos en raíz deberían conservar:
- reglas estables de todo el proyecto
- reglas de estilo de código
- reglas de naming
- reglas de arquitectura de aplicación amplia
- puntos de entrada al repositorio
- contexto duradero que convenga encontrar rápido

Ejemplos:
- `AGENT_ENTRY.md`
- `STYLE.md`
- `AGENTS.md`
- `README.md`

### Los documentos de workflow deberían contener:
- instrucciones específicas por etapa
- alcance y no objetivos de cada etapa
- criterio de cierre específico de la pasada
- ejemplos de cómo pedir o revisar esa pasada

En resumen:
- los documentos en raíz definen reglas duraderas del proyecto
- los documentos de workflow definen cuándo y cómo se aplica una etapa

---

## Enfoque de migración
Este repositorio no necesita una reescritura completa de documentación antes de empezar a usar un workflow por etapas.

La migración debería ser incremental.

Orden recomendado:
1. Definir este modelo de etapas
2. Agregar un documento de etapa con valor inmediato
3. Aplicarlo sobre un módulo real
4. Ajustarlo según uso real
5. Recién después extraer o reescribir el resto del material de workflow

El primer documento de etapa recomendado es:
- `comment_pass.md`

Motivos:
- la consistencia de comentarios hoy es despareja en el repo
- los comentarios suelen resolverse mejor una vez estabilizada la implementación
- esta etapa se puede aplicar con bajo riesgo
- esta etapa se puede revisar con claridad

---

## Regla práctica
No asumir que una primera pasada tiene que resolver todo.

Cuando haga falta, pedir explícitamente la siguiente pasada.

Ejemplos:
- "analizá primero este módulo"
- "analizá y proponé un plan"
- "hacé solo la implementación"
- "hacé una pasada de comentarios sobre estos dos archivos"
- "hacé una pasada de validación sobre este cambio"

Esto debería preferirse por sobre mezclar múltiples objetivos en un único pedido vago.

---

## Convención inicial
Hasta que existan más documentos de etapa, usar estos significados por defecto:

- análisis: entender el código actual y sus restricciones locales
- plan: definir el cambio buscado y la secuencia recomendada
- implementación: cambiar código
- pasada de comentarios: mejorar comentarios una vez estabilizado el código
- validación: verificar que el cambio está en condiciones de integrarse

Los futuros documentos de etapa pueden refinar estos significados, pero no deberían contradecirlos sin una buena razón.
