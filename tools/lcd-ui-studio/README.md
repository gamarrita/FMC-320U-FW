# FMC LCD UI Studio

MVP local para proponer, revisar y simular modelos visuales del LCD segmentado
del FMC-320U. La herramienta representa las pantallas aceptadas en Fase 8, sus
datos de ejemplo y las transiciones producidas por botones.

## Ejecutar

Requiere Node.js 22.13 o posterior.

```powershell
npm install
npm run dev
```

Abrir la URL informada por el servidor. Para verificar cambios:

```powershell
npm test
npm run lint
npm run build
```

## Artefactos del MVP

- `public/projects/fmc-phase8.yaml`: proyecto visual portable de referencia.
- `public/lcd-ui-project.schema.json`: forma pública del proyecto versión 1.
- `lib/lcd-model.mjs`: validación, formato de campos y navegación, sin React.
- `lib/fmc-160ldc-profile.mjs`: geometría visible normalizada del vidrio,
  separada de las pantallas y del ruteo eléctrico.
- `app/page.tsx`: editor/simulador; no es autoridad de comportamiento.

El perfil visual toma dimensiones y disposición de `datasheet/FMC-160LDC-EX.pdf`
y `datasheet/ENH-SS024197-01-YFPHGN-1.pdf`. La envolvente frontal sigue la
placa real V1R1R2 de `datasheet/fmc-320.jpg`: ESC y ENTER arriba, DOWN y UP
abajo. `EXT1` y `EXT2` se muestran como pulsadores externos, no como teclas del
frente. `260131 - CAD 4.jpg` aporta solamente la forma general; el orden de
teclas de la fotografía real prevalece sobre ese render anterior.
Color, contraste y escala aparente de los elementos activos se calibran con
`datasheet/fmc-320-lcd-all-segments-on.jpg`; la casi nula presencia de
segmentos apagados y la composición TTL/RATE se verifican con
`datasheet/fmc-320-lcd-acm-rate.jpg`.

El YAML modela propuestas de pantalla por identificador estable, variables de
simulación, campos del vidrio, indicadores y eventos semánticos
`SHORT_*`/`LONG_*`. El simulador permite importar, validar, modificar y
exportar ese archivo. Una propuesta sólo se convierte en contrato detallado de
producto mediante revisión humana explícita y referencia desde su documento de
producto propietario.

## Límites intencionales

Este MVP no implementa configuración del instrumento, edición transaccional de
parámetros, autenticación, persistencia, acceso al RTC ni generación de código
C. Los controles de datos cambian únicamente valores de simulación y no
representan edición del operador. El esquema actual tampoco define por sí solo
autorización, rangos, borradores, aplicar o cancelar. La forma física del vidrio
sigue gobernada por
`docs/specs/lcd/lcd_true_source.yaml`.
