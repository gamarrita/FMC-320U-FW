# cube

## Intención
Contener código generado o fuertemente condicionado por STM32CubeMX y la base HAL asociada.

## Debe contener
- archivos generados,
- configuración base asociada al tooling,
- elementos regenerables o condicionados por CubeMX.

## No debe contener
- lógica de aplicación,
- decisiones funcionales,
- servicios reutilizables,
- refactors manuales que deberían vivir en otras carpetas.

## Relación con otras carpetas
`cube/` debe mantenerse separado para preservar regenerabilidad y ownership del tooling.
Las demás capas deberían apoyarse en esto sin convertirlo en lugar de lógica manual.

## Criterio práctico
Si el código debe mantenerse separado para no romper regeneración o ownership del tooling, probablemente pertenece aquí.