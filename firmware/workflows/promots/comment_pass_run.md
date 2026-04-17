Trabaja en `firmware/`.

Tarea:
Ejecuta una pasada de comentarios sobre `bsp/fm_board.c`.

Objetivo:
Aplicar el método de comment pass del repositorio a `bsp/fm_board.c` y dejar el archivo con un único estilo de comentarios consistente, sin cambiar el comportamiento esperado.

Rol de los documentos:
- `firmware/AGENT_ENTRY.md` es el punto de entrada operativo
- `firmware/STYLE.md` es la fuente de verdad para estilo y naming
- `firmware/workflows/README.md` define las etapas del workflow
- `firmware/workflows/comment_pass.md` define el método canónico de comment pass
- `firmware/05_working_context.md` puede aportar continuidad de la sesión actual, pero no debe sobrescribir reglas estables
- `firmware/libs/fm_debug.h` y `firmware/libs/fm_debug.c` son el patrón concreto de referencia para un comment pass ya completado

Referencias:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/workflows/README.md
- firmware/workflows/comment_pass.md
- firmware/05_working_context.md
- firmware/libs/fm_debug.h
- firmware/libs/fm_debug.c

Restricciones:
- cambio enfocado solo en comentarios
- no cambiar comportamiento
- no hacer limpieza no relacionada
- conservar comentarios legacy que sigan siendo útiles
- eliminar comentarios viejos, redundantes, decorativos o contradictorios
- eliminar placeholders vacíos como `(none)`
- eliminar bloques de secciones completamente vacías si ya no aportan valor
- no introducir documentación pesada estilo Doxygen en archivos `.c`
- mantener los comentarios de implementación técnicos y selectivos
- mantener los comentarios alineados con restricciones de sistemas embebidos y con el comportamiento actual del código
- usar `fm_debug.h/.c` como ejemplo de referencia para el estilo esperado del resultado
- usar `05_working_context.md` solo si aporta continuidad relevante para esta tarea
- prestar especial atención al orden de inicialización de board bring-up, a los límites board-versus-port, a la intención de los wrappers y a la secuencia de hardware
- evitar comentar línea por línea wrappers obvios

Resultado esperado:
- patch mínimo
- comportamiento esperado sin cambios
- comentarios más claros y orientados a embedded
- ningún estilo de comentarios en competencia dentro del archivo tocado
- sin placeholders vacíos ni bloques legacy vacíos, salvo que todavía cumplan una función estructural real