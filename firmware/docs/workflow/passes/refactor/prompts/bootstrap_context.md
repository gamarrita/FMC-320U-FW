Trabaja en `firmware/`.

Tipo de prompt:
- bootstrap wizard MVP
- limitado a `refactor`
- puede crear o reencuadrar `WORKING_CONTEXT.md`
- debe dejar un contexto activo nuevo o actualizado bajo `docs/contexts/`

Objetivo:
dejar un par de contexto inicial útil para un refactor chico, claro y
revisable, sin expandir el alcance innecesariamente.

Leé primero:
- `firmware/AGENTS.md`
- `firmware/STYLE.md`
- `firmware/docs/workflow/README.md`
- `firmware/docs/workflow/passes/refactor/README.md`

Comportamiento esperado:
1. inferí del pedido del usuario y del repo todo lo que se pueda inferir con
   seguridad
2. antes de bootstrappear, determiná si el caso es:
   - `create`
   - `reframe`
3. si es `reframe`, no crees un contexto nuevo a ciegas:
   - primero identificá qué par activo se está reencuadrando
   - y cuál será el nuevo foco
4. antes de bootstrappear, asegurate de conocer como mínimo:
   - el target real del refactor
   - el comportamiento que debe preservarse
   - el problema estructural real
   - al menos un límite claro fuera de alcance
   - el siguiente paso concreto de análisis
   - si es `reframe`, qué par activo se está actualizando
5. si falta alguno de esos datos, hacé preguntas cortas y concretas
6. preguntá sólo por lo que sigue faltando
7. detené las preguntas apenas ya puedas dejar un contexto activo útil

No bloquees el bootstrap por:
- lista exacta de archivos candidatos
- estrategia final de validación
- inventario completo de side effects
- historial largo de rationale

Restricciones:
- no modificar código todavía
- no proponer rediseño amplio
- no pedir un plan de implementación completo
- no hacer una explicación larga
- no duplicar policy estable de `AGENTS.md`
- mantener ownership claro
- mantener naming consistente con `STYLE.md`

Condición de stop:
dejá de preguntar y bootstrappeá cuando:
- el caso ya puede tratarse con confianza como un `refactor`
- ya está claro si el wizard debe crear o reencuadrar
- el target del refactor ya está nombrado con precisión suficiente
- el comportamiento a preservar ya es explícito
- el problema estructural ya es explícito
- ya existe al menos un límite fuera de alcance
- el siguiente paso ya es una acción concreta y ejecutable

Si la condición de stop todavía NO se cumple:
respondé usando exactamente esta estructura:

Preguntas:
- Known
- Missing blocking input
- Questions
- Stop reason

Si la condición de stop sí se cumple:
1. creá o actualizá `WORKING_CONTEXT.md`
2. creá o actualizá `docs/contexts/<slug>.md`
3. dejá el trabajo listo para arrancar en `analysis`

Respondé usando exactamente esta estructura:

WORKING_CONTEXT.md:
- Current Work
- Stage = analysis
- Active pass = refactor
- Operational context:
  - Area
  - Core structure
  - Current focus
  - Constraints
- Next Step
- References:
  - `AGENTS.md`
  - `docs/contexts/<slug>.md`
  - `STYLE.md`
- Invocation Rule
- Maintenance Rule
- Strong Rule

docs/contexts/<slug>.md:
- Purpose
- Current State
- Scope
- Observed Validated Case or Trusted Reference Evidence
- Known Gaps
- Risks
- Minimum Plan

Resumen corto:
- ...

Do NOT:
- escribir análisis largo en `WORKING_CONTEXT.md`
- convertir el bootstrap en una entrevista grande
- pedir información que ya puede inferirse del repo
- forzar la salida de contexto si todavía faltan blockers
- copiar completo el archivo de contexto extendido
- olvidar embebir la regla de mantener alineados `WORKING_CONTEXT.md` y
  `docs/contexts/<slug>.md`
