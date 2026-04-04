# 02_chatgpt.md — Plantillas operativas para ChatGPT
Versión: v1.0

**Regla general:** usar `STYLE.md`, `CHANGELOG.md`, `AGENTS.md`, `style-examples/` como referencias fijas.

## Plantillas / Flujos (resumidas)

### 1) Delimitar tarea y ubicarla
Input: descripción breve del problema.  
Output esperado de ChatGPT:
- Micro-tarea acotada (1–3 frases).
- Tipo de tarea (inspección / diseño / refactor / fix).
- Carpeta objetivo y porqué.
- Archivos implicados.
- Siguiente prompt para Codex.

### 2) Analizar módulo y evaluar ubicación
Input: ruta o módulo.  
Salida:
- Resumen técnico (public/private, dependencias, estado).
- Evaluación si está en carpeta correcta.
- Ejemplo(s) de `style-examples/` para forma.
- Siguiente micro-tarea.

### 3) Diseñar interfaz alineada al repo
Salida:
- API mínima (.h), responsabilidades incluidas/excluidas.
- Carpeta propuesta.
- Migración incremental y riesgos.

### 4) Revisar propuesta de Codex
Input: diff o patch.  
Salida:
- Validación contra `STYLE.md`, `style-examples/`, `CHANGELOG.md`, `AGENTS.md`.
- Lista de correcciones necesarias y justificación.

### 5) Generar prompt para Codex
Debe incluir:
- Objetivo concreto.
- Carpeta objetivo.
- Restricciones (no tocar fuera del scope, respetar `STYLE.md`, no modificar `cube/`).
- Salida verificable esperada (git patch + checklist).

### 6) Debug guiado
Salida:
- Hipótesis ordenadas por probabilidad.
- Evidencias a buscar.
- Plan mínimo de instrumentación.
- Paso siguiente.

### 7) Cerrar tarea
Salida:
- Qué se resolvió.
- Dónde quedó la responsabilidad y porqué.
- Reglas del repo usadas.
- Dudas abiertas y siguiente micro-tarea.