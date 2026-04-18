# Pasada de comentarios

## Proposito

Este documento define cómo ejecutar una pasada de comentarios en `firmware/`.

Una pasada de comentarios es una limpieza documental enfocada que se realiza después de que la estructura del código ya está lo suficientemente estable como para describirse con claridad.

El objetivo no es agregar más comentarios en todos lados.
El objetivo es volver los comentarios consistentes, técnicamente útiles y alineados con el comportamiento real del código.

Esta etapa es especialmente importante en sistemas embebidos porque los comentarios suelen tener que capturar cosas que no son obvias solo leyendo el código, como:

- restricciones entre ISR y foreground
- restricciones de orden de hardware
- límites de ownership
- comportamiento bloqueante versus no bloqueante
- supuestos de lifetime
- particularidades de la placa

---

## Cuándo usar esta etapa

Usar una pasada de comentarios cuando:

- una implementación o refactor ya está lo suficientemente completa como para documentarse
- un archivo tiene comentarios útiles pero con estilo inconsistente
- un archivo tiene comentarios viejos o engañosos
- un módulo mezcla dos o más estilos de comentarios
- una API pública necesita comentarios contractuales más claros

No usar esta etapa como reemplazo de:

- mal naming
- mala estructura de archivo
- responsabilidades mezcladas
- implementación todavía inestable

Si el código todavía se está moviendo mucho, primero terminar la implementación.

---

## Alcance

Una pasada de comentarios puede:

- agregar comentarios faltantes en la API pública
- remover comentarios viejos, decorativos o redundantes
- reescribir comentarios inexactos
- normalizar el estilo de comentarios dentro del archivo tocado
- aclarar restricciones técnicas no obvias
- reducir sobrecomentado en implementación privada

Una pasada de comentarios no debería:

- introducir cambios de comportamiento salvo que hagan falta para corregir una contradicción entre comentario y código
- hacer refactors amplios
- rediseñar ownership de módulos
- renombrar grandes superficies de API salvo que sea claramente necesario por consistencia
- mezclar limpieza documental con cambios lógicos no relacionados

Si un comentario contradice al código, la prioridad es:
1. verificar el comportamiento esperado
2. corregir el código o el comentario para que coincidan
3. no dejar la contradicción viva

---

## Regla orientada al repositorio

No inventar un estilo nuevo de comentarios dentro de un archivo.

Al tocar un archivo, hacerlo converger al método del repositorio definido aquí.
Si los comentarios legacy siguen siendo útiles, conservar su contenido técnico.
Si los comentarios legacy son ruidosos, duplicados, viejos o inconsistentes, removerlos o reescribirlos.

El repositorio no debería mantener dos sistemas de comentarios compitiendo dentro del mismo módulo.

---

## Modelo de comentarios para este repositorio

### Archivos de header (`.h`)

Los comentarios en headers deben documentar el contrato público.

Usar comentarios Doxygen cortos para APIs públicas.
Poner el foco en:
- qué ofrece la función
- parámetros o retorno cuando no sean obvios
- restricciones que el caller necesita conocer
- ISR-safety, comportamiento bloqueante, lifetime o requisitos de orden de init cuando aplique

Preferir formas concisas como:

```c
/** @brief Initialize the debug subsystem. */
void FM_DEBUG_Init(void);
```

Usar bloques Doxygen más largos solo cuando el contrato realmente lo necesite.
Los tags útiles suelen ser:
- `@param`
- `@return`
- `@note`
- `@warning`

Evitar bloques largos de `@details` salvo que agreguen valor contractual real.

No documentar detalles privados de implementación en headers públicos.

---

### Archivos fuente (`.c`)

Los comentarios en fuentes deben ser más livianos que en headers.

Usar comentarios C normales para notas de implementación.
En archivos `.c`, comentar solo cuando el código se beneficie de contexto técnico que no sea obvio por nombres y estructura.

Buenos objetivos para comentarios en `.c`:
- por qué importa una secuencia
- restricciones ISR versus foreground
- límites de ownership o adaptación
- particularidades de hardware
- política de descarte o buffering
- supuestos de timing, persistencia o concurrencia

Evitar usar Doxygen de forma amplia para funciones privadas, estado privado y helpers privados.

No documentar cada símbolo privado solo porque existe.

---

## Prioridades específicas de embebidos

Para este repositorio, los comentarios deberían priorizar preocupaciones de sistemas embebidos por sobre prosa general.

Los comentarios son especialmente valiosos cuando explican:

### ISR y contexto de ejecución
Ejemplos:
- productor ISR-safe, flush en foreground
- no IRQ-safe
- UART bloqueante solo en foreground
- callback fino que solo reenvía

### Orden de hardware
Ejemplos:
- clocks antes de usar periféricos
- GPIO antes de samplear debug
- pull-up temporal antes de volver el pin a analógico
- DWT solo después de que el baseline de runtime está listo

### Límites de ownership
Ejemplos:
- wrapper de board versus backend de port
- fachada pública versus adaptación de hardware
- helper local versus API exportada

### Recursos y lifetime
Ejemplos:
- puntero a string constante debe vivir más que el evento encolado
- buffer temporal solo para formato
- ring con descarte al llenarse para acotar latencia

Si un comentario no ayuda a aclarar alguno de estos tipos de restricciones, normalmente debería acortarse o eliminarse.

---

## Manejo del legacy

Usar las siguientes reglas cuando un archivo ya contiene comentarios legacy.

### Conservar
Conservar comentarios que todavía aporten valor técnico, especialmente si describen:
- restricciones de contexto de ejecución
- comportamiento de hardware
- límites de ownership
- supuestos de timing
- racional técnico no obvio

### Reescribir
Reescribir comentarios cuando:
- la redacción sea inconsistente con el método del repositorio
- la misma idea pueda expresarse más clara y brevemente
- un contrato público esté escondido dentro de una nota privada de diseño
- el comentario sea correcto pero demasiado largo para el valor que aporta

### Eliminar
Eliminar comentarios cuando sean:
- viejos
- contradichos por el código
- decorativos
- repetitivos
- narración línea por línea de código obvio
- bloques Doxygen grandes en privadas sin valor real de mantenimiento
- placeholders de sección sin contenido como `(none)`
- metadata como autor/fecha que ya está cubierta por control de versiones

### Regla frente a arbitrariedad
Si una regla nueva es arbitraria, preferir el legacy válido ya existente en el repositorio.
Si el legacy es inconsistente, engañoso, duplicado o poco útil, converger al método de este documento.

---

## Línea base actual del repositorio

Según el estado actual del repositorio, la dirección preferida es:

- conservar comentarios cortos de propósito a nivel archivo
- conservar comentarios de API pública en headers
- conservar comentarios técnicos sobre secuencia, contexto y restricciones de hardware
- reducir sobrecomentado de datos privados y helpers privados
- evitar bloques Doxygen grandes en `.c` para símbolos solo internos
- evitar estilos mezclados dentro del mismo archivo

Ejemplos del código actual:
- `bsp/fm_board.c` está más cerca del tono preferido para comentarios de implementación
- `libs/fm_debug.h` contiene comentarios útiles de contrato en la API pública
- `libs/fm_debug.c` contiene información técnica útil pero es demasiado pesado para implementación privada y debería reducirse en pasadas de comentarios
- `apps/main/fm_main.c` muestra comentarios de intención útiles pero debería normalizarse para ganar consistencia

---

## Reglas prácticas

### Header de archivo
Se permite un header de archivo corto cuando ayuda a establecer propósito.
Preferir:
- propósito del archivo
- una o dos restricciones clave si hace falta

Evitar headers largos que vuelvan a contar toda la implementación.

### Comentarios de API pública
Las APIs públicas deberían comentarse cuando son exportadas por el módulo.
Mantener los comentarios cortos y contractuales.

### Comentarios de funciones privadas
Comentar funciones privadas solo cuando necesiten explicación técnica.
Un buen nombre de función privada debería reducir la necesidad de muchos comentarios.

### Comentarios de datos privados
Comentar datos privados solo cuando tengan significado no obvio,
supuestos de concurrencia o restricciones de hardware.
No documentar por defecto cada buffer, flag o variable.

### Comentarios inline
Usar comentarios inline para:
- ramas no obvias
- dependencias de orden
- decisiones para acotar latencia
- efectos laterales fáciles de pasar por alto

No usar comentarios inline para repetir asignaciones simples o llamadas obvias.

---

## Reglas de normalización durante una pasada de comentarios

Al editar un archivo, normalizar el área tocada hacia estas convenciones:

- contrato público en `.h`
- comentarios técnicos más livianos en `.c`
- un idioma por archivo salvo razón fuerte
- un estilo de comentarios por archivo
- ninguna contradicción entre comentario y comportamiento

No dejar un archivo tocado a medio convertir si la inconsistencia es local y fácil de corregir.

---

## Criterio de cierre

Una pasada de comentarios está en buen estado cuando:

- las APIs públicas están documentadas de forma consistente
- los comentarios de implementación son técnicos y selectivos
- los comentarios viejos o contradictorios fueron removidos o corregidos
- la implementación privada no está sobrecomentada
- las restricciones específicas de embebidos están documentadas donde hace falta
- el archivo tocado ya no mezcla múltiples métodos de comentarios en competencia

---

## Checklist de review

Antes de cerrar una pasada de comentarios, verificar:

- [ ] los comentarios de header describen contrato, no internals
- [ ] los comentarios de fuente explican solo intención técnica no obvia
- [ ] ISR, bloqueo, orden, hardware y lifetime están documentados donde hace falta
- [ ] se removieron comentarios viejos o engañosos
- [ ] no se documentaron helpers privados en exceso
- [ ] no quedaron placeholders `(none)` en archivos tocados
- [ ] no quedó metadata de autor/fecha en archivos tocados salvo requerimiento explícito
- [ ] el estilo de comentarios es consistente dentro del archivo

---

## Documentos relacionados

- Modelo de workflow: `workflows/README.md`
- Reglas estables de estilo: `STYLE.md`
- Punto de entrada para agentes: `AGENT_ENTRY.md`
