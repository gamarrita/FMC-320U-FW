# Caso de uso: Codex en un firmware STM32 real con VS Code

## Propósito

Este documento resume una experiencia concreta de programación con agentes en
un entorno de firmware real. El objetivo no fue hacer una demo aislada, sino
trabajar sobre un repositorio existente, con reglas de arquitectura, build
canónico, refactor incremental y validación en hardware por UART.

## Petición humana, redactada como objetivo técnico

Reorganizar `src/apps` porque las aplicaciones de bring-up, tests y demos
quedaron mezcladas. La solución debía ser simple, apropiada para un firmware
mediano y compatible con el criterio práctico de un ingeniero senior de
firmware.

Además, el cambio debía verificarse con el flujo canónico STM32Cube/VS Code,
flasheando la placa cuando aplicara y leyendo la UART para confirmar que el
firmware ejecutaba correctamente los tests.

## Contexto del repositorio

El proyecto usa:

- STM32U5
- VS Code con bundle STM32
- CMake integrado al flujo STM32Cube
- selección de aplicación mediante `FM_ACTIVE_APP`
- salida UART como mecanismo de validación humana y automática
- contexto operativo en `WORKING_CONTEXT.md`
- reglas de trabajo en `AGENTS.md`

La regla importante fue no inventar un build alternativo. La validación debía
usar el flujo documentado en `docs/canonical-build/stm32cube-cli-workflow.md`.

## Refactor realizado

La carpeta `src/apps` pasó de una estructura plana a una estructura por rol:

```text
src/apps/
  product/
    main/
  bringups/
    lcd/
    lcd_blink/
    display_format_lcd/
    debug_panic/
  tests/
    regression/
  template/
```

Decisiones relevantes:

- `panic_demo` pasó a `bringups/debug_panic`, porque valida caminos de debug,
  panic, error-handler y hard fault.
- `fmc_model_units_test` pasó a `tests/regression`, porque ya no probaba solo
  unidades: también cubría rate, volume y display format.
- No se creó `demos/` todavía, para evitar taxonomía prematura.
- No se creó un framework de tests nuevo: se mantuvo UART `PASS`/`FAIL`.

## Validación realizada

Codex verificó referencias cruzadas para evitar paths viejos y compiló con el
flujo canónico:

- `tests/regression`
- `bringups/display_format_lcd`
- `bringups/lcd`
- `bringups/lcd_blink`
- `bringups/debug_panic`
- `product/main`
- `template`

Luego flasheó la placa STM32 y capturó salida UART por `COM7`.

Resultado observado:

```text
DEBUG_INIT:MSG=ENABLED LED=ENABLED
REGRESSION_TEST:START
REGRESSION_TEST:INIT_DEFAULTS:PASS
REGRESSION_TEST:TOTALS:PASS
REGRESSION_TEST:RATE_WINDOWS:PASS
REGRESSION_TEST:VOLUME_VALUES:PASS
REGRESSION_TEST:DISPLAY_FORMAT_VALUES:PASS
REGRESSION_TEST:DISPLAY_FORMAT_ERROR_PATHS:PASS
REGRESSION_TEST:PASS
```

## Por qué importa

El valor no fue solo generar código. Fue cerrar el ciclo:

```text
intención humana
  -> análisis del repo
  -> refactor pequeño
  -> documentación
  -> build canónico
  -> flash
  -> UART PASS
```

Ese ciclo es el que vuelve interesante a un agente de IA en firmware real.

