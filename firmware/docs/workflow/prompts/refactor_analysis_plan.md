Trabaja en `firmware/`.

Tarea:
Hacé una pasada de análisis y plan sobre <módulo o archivo objetivo> para evaluar un posible refactor de código.

Objetivo:
entender la estructura actual del módulo y proponer un refactor chico, claro y revisable, sin expandir el alcance innecesariamente.

Usá como referencia:
- `firmware/WORKING_CONTEXT.md`
- `firmware/STYLE.md`
- `firmware/docs/workflow/README.md`
- `firmware/docs/contexts/...` solo si hay una línea activa de trabajo relevante para esta tarea

Restricciones:
- análisis y plan solamente
- no modificar código todavía
- no proponer cambios de comportamiento salvo que detectes una inconsistencia real
- no expandir el alcance más allá del módulo o archivo objetivo
- no inventar nueva arquitectura
- mantener ownership claro
- mantener naming consistente con `STYLE.md`

Quiero que determines:
1. cuál es la responsabilidad actual del módulo
2. qué problema estructural real tiene hoy
3. si mezcla responsabilidades
4. qué habría que preservar
5. qué partes serían candidatas a refactor
6. qué archivos habría que tocar
7. qué debería quedar fuera de alcance
8. si el refactor puede hacerse con un patch chico y revisable
9. cuál sería el plan mínimo de implementación

Respondé usando exactamente esta estructura:

Responsabilidad actual:
- ...

Problema estructural:
- ...

Mezcla de responsabilidades:
- ...

Preservar:
- ...

Candidatos a refactor:
- ...

Archivos a tocar:
- ...

Fuera de alcance:
- ...

Riesgos:
- ...

Refactor chico y revisable:
- sí/no
- por qué

Plan mínimo:
- ...
