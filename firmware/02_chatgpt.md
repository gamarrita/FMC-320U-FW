# Plantillas operativas para ChatGPT
Versión: v0.5

## Regla general
En todas estas plantillas, usar como referencia:
- `STYLE.md` como documento normativo,
- `CHANGELOG.md` como contexto de continuidad,
- `AGENTS.md` como guía de interacción con agentes,
- `style-examples/` como ejemplos mínimos de estilo y forma.

Importante:
No usar `style-examples/` como arquitectura objetivo del firmware.
Usarlo como referencia de estructura, naming, separación pública/privada y simplicidad.

No asumir naming de archivos o prefijos por intuición.
Si hay duda, contrastar contra `STYLE.md` y ejemplos reales.

---

## 1. Delimitar tarea y ubicarla en la carpeta correcta
Estoy trabajando en el refactor de un firmware embebido dentro de este repo.

Contexto:
- Deben respetarse `STYLE.md`, `CHANGELOG.md`, `AGENTS.md` y `style-examples/`
- El firmware se organiza en `app/`, `bsp/`, `cube/`, `port/`, `services/`
- `style-examples/` debe usarse como referencia mínima de estilo y forma, no como arquitectura final

Tarea inicial:
[describir problema]

Necesito:
1. recortar la tarea a una unidad pequeña y verificable,
2. identificar el tipo de tarea,
3. proponer la carpeta correcta,
4. indicar qué carpetas no deberían absorber esta responsabilidad,
5. decir qué referencia del repo conviene consultar,
6. proponer el siguiente prompt para Codex.

Separá:
- hechos,
- inferencias,
- recomendaciones.

---

## 2. Analizar un módulo y evaluar su ubicación
Estoy en fase de comprensión.

Objetivo:
Entender este módulo y verificar si su responsabilidad actual está ubicada en la carpeta correcta del repo.

Alcance:
[ruta, código o módulo]

Referencias obligatorias:
- `STYLE.md`
- `CHANGELOG.md`
- `AGENTS.md`
- `style-examples/`

Necesito:
1. resumen técnico del módulo,
2. responsabilidades actuales,
3. dependencias,
4. señales de acoplamiento,
5. evaluación de si está en la carpeta correcta,
6. qué ejemplo(s) de `style-examples/` podrían servir de referencia de forma,
7. siguiente micro-tarea.

---

## 3. Diseñar una interfaz alineada con el repo
Estoy en fase de diseño.

Objetivo:
Proponer una interfaz para [módulo / servicio / driver] respetando la separación de responsabilidades del repo.

Contexto:
[describir problema]

Restricciones:
- respetar `STYLE.md`,
- usar `style-examples/` solo como referencia de forma y simplicidad,
- mantener continuidad con `CHANGELOG.md` si aplica,
- considerar `AGENTS.md` si la implementación será delegada,
- no mezclar lógica de aplicación con hardware o código generado,
- no asumir naming sin verificar.

Necesito:
1. qué responsabilidad debería tener la interfaz,
2. en qué carpeta debería vivir,
3. qué carpetas no deberían contener esta lógica,
4. propuesta de API mínima,
5. estrategia de migración incremental,
6. riesgos.

---

## 4. Revisar trabajo de Codex contra referencias del repo
Estoy en fase de verificación.

Te paso una propuesta o diff generado por Codex.

Objetivo:
Revisarlo contra la estructura del repo y sus referencias.

Responsabilidad esperada en:
[carpeta]

Referencias obligatorias:
- `STYLE.md`
- `CHANGELOG.md`
- `AGENTS.md`
- `style-examples/`

Contenido:
[pegar diff o código]

Necesito:
1. validar si responde al objetivo,
2. validar si está en la carpeta correcta,
3. detectar conflictos con `STYLE.md`,
4. verificar si el uso de `style-examples/` es correcto,
5. señalar si ignoró algo relevante de `CHANGELOG.md`,
6. señalar si la forma de trabajo contradice `AGENTS.md`,
7. proponer correcciones.

---

## 5. Generar prompt para Codex
Necesito que conviertas esta tarea en un prompt operativo para Codex.

Contexto:
[describir tarea]

Objetivo:
[resultado esperado]

Carpeta objetivo:
[`app/` / `bsp/` / `cube/` / `port/` / `services/`]

Restricciones:
- respetar `STYLE.md`,
- usar `style-examples/` como referencia mínima de estilo,
- tomar `CHANGELOG.md` en cuenta si la tarea continúa cambios previos,
- considerar `AGENTS.md` si la tarea se delega al agente,
- no tocar capas innecesarias,
- mantener el cambio acotado y revisable,
- no asumir naming sin verificar contra repo.

Quiero un prompt que:
1. limite alcance,
2. haga referencia explícita a los documentos del repo,
3. pida justificar por qué la responsabilidad vive en esa carpeta,
4. pida una salida verificable,
5. desaliente rediseño innecesario.

---

## 6. Decidir si una responsabilidad está en la carpeta correcta
Necesito analizar si esta responsabilidad está ubicada en la capa correcta del repo.

Contexto:
[describir código, módulo o responsabilidad]

Estructura disponible:
- `app/`
- `bsp/`
- `cube/`
- `port/`
- `services/`

Referencias:
- `STYLE.md`
- `CHANGELOG.md`
- `AGENTS.md`
- `style-examples/`

Necesito:
1. clasificar la responsabilidad,
2. decir en qué carpeta debería vivir idealmente,
3. explicar por qué,
4. indicar qué dependencias serían aceptables o no,
5. proponer el mínimo movimiento razonable para corregir la ubicación si hiciera falta.

---

## 7. Debug guiado
Estoy en fase de debug dentro de firmware.

Problema observado:
[describir síntoma]

Contexto:
[describir módulo, periférico, flujo, ISR, init, etc.]

Referencias:
- `STYLE.md`
- `CHANGELOG.md`
- `style-examples/`

Restricciones:
- no asumir causas sin evidencia,
- priorizar hipótesis observables,
- considerar timing, init order, concurrencia, estado interno y acoplamientos,
- mantener separación de responsabilidades del repo.

Necesito:
1. hipótesis ordenadas por probabilidad o impacto,
2. evidencias que buscar para confirmar o descartar cada una,
3. plan mínimo de instrumentación,
4. siguiente paso concreto de debug.

---

## 8. Cerrar una tarea dejando contexto útil
Terminamos una tarea del refactor y quiero cerrar bien el contexto.

Información:
[pegar resumen]

Necesito:
1. qué se resolvió,
2. en qué carpeta quedó la responsabilidad y por qué,
3. qué reglas o referencias del repo fueron relevantes (`STYLE.md`, `CHANGELOG.md`, `AGENTS.md`, `style-examples/`),
4. qué dudas quedan abiertas,
5. cuál es la siguiente micro-tarea.