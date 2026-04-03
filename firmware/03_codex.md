# Plantillas operativas para Codex
Versión: v0.5

## Regla general
Para cualquier análisis o cambio dentro del repo:
- revisar `STYLE.md`,
- revisar `CHANGELOG.md` si la tarea depende de continuidad,
- revisar `AGENTS.md` si la tarea se ejecuta como trabajo delegado,
- usar `style-examples/` como referencia mínima de estilo y forma,
- respetar la separación entre:
  - `app/`
  - `bsp/`
  - `cube/`
  - `port/`
  - `services/`

Importante:
No uses `style-examples/` como arquitectura objetivo.
Úsalo como referencia mínima de naming, forma de módulo, separación pública/privada y simplicidad.

No asumas naming de archivos o prefijos por intuición.
Si detectas inconsistencias o dudas, explícalas antes de proponer cambios.

---

## 1. Inspección de módulo con foco en capa y estilo
Inspecciona este módulo del proyecto y genera un resumen técnico estructurado.

Alcance:
[ruta o archivos]

Antes de resumir:
1. revisa `STYLE.md`,
2. identifica ejemplos relevantes en `style-examples/`,
3. considera `CHANGELOG.md` si aporta continuidad,
4. considera `AGENTS.md` si la tarea implica reglas de ejecución del agente.

Necesito:
- funciones públicas y privadas,
- dependencias,
- variables de estado relevantes,
- acceso a hardware,
- responsabilidad principal del módulo,
- evaluación de si está en la carpeta correcta,
- ejemplos mínimos de `style-examples/` que se parezcan en forma.

Restricciones:
- no propongas cambios todavía,
- no asumas arquitectura ideal,
- basate en lo observado.

---

## 2. Buscar impacto de un cambio
Quiero evaluar el impacto de cambiar [función / interfaz / módulo].

Tarea:
Inspecciona el código y detecta:
- dónde se usa,
- qué módulos dependen de esto,
- qué carpeta parece ser responsable,
- qué riesgos habría al cambiarlo o moverlo.

Referencias:
- `STYLE.md`
- `CHANGELOG.md`
- `AGENTS.md`
- `style-examples/`

Restricciones:
- no modifiques código,
- no rediseñes todavía,
- prioriza trazabilidad real.

Salida:
Mapa de impacto, riesgos y observaciones sobre ubicación por capa.

---

## 3. Proponer cambio acotado en una carpeta específica
Necesito una propuesta de cambio localizada.

Objetivo:
[describir objetivo]

Carpeta objetivo:
[`app/` / `bsp/` / `cube/` / `port/` / `services/`]

Alcance permitido:
[archivos]

Antes de proponer cambios:
1. revisa `STYLE.md`,
2. revisa ejemplos aplicables en `style-examples/`,
3. verifica si `CHANGELOG.md` aporta contexto útil,
4. verifica si `AGENTS.md` impone reglas de trabajo relevantes.

Restricciones:
- no tocar archivos fuera del alcance,
- no mover responsabilidades sin justificarlo,
- no mezclar este cambio con otros refactors,
- mantener comportamiento actual salvo indicación explícita,
- usar `style-examples/` solo como referencia de forma mínima,
- no asumir naming de archivos sin verificar.

Primero:
1. resume qué entendiste,
2. explica por qué esta responsabilidad vive en esa carpeta,
3. explica qué ejemplo de `style-examples/` tomás como referencia de forma,
4. explica qué vas a cambiar y por qué.

Después:
5. propone el cambio.

---

## 4. Refactor incremental respetando la estructura del repo
Quiero refactorizar esta parte del código de forma incremental.

Objetivo:
[describir]

Alcance:
[archivos / módulo]

Necesito:
1. identificar responsabilidades actuales,
2. decir cuáles pertenecen realmente a la carpeta actual,
3. señalar cuáles parecen estar fuera de lugar,
4. contrastar la forma del módulo con `STYLE.md` y con ejemplos de `style-examples/`,
5. proponer una separación mínima razonable,
6. aplicar solo el primer paso del refactor.

Restricciones:
- no reescribir todo,
- preservar compatibilidad funcional,
- no usar `style-examples/` como arquitectura final,
- mantener el cambio listo para revisión humana.

---

## 5. Crear borrador de interfaz alineado con el repo
Necesito un borrador de interfaz para [módulo / servicio / driver].

Contexto:
[describir problema]

Carpeta objetivo:
[`app/` / `bsp/` / `port/` / `services/`]

Antes de diseñar:
- revisa `STYLE.md`,
- busca ejemplos parecidos en `style-examples/`,
- toma en cuenta `CHANGELOG.md` si aplica,
- considera `AGENTS.md` si el cambio será delegado.

Restricciones:
- mantener la interfaz pequeña,
- no mezclar hardware con lógica de aplicación,
- pensar en migración incremental,
- evitar cambios masivos,
- verificar naming antes de proponer archivos nuevos.

Salida:
- propuesta de `.h`,
- responsabilidades incluidas,
- responsabilidades excluidas,
- justificación de por qué vive en esa carpeta,
- referencia de estilo mínima tomada de `style-examples/`.

---

## 6. Revisar tu propia propuesta antes de mostrarla
Antes de proponer cambios finales, revisa críticamente tu propia propuesta.

Verifica:
- si la responsabilidad quedó en la carpeta correcta,
- si respetaste `STYLE.md`,
- si usaste `style-examples/` como referencia mínima y no como arquitectura,
- si ignoraste algo relevante de `CHANGELOG.md`,
- si tocaste más de lo necesario,
- si mezclaste capas,
- si asumiste naming sin comprobarlo.

Después:
- resume riesgos,
- indica dudas,
- presenta el diff final.

---

## 7. Cambio compatible con CubeMX
Necesito trabajar en una zona del firmware que convive con código generado por STM32CubeMX.

Objetivo:
[describir]

Carpeta objetivo:
[`cube/` o interacción con `bsp/`, `services/`, `app/`]

Restricciones:
- no colocar lógica de aplicación dentro de zonas generadas,
- respetar límites entre código regenerable y código manual,
- mantener la separación de capas del repo,
- revisar `STYLE.md` y `style-examples/`.

Tarea:
Proponer una forma de integrar este cambio respetando esa separación, con impacto mínimo.

---

## 8. No ejecutar, solo plan técnico
No quiero cambios todavía.

Quiero que:
1. inspecciones el código relevante,
2. revises `STYLE.md`,
3. identifiques ejemplos útiles en `style-examples/`,
4. uses `CHANGELOG.md` si aporta contexto,
5. uses `AGENTS.md` si afecta la forma de ejecución,
6. describas estado actual,
7. propongas plan técnico paso a paso,
8. señales qué carpeta debería contener cada responsabilidad,
9. esperes al siguiente prompt antes de modificar nada.