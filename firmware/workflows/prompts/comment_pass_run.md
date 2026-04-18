Trabaja en `firmware/`.

Hacé una pasada de comentarios sobre:
- <target .c>
- <target .h>

Objetivo:
mejorar los comentarios sin cambiar comportamiento.

Reglas generales:
- en el `.h`, usar Doxygen corto y contractual para la API pública
- en el `.c`, no agregar Doxygen para implementación privada
- en el `.c`, dejar solo comentarios técnicos útiles y selectivos
- conservar comentarios legacy que sigan aportando valor
- eliminar comentarios redundantes, decorativos, vacíos o contradictorios
- no comentar línea por línea código obvio
- no hacer cleanup no relacionado
- no cambiar lógica

Referencias:
- `firmware/STYLE.md`
- `firmware/workflows/comment_pass.md`
- `firmware/libs/fm_debug.h`
- `firmware/libs/fm_debug.c`

Aplicá además estas decisiones específicas del análisis:
Todas las recomendadas