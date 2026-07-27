> Human reference only.
>
> This tutorial is not part of the active agent route, does not define the
> current workstream, and does not define the public API contract.
>
> For agent work, start with `AGENTS.md` and, when present,
> `WORKING_CONTEXT.md`, then follow the applicable roadmap and specifications.
>
> The authoritative API contract is `src/product/fmc/fmc_runtime.h`.
> Do not apply `docs/workflow/doc_closure.md` to delete or compress this file
> unless the user explicitly asks to curate human tutorial/reference material.

# Como identificar `fmc_runtime` con y sin RTOS

## Dos usos de la palabra runtime

Conviene separar dos conceptos:

1. Runtime en sentido general: todo el firmware mientras esta funcionando
   despues de la inicializacion: threads, interrupciones, callbacks, timers,
   scheduler y codigo del `while (1)`.
2. `fmc_runtime` como modulo arquitectonico: el componente que coordina eventos,
   ordena operaciones y controla el acceso al estado vivo de `fmc_service`.

Por lo tanto, un thread no pertenece a `fmc_runtime` porque este en estado
`RUNNING`, sino por la responsabilidad que ejecuta.

## Ejemplo con RTOS

Supongamos cuatro threads:

```text
AcquisitionThread
FmcRuntimeThread
UiThread
LoggerThread
```

### `AcquisitionThread`

Transforma informacion de hardware en deltas de pulsos.

No es `fmc_runtime`. Es un productor de eventos.

Responde preguntas como:

```text
Que ocurrio en el hardware?
Cuantos pulsos fueron capturados?
```

### `FmcRuntimeThread`

Recibe eventos y ejecuta las operaciones correspondientes sobre el estado vivo.

Este si pertenece a `fmc_runtime`, porque:

- recibe eventos del producto;
- define su orden de procesamiento;
- es propietario del acceso a `fmc_service`;
- evita modificaciones concurrentes directas;
- convierte eventos en operaciones del dominio.

Una regla posible seria:

```text
Solo FmcRuntimeThread modifica fmc_service.
```

### `UiThread`

Forma parte del runtime general del sistema, pero no del modulo
`fmc_runtime`.

Su responsabilidad es presentar informacion:

```text
Como se muestra este dato?
Que pantalla se actualiza?
Que texto se dibuja?
```

### `LoggerThread`

Tampoco pertenece a `fmc_runtime`. Se ocupa del almacenamiento de registros.

### Relacion entre los threads

```text
AcquisitionThread
        |
        | evento: pulse delta
        v
FmcRuntimeThread
        |
        | llamadas controladas
        v
    fmc_service
        |
        | snapshot
        +----------------+
        |                |
        v                v
    UiThread        LoggerThread
```

Todos forman parte del runtime general del firmware. Solo `FmcRuntimeThread`
implementa la responsabilidad arquitectonica de `fmc_runtime`.

## Threads que no pertenecen a `fmc_runtime`

Ejemplos:

- thread de UI;
- thread de adquisicion;
- thread Modbus;
- thread de escritura en Flash;
- thread de logging;
- thread de diagnostico;
- thread de actualizacion de firmware.

Por eso es mas preciso decir:

```text
Thread perteneciente a fmc_runtime
```

y no simplemente:

```text
Thread runtime
```

## Caso sin RTOS

Sin RTOS, `fmc_runtime` no necesita ser un thread. Puede ser un modulo llamado
periodicamente desde un superloop.

No todas las funciones llamadas dentro del `while (1)` pertenecen a
`fmc_runtime`.

### Adquisicion

Responsabilidades:

- leer informacion capturada;
- interpretar contadores o timestamps;
- calcular deltas;
- generar eventos.

### Runtime del producto

Responsabilidades:

- tomar eventos pendientes;
- decidir que operacion corresponde;
- ordenar las operaciones;
- llamar a `fmc_service`;
- mantener serializado el acceso al estado vivo.

### Presentacion

Responsabilidades:

- procesar entradas del usuario;
- seleccionar pantallas;
- formatear datos;
- actualizar el LCD.

### Persistencia

Responsabilidades:

- guardar estado en memoria no volatil;
- restaurar configuracion;
- validar versiones;
- verificar CRC;
- manejar escritura diferida.

### Bajo consumo o plataforma

Determina si el MCU puede entrar en un modo de bajo consumo.

La pertenencia a `fmc_runtime` no depende de aparecer dentro del `while (1)`,
sino de la responsabilidad ejecutada.

## Las interrupciones tambien participan

Sin RTOS, el funcionamiento del sistema no se limita al superloop.

Tambien participan las interrupciones:

```text
Interrupciones
      |
      | producen datos o flags
      v
Adquisicion
      |
      | genera eventos
      v
while (1)
      |
      +--> FMC_RUNTIME_Process()
      |
      +--> FMC_UI_Process()
      |
      +--> FMC_PERSISTENCE_Process()
      |
      +--> SYSTEM_TrySleep()
```

La ISR deberia hacer el trabajo minimo. El procesamiento del producto deberia
quedar fuera de la interrupcion siempre que sea posible.

## Ejemplo completo sin RTOS

En un firmware sin RTOS, una arquitectura posible es:

```text
HARDWARE_Init()
FMC_RUNTIME_Init()

while (1)
{
    FMC_RUNTIME_Process()
    FMC_UI_Process()
    FMC_COMMUNICATION_Process()
    SYSTEM_EnterSleepIfIdle()
}
```

En este caso:

```text
fmc_runtime no es un thread.

fmc_runtime es una maquina de estados o un despachador de eventos
ejecutado periodicamente desde el superloop.
```

## Las funciones puras no se convierten en runtime

Supongamos que `fmc_runtime` decide tomar un snapshot. Dentro de esa operacion,
`fmc_service` puede llamar a `fmc_volume` para calcular volumen visible.

`fmc_volume` se ejecuta mientras el firmware funciona, pero
arquitectonicamente sigue siendo un calculo puro.

No pasa a pertenecer a `fmc_runtime` por haber sido llamado desde alli.

```text
fmc_runtime
    Decide cuando se ejecuta una operacion.

fmc_service
    Controla la operacion sobre el estado vivo.

fmc_volume
    Realiza el calculo matematico.
```

Ejemplo conceptual:

```text
Runtime:
    "Llego un evento del producto."

Service:
    "Voy a modificar o copiar el estado vivo."

Volume:
    "Con estos pulsos y esta calibracion, el volumen es 25,4 L."
```

## Regla practica para identificar `fmc_runtime`

Una funcion o thread pertenece a `fmc_runtime` cuando responde principalmente a
estas preguntas:

```text
Que evento ocurrio?
Que operacion del producto corresponde ejecutar?
En que orden debe ejecutarse?
Quien puede modificar fmc_service?
Cuando se genera un snapshot?
A que consumidor debe entregarse?
```

No pertenece a `fmc_runtime` cuando responde principalmente a preguntas como:

```text
Como leo el timer?
Como convierto pulsos a litros?
Como dibujo el LCD?
Como escribo en Flash?
Como transmito por Modbus?
Como configuro el periferico?
```

Estas responsabilidades corresponden a:

```text
adquisicion / port
calculos puros
UI
persistencia
comunicacion
HAL / port / BSP
```

## Criterio de diseno

El criterio central es separar la coordinacion temporal de la implementacion
tecnica de cada responsabilidad.

```text
fmc_runtime
    Coordina cuando y en que orden.

fmc_service
    Protege las reglas del estado vivo.

fmc_model
    Define el estado canonico.

fmc_volume
    Ejecuta calculos puros.

acquisition
    Obtiene informacion del hardware.

UI
    Presenta informacion.

persistence
    Guarda y restaura informacion.

port / BSP
    Accede al hardware.
```

## Conclusion

`Runtime` no significa simplemente "codigo que esta corriendo".

En sentido arquitectonico:

> `fmc_runtime` es el codigo que coordina el comportamiento temporal del
> producto y controla el acceso a su estado vivo.

Con RTOS, puede implementarse como un thread propietario de `fmc_service`.

Sin RTOS, puede implementarse como una maquina de estados o un despachador de
eventos llamado desde el superloop.

Las demas funciones del `while (1)` forman parte de la ejecucion general del
firmware, pero no necesariamente pertenecen al modulo `fmc_runtime`.
