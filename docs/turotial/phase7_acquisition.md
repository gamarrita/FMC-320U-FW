> Human reference only.
>
> This protected tutorial is not part of the active agent route, does not
> define a current workstream, and is not a product, hardware, or public API
> contract.
>
> For agent work, start with `AGENTS.md` and, when present,
> `WORKING_CONTEXT.md`. Then follow the applicable roadmap, product documents,
> technical specifications, and public headers.
>
> The authoritative Phase 7 contracts are
> `docs/specs/fmc/acquisition.md`,
> `docs/specs/fmc/frequency_observation.md`, the accepted product documents
> under `docs/product/fmc/`, and the implemented public headers.
>
> Do not apply `docs/workflow/doc_closure.md` to delete or compress this file
> unless the user explicitly asks to curate human tutorial/reference material.

# Phase 7: de los pulsos fisicos a TTL y RATE

## Proposito

Phase 7 reemplazo los valores provisorios de la pantalla TTL/RATE por una ruta
de adquisicion real. El punto mas importante no fue solamente "leer un
contador", sino separar responsabilidades que suelen mezclarse:

- acumular todos los pulsos;
- formar deltas sin perder el wrap de 16 bits;
- medir una ventana temporal;
- decidir si esa ventana es utilizable;
- calcular RATE;
- actualizar ACM y TTL exactamente una vez;
- presentar un snapshot coherente;
- conservar el funcionamiento durante Stop2.

Este documento explica esas ideas de manera didactica. No sustituye los
contratos.

## La idea central

Un mismo sensor fisico produce pulsos, pero el firmware formula dos preguntas
distintas:

```text
Pregunta de totalizacion:
    Cuantos pulsos nuevos se acumularon?

Pregunta de frecuencia:
    Cuantos pulsos ocurrieron durante cuanto tiempo?
```

La primera pregunta determina ACM y TTL. La segunda permite calcular RATE.

Aunque ambas observan LPTIM4, no comparten baseline ni necesitan leer el
contador en el mismo instante.

```text
                          LPTIM4
                    contador acumulativo
                            |
             +--------------+--------------+
             |                             |
             v                             v
      pulse_delta                 frequency_observation
    baseline de total               baseline de frecuencia
             |                       + timestamp LPTIM3
             v                             |
        pulse delta                        v
             |                    pulse delta + elapsed_us
             v                             |
       ACM y TTL                          RATE
```

Esta independencia es una proteccion: una frecuencia ausente, tardia o
invalida nunca debe impedir que los pulsos lleguen a los totalizadores.

## Terminologia

### Pulso

Una transicion aceptada de la entrada primaria. LPTIM4 acumula esas
transiciones.

### Contador acumulativo

Un registro que contiene la cantidad acumulada de pulsos desde que fue armado,
modulo su ancho numerico. LPTIM4 es un contador de 16 bits:

```text
0, 1, 2, ... 65534, 65535, 0, 1, ...
```

### Observacion

Una lectura confiable del estado fisico. Para totalizacion es una lectura
estable de LPTIM4. Para frecuencia es un par:

```text
{
    pulse_count,
    timestamp_us
}
```

### Baseline

La observacion anterior que un modulo conserva como referencia para la
siguiente resta.

No es un valor global compartido. Phase 7 usa, como minimo:

```text
baseline pulse_delta          = previous_count
baseline frecuencia           = previous_count + previous_timestamp_us
```

Modificar uno no modifica el otro.

### Delta

La diferencia entre la observacion actual y su baseline:

```text
delta = actual - anterior
```

Para un contador de 16 bits, la resta es modulo 65.536.

### Wrap

El paso del maximo valor del contador nuevamente a cero. No es un error por si
mismo.

Ejemplo:

```text
anterior = 65530
actual   =     9

delta = 15
```

### Ventana

El intervalo delimitado por dos muestras aceptadas por el observador de
frecuencia:

```text
baseline                                muestra actual
   |                                          |
   +------------- elapsed_us ----------------+
   +------------- pulse_delta ---------------+
```

La ventana pertenece a `frequency_observation`, no a ThreadX, LPTIM3,
`product/main` ni `fmc_rate`.

### Timestamp monotonico

Un tiempo que solamente avanza. Phase 7 entrega al observador un
`uint64_t timestamp_us`. Un timestamp igual o menor que el anterior viola este
contrato.

### Calidad

La clasificacion que acompana una observacion de frecuencia:

- `VALID`;
- `UNAVAILABLE`;
- `STALE`;
- `INVALID`.

Calidad y valor RATE viajan juntos. Un numero retenido no es actual cuando su
calidad no es `VALID`.

### RATE

Caudal calculado a partir de pulsos y tiempo:

```text
RATE = funcion(pulse_delta, elapsed_seconds, configuracion de medida)
```

`fmc_rate` es una funcion pura. No lee hardware, no conoce baselines y no
decide calidad.

### Snapshot

Una copia coherente por valor del estado que necesita otro modulo. La
presentacion recibe un snapshot; no recibe punteros hacia estado interno vivo.

### Owner

El componente responsable de modificar un estado o tomar una decision. Que un
modulo transporte un dato no lo vuelve propietario de su significado.

## Ruta arquitectonica completa

La composicion final queda asi:

```text
                         Hardware
            +----------------+----------------+
            |                                 |
            v                                 v
      LPTIM4 / pulsos                 LPTIM3 / tiempo
            |                                 |
            +---------- product/main ---------+
                            |
              lecturas fisicas y coordinacion
                            |
                            v
                fm_main_acquisition
                  |                   |
                  v                   v
            pulse_delta      frequency_observation
                  |                   |
                  v                   v
        PULSE_DELTA event     FREQUENCY_RESULT event
                  |                   |
                  +---------+---------+
                            v
                       fmc_runtime
                  |                   |
                  v                   v
             fmc_service           fmc_rate
             ACM + TTL              RATE
                  |                   |
                  +---------+---------+
                            v
                   snapshot coherente
                            |
                            v
                    fmc_presentation
                            |
                            v
                       LCD fisico
```

Las responsabilidades quedan acotadas:

| Componente | Hace | No hace |
|---|---|---|
| LPTIM4 | Cuenta pulsos fisicos | No calcula delta, volumen o RATE |
| LPTIM3 | Provee una base temporal fisica | No define la ventana |
| `product/main` | Lee hardware y ordena el ciclo | No posee baselines matematicos |
| `pulse_delta` | Forma un delta modulo 16 bits | No conoce tiempo ni frecuencia |
| `frequency_observation` | Posee ventana, baseline y calidad | No calcula RATE |
| `fmc_runtime` | Serializa eventos y posee estado RATE/service | No lee HAL ni LCD |
| `fmc_service` | Actualiza ACM y TTL | No adquiere pulsos |
| `fmc_rate` | Calcula RATE puro | No retiene estado |
| `fmc_presentation` | Compone el frame semantico | No calcula medida |

## Totalizacion: conservar cada pulso

El contrato autoritativo esta en
`docs/specs/fmc/acquisition.md` y `src/services/pulse_delta.h`.

### Baseline inicial

LPTIM4 y `pulse_delta` comienzan desde cero:

```text
previous_count = 0
```

Por eso la primera lectura puede producir un delta no nulo. Esos pulsos
ocurrieron despues de armar el contador y deben conservarse.

### Observacion normal

```text
baseline             lectura
   100 ---------------> 137

delta = 37
nuevo baseline = 137
```

### Observacion despues de wrap

```text
baseline                         lectura
 65530 ---- 65535 ---- 0 ----------> 9

delta = 15
nuevo baseline = 9
```

### Precondicion importante

Entre dos observaciones puede haber como maximo 65.535 pulsos reales. Una
vuelta completa o varias vueltas son indistinguibles de un delta modulo mas
pequeno.

### Entrega a ACM y TTL

`pulse_delta` no conoce totalizadores. Produce un numero y
`fm_main_acquisition` lo transporta como evento:

```text
FMC_RUNTIME_EVENT_PULSE_DELTA
```

`fmc_runtime` delega en `fmc_service`, que aplica el mismo delta exactamente
una vez:

```text
ACM += delta
TTL += delta
```

La diferencia entre ACM y TTL esta en sus permisos de reset, no en su fuente
de acumulacion.

## Frecuencia: observar pulsos dentro de tiempo real

El contrato autoritativo esta en
`docs/specs/fmc/frequency_observation.md` y
`src/services/frequency_observation.h`.

### Forma de una muestra

```text
frequency_observation_sample_t
    pulse_count   : uint16_t
    timestamp_us  : uint64_t monotonico
```

### Primera muestra

La primera muestra solamente crea baseline:

```text
t = 0
count = 200
quality = UNAVAILABLE
```

No hay una ventana hasta disponer de dos extremos.

### Rango temporal admitido

La ventana nominal es un segundo. Se acepta de manera inclusiva:

```text
900.000 us <= elapsed_us <= 1.100.000 us
```

La tolerancia admite variacion real del scheduler sin fingir que cada llamada
ocurrio exactamente al segundo.

### Muestra temprana

Una muestra antes de 900.000 us:

- no produce resultado;
- no cambia calidad;
- no avanza el baseline.

```text
baseline t=0
     |
     +---- muestra t=500 ms: temprana, sin resultado
     |
     +---- muestra t=1000 ms: ventana valida desde t=0
```

### Muestra valida

Una muestra dentro del rango:

- produce `VALID`;
- entrega `pulse_delta` y `elapsed_us`;
- se convierte en el nuevo baseline.

```text
t0=0 ms,    count=100
t1=1010 ms, count=201

pulse_delta = 101
elapsed_us  = 1.010.000
quality     = VALID
```

### Muestra tardia

Una muestra despues de 1.100.000 us no puede representar la ventana nominal:

- no produce una ventana valida;
- resincroniza el baseline con la muestra actual;
- produce `STALE` si anteriormente existio una ventana valida;
- produce `UNAVAILABLE` si todavia nunca existio una ventana valida.

```text
VALID anterior
     |
     +--------- 1,2 s ---------> STALE + nuevo baseline
                                      |
                                      +---- ~1 s ----> VALID recuperado
```

La resincronizacion evita que todas las ventanas siguientes queden
permanentemente tarde.

### Timestamp invalido

Un timestamp igual o decreciente produce `INVALID`:

```text
anterior = 5.000.000 us
actual   = 5.000.000 us  -> INVALID
actual   = 4.900.000 us  -> INVALID
```

El baseline de frecuencia se descarta. La recuperacion requiere:

```text
INVALID
   |
   +---- primera muestra confiable ---> UNAVAILABLE / nuevo baseline
   |
   +---- siguiente ventana valida ----> VALID
```

El baseline de `pulse_delta` permanece intacto durante esta recuperacion.

## Que significa cada calidad

| Calidad | Significado | Hay RATE actual? | Cambia ACM/TTL? |
|---|---|---:|---:|
| `VALID` | Existe una ventana completa utilizable | Si, incluso puede ser cero | No |
| `UNAVAILABLE` | Todavia no existe una ventana completa utilizable | No | No |
| `STALE` | Una ventana valida previa no fue renovada a tiempo | No | No |
| `INVALID` | La muestra fisica o temporal no es confiable | No | No |

### Cero valido

Una ventana admisible sin pulsos es valida:

```text
pulse_delta = 0
elapsed_us  = 1.000.000
quality     = VALID
RATE        = 0
```

Esto es distinto de `UNAVAILABLE`. Cero es un resultado de medida; unavailable
es ausencia de una ventana utilizable.

## Por que hay dos baselines independientes

Supongamos dos lecturas fisicas diferentes dentro del mismo ciclo:

```text
tiempo ------------------------------------------------------------>

lectura totalizacion       pulso       lectura frecuencia
        count=500             |              count=501
                              v
```

Ese pulso:

- entra ahora en la ventana de frecuencia;
- entra en el proximo delta de totalizacion.

No se pierde ni se duplica porque cada ruta compara contra su propio baseline:

```text
baseline totalizacion  ---> proxima lectura totalizacion
baseline frecuencia    ---> lectura frecuencia actual
```

Forzar una unica lectura compartida pareceria mas sincronizado, pero acoplaria
dos contratos que no necesitan compartir instante ni recuperacion.

## Realizacion fisica seleccionada

### LPTIM4

- contador acumulativo de pulsos primarios;
- 16 bits;
- continua contando en Stop2;
- lectura estable antes de entregar una observacion.

### LPTIM3

- contador libre alimentado por LSE;
- LSE de 32.768 Hz con prescaler 16;
- frecuencia de tick de 2.048 Hz;
- un tick representa aproximadamente 488,28125 us;
- wrap fisico de 16 bits aproximadamente cada 32 segundos;
- extension por software a un timestamp monotonicamente creciente.

No se agregaron para esta realizacion:

- captura sobre flanco;
- pin de entrada adicional;
- interrupcion de captura;
- DMA.

`product/main` debe observar LPTIM3 antes de que una vuelta completa sin
observacion vuelva ambiguo el tiempo. La futura inactividad indefinida debe
iniciar una nueva epoca temporal y un nuevo baseline de frecuencia.

### Seguimiento FREQ-1: medir entre flancos

La simplificacion anterior permitio cerrar la primera ruta real sin agregar
otra tecnica de hardware. No elimina la importancia de una medicion
edge-to-edge:

```text
pulso primario
     |
     +--------------------> LPTIM4_IN
     |                       cuenta todos los pulsos
     |
     +--------------------> LPTIM3 capture
                             registra tiempo en el flanco
```

Conceptualmente, dos capturas consecutivas permiten observar:

```text
flanco N                              flanco N+1
   |                                     |
   +----------- periodo medido ----------+
```

Esto elimina la fase arbitraria entre una lectura periodica y el flanco. Puede
ser especialmente util para medir periodo, mejorar una realizacion de baja
frecuencia o asociar count y tiempo al mismo evento fisico.

La tarea queda abierta como `FREQ-1` en el roadmap. Antes de implementarla se
debe decidir:

- si complementa o reemplaza al productor actual de ventanas;
- como se arma el primer baseline entre flancos;
- que significa ausencia de un siguiente flanco;
- rango de periodo y frecuencia;
- wrap del timestamp;
- perdida, overrun o incoherencia de captura;
- interrupcion, DMA y autonomous mode;
- comportamiento y consumo en Stop2;
- como se traducen esos casos a calidad.

La captura no debe convertirse en condicion para totalizar. LPTIM4 y
`pulse_delta` siguen conservando todos los pulsos aunque no exista una captura
de frecuencia utilizable.

Legacy demuestra que esta arquitectura fisica fue intentada y aporta evidencia
sobre captura coherente y posibles problemas STM32U575. No autoriza a copiar
su ISR, su aritmetica ni su comportamiento de 0,1 Hz sin una nueva decision y
validacion.

## Orden de un ciclo periodico

El ciclo de `product/main` protege primero la totalizacion:

```text
1. Leer LPTIM4 estable
2. Formar pulse_delta
3. Despachar PULSE_DELTA a runtime/service

4. Leer nuevamente LPTIM4 estable
5. Leer timestamp LPTIM3
6. Formar frequency_observation
7. Si existe resultado, despachar FREQUENCY_RESULT

8. Construir snapshot vivo
9. Refrescar TTL/RATE
10. Confirmar que la actualizacion fue presentada
```

El punto 4 es deliberadamente otra lectura. No existe un requisito de que los
dos observadores consuman el mismo valor de LPTIM4.

## Handoff de frecuencia a runtime

El observador produce:

```text
frequency_observation_result_t
    quality
    pulse_delta
    elapsed_us
```

El resultado viaja por valor en:

```text
FMC_RUNTIME_EVENT_FREQUENCY_RESULT
```

Runtime aplica estas reglas:

```text
VALID
  -> convierte elapsed_us a segundos
  -> llama FMC_RATE_Calc()
  -> confirma valor + calidad atomicamente

UNAVAILABLE / STALE / INVALID
  -> no llama FMC_RATE_Calc()
  -> actualiza la calidad segun contrato
  -> no toca ACM ni TTL
```

Un error de calculo RATE no se transforma silenciosamente en calidad
`INVALID`. `INVALID` describe la observacion fisica, no cualquier falla de
software posterior.

## Snapshot y presentacion

`product/main` toma por valor:

- snapshot de `fmc_service`;
- estado RATE de `fmc_runtime`.

Con ambos construye un `fmc_presentation_snapshot_t`. La presentacion decide:

```text
VALID                         -> RATE numerico
UNAVAILABLE / STALE / INVALID -> -------
```

La pantalla inicial viva usa litros por segundo:

```text
TTL   <valor> Lt
RATE  <valor> Lt/s
```

Los indicadores TTL, RATE, `Lt`, slash y segundo permanecen visibles tambien
cuando RATE muestra `-------`.

La entrada a TTL/RATE usa el snapshot vivo mas reciente en una sola operacion.
No se presenta primero un valor provisorio para corregirlo inmediatamente
despues.

## Startup de adquisicion

La secuencia conceptual es:

```text
Inicializar runtime
       |
Inicializar ambos observadores
       |
Arrancar LPTIM3
       |
Arrancar LPTIM4 desde cero
       |
Tomar count + timestamp iniciales
       |
Crear baseline de frecuencia -> UNAVAILABLE
       |
Iniciar presentacion y deadline periodico
```

El startup no espera que LPTIM4 sea distinto de cero. Esperar actividad
impediria iniciar correctamente a 0 Hz e introduciria una politica de
actividad que Phase 7 no selecciono.

## Stop2 y el alcance de Phase 7

Durante observacion activa, la tarea duerme aproximadamente un segundo.
ThreadX puede convertir ese idle en Stop2:

```text
tarea activa
    |
    +---- sleep ThreadX ----> idle ----> Stop2
                                      |
                                      +---- deadline ----> tarea activa
```

Mientras tanto:

- LPTIM4 conserva pulsos;
- LPTIM3 conserva tiempo;
- los baselines de software no se reinician.

La futura politica de dormir por tiempo indefinido despues de inactividad no
forma parte de esta implementacion. Su mecanismo de entrada y su fuente de
despertar deben seleccionarse en otro trabajo.

### Seguimiento paralelo LP-1

La caracterizacion PPK2 de la integracion combinada observo mas de un pico
activo alrededor de algunos deadlines, especialmente a 100 Hz. El codigo
funcional sigue conservando pulsos, tiempo, RATE y presentacion, pero el perfil
sugiere una interaccion pendiente entre:

- compare de LPTIM1;
- ajuste masivo de ticks ThreadX;
- SysTick final;
- nueva evaluacion de idle.

Esta observacion esta registrada como `LP-1` en
`docs/roadmaps/fmc_refactoring.md`.

`LP-1` no pertenece a los contratos de adquisicion y no bloquea Phase 8. Una
rama paralela debe reproducir y cerrar su propia decision sin adoptar el sesgo
aritmetico de legacy como autoridad.

## Por que la regresion precede a la integracion fisica

La regresion permite probar secuencias que son dificiles o lentas de producir
con hardware:

```text
baseline
early exacto
limite 900.000 us
limite 1.100.000 us
late
timestamp igual
timestamp decreciente
wrap 16 bits
errores de puntero
```

Tambien demuestra propiedades arquitectonicas:

```text
evento frecuencia  --X--> ACM/TTL
evento pulse delta --X--> baseline frecuencia
error RATE         --X--> commit parcial
```

Donde `--X-->` significa "no debe modificar".

Despues, la prueba fisica responde otras preguntas:

- el pin cuenta pulsos reales?;
- LPTIM3 sigue midiendo durante Stop2?;
- los wraps fisicos se extienden correctamente?;
- LCD y UART coinciden con el generador?;
- el consumo y el perfil temporal son observables?

La combinacion evita dos extremos:

- confiar solamente en mocks sin probar el hardware;
- intentar provocar cada borde contractual manualmente con instrumentos.

## Evidencia obtenida, no nuevos contratos

Phase 7 verifico, entre otros puntos:

- conteo y wrap de LPTIM4;
- frecuencia fisica a 0 Hz, 1 Hz, 100 Hz y 1 kHz;
- wraps de LPTIM3 y LPTIM4;
- secuencia combinada 100 -> 0 -> 100 Hz sin reset;
- igualdad y monotonia de ACM/TTL;
- RATE y LCD coherentes con el generador;
- ejecucion de la regresion completa en target;
- comportamiento silencioso y consumo medido con PPK2.

Los valores concretos de una sesion de medicion son evidencia. No reemplazan
los limites y significados definidos en los contratos.

## Errores conceptuales frecuentes

### "UNAVAILABLE es cero"

No. `UNAVAILABLE` significa que no existe una ventana completa utilizable.
Cero requiere una ventana `VALID` con `pulse_delta = 0`.

### "El observador de frecuencia decide cuando dormir"

No. Solamente consume muestras y clasifica ventanas.

### "fmc_rate puede revisar si la muestra llego tarde"

No. `fmc_rate` recibe una ventana ya admitida y calcula matematicamente.

### "Una falla de frecuencia puede frenar TTL"

No. Es justamente la dependencia que Phase 7 evita.

### "Ambas rutas deben leer el mismo count"

No. Deben conservar sus propios baselines y sus propias secuencias.

### "El wrap de 16 bits siempre puede reconstruirse"

No. Una vuelta completa entre observaciones es ambigua.

### "El tutorial es la especificacion"

No. El tutorial ayuda a formar un modelo mental. Ante una diferencia, ganan
las autoridades enumeradas al inicio.

## Recorrido recomendado por el codigo

Para estudiar la implementacion en orden:

1. `src/services/pulse_delta.h`
2. `src/services/frequency_observation.h`
3. `src/apps/product/main/fm_main_acquisition.h`
4. `src/product/fmc/fmc_runtime.h`
5. `src/product/fmc/fmc_service.h`
6. `src/product/fmc/fmc_rate.h`
7. `src/product/fmc/fmc_presentation.h`
8. `src/apps/product/main/fm_main.c`
9. `src/apps/tests/regression/fm_regression_test.c`

Para confirmar significado y limites:

1. `docs/specs/fmc/acquisition.md`
2. `docs/specs/fmc/frequency_observation.md`
3. `docs/product/fmc/requirements.md`
4. `docs/product/fmc/behavior.md`
5. `docs/product/fmc/user_interface.md`
6. `docs/roadmaps/fmc_refactoring.md`

La pregunta guia durante la lectura es:

```text
Quien observa?
Quien posee el baseline?
Quien decide calidad?
Quien modifica estado?
Quien calcula?
Quien presenta?
```

Si cada respuesta apunta a un componente distinto y acotado, la arquitectura
esta preservando la separacion lograda en Phase 7.
