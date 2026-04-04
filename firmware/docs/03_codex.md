# 03_codex.md — Plantillas operativas para Codex
Versión: v1.0

**Regla general:** Codex ejecuta cambios acotados y verificables. Antes de ejecutar, debe revisar `STYLE.md`, `CHANGELOG.md`, `AGENTS.md` y `style-examples/`.

## Plantillas / Flujos clave

### 1) Inspección de módulo
Entrada: ruta(s).  
Salida:
- Funciones públicas/privadas.
- Dependencias y acceso HW.
- Responsabilidad principal y evaluación de ubicación.
- Ejemplos de `style-examples/` de referencia.

### 2) Evaluar impacto de un cambio
Salida:
- Lista de usos y dependencias.
- Riesgos y módulos afectados.
- Carpeta responsable y justificación.

### 3) Propuesta de cambio acotado
Salida:
- Resumen de entendimiento.
- Justificación de carpeta.
- Patch git que implemente el primer paso (solo archivos en scope).
- Checklist de validación.

### 4) Refactor incremental
Reglas:
- No rediseñar masivamente.
- Mantener compatibilidad funcional.
- Aplicar el primer paso y dejar claro siguiente paso.

### 5) Interfaces y headers
Salida:
- Borrador `.h` con API pública mínima.
- Responsabilidades excluidas.
- Estrategia de migración.

### 6) CubeMX-compat
Reglas:
- No meter lógica de aplicación dentro de código regenerado.
- Cambios en `cube/` solo vía puntos de integración definidos.

### 7) Salida esperada estándar (formato)
1. Justificación (1–2 párrafos).
2. Lista de archivos creados/modificados.
3. Git patch/diff.
4. Checklist validación (estilos, carpeta, `cube/`, tests).
5. Plan de smoke test/manual.

**Nota:** si detectas inconsistencias de naming, reportar y proponer opciones en lugar de corregir automáticamente.