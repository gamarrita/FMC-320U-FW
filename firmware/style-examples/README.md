# Examples

Este directorio contiene ejemplos mínimos de estilo para FMC-320U.

## Contenido

- `baremetal/fm_main_simple.h/.c`
  - Aplicación baremetal mínima con `Init()` y `Main()`.
- `baremetal/fm_gpio_poll.h/.c`
  - Módulo simple por polling.
- `rtos/fmx_kernel_basic.h/.c`
  - Integración mínima con ThreadX.
- `rtos/fm_task_simple.h/.c`
  - Tarea simple de aplicación sobre RTOS.
- `interrupts/fm_exti_flag.h/.c`
  - Manejo mínimo de evento por interrupción.
- `drivers/fm_adc_basic.h/.c`
  - Driver simple con init y lectura.

## Objetivo

Estos ejemplos no buscan mostrar arquitectura final ni features reales del
producto. Solo sirven como referencia de:
- estructura de archivos
- naming
- separación API pública / privados
- uso básico baremetal / RTOS / ISR
- simplicidad de implementación