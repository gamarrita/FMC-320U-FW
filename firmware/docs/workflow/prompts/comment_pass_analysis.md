Trabaja en `firmware/`.

Tipo de prompt:
- non-bootstrap
- opera sobre el contexto existente
- no crea ni reemplaza `WORKING_CONTEXT.md`

Analizá:
- <target .c>
- <target .h>

No modifiques código todavía.

Objetivo:
decidir cómo debería hacerse una pasada de comentarios en este módulo para dejarlo consistente con el repo.

Usá como referencia:
- `firmware/WORKING_CONTEXT.md`
- `firmware/STYLE.md`
- `firmware/docs/workflow/passes/comments/README.md`
- `firmware/libs/fm_debug.h`
- `firmware/libs/fm_debug.c`

Evaluá explícitamente:
1. qué comentarios actuales sirven
2. cuáles sobran
3. cuáles habría que reescribir
4. si faltan comentarios técnicos importantes
5. si el uso actual de Doxygen es correcto o no
6. qué debería tener Doxygen en el `.h`
7. qué no debería usar Doxygen en el `.c`
8. si hay desbalance entre el nivel de comentario del `.h` y del `.c`
9. si vale la pena hacer un patch de comment pass en este módulo

Prestá especial atención a:
- orden de inicialización de hardware
- límites entre board y port
- wrappers
- restricciones de contexto de ejecución

Criterio esperado:
- en el `.h`, Doxygen corto y contractual para la API pública
- en el `.c`, comentarios técnicos útiles y selectivos
- no Doxygen pesado para implementación privada
- no comentarios decorativos ni narración línea por línea

Quiero una respuesta corta, técnica y accionable.
