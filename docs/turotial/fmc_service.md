> Human reference only.
>
> This tutorial is not part of the active agent route, does not define the
> current workstream, and does not define the public API contract.
>
> For agent work, start with `AGENTS.md` and, when present,
> `WORKING_CONTEXT.md`, then follow the applicable roadmap and specifications.
>
> The authoritative API contract is `src/product/fmc/fmc_service.h`.
> Do not apply `docs/workflow/doc_closure.md` to delete or compress this file
> unless the user explicitly asks to curate human tutorial/reference material.

# Por que es conveniente implementar `fmc_service.c/h`

## Contexto

El modulo `fmc_service` fue planteado como una frontera arquitectonica central
antes de incorporar responsabilidades mas complejas como:

- ThreadX;
- bajo consumo;
- adquisicion de pulsos;
- persistencia;
- comunicacion;
- presentacion;
- autorizacion de operaciones.

Su funcion principal no es capturar pulsos, manejar interrupciones, dibujar la
pantalla ni guardar datos en memoria no volatil.

Su responsabilidad es mas especifica:

> Ser el propietario del estado vivo del computador de caudal y ofrecer
> operaciones controladas para modificarlo o consultarlo.

La arquitectura conceptual queda asi:

```text
Interrupcion / captura de pulsos
              |
              v
        Adquisicion
              |
              | delta de pulsos
              v
          Runtime
     serializacion / eventos
              |
              v
        fmc_service
       estado vivo unico
          |       |
          |       +--> fmc_volume: calculos puros
          |
          +----------> fmc_model: estado canonico
              |
              v
          Snapshot
      UI / comunicacion / log
```

## 1. Define quien es dueno del estado vivo

`fmc_model_t` es una estructura de datos copiable. El modelo describe que
informacion existe, pero no determina cual de todas las copias posibles
representa el estado real del equipo.

Podrian existir simultaneamente:

- configuracion activa;
- buffer de edicion;
- valores de fabrica;
- copia restaurada desde Flash;
- snapshot enviado a la pantalla;
- estado utilizado por un test.

`fmc_service_t` responde una pregunta arquitectonica importante:

> Cual de todos esos modelos es el estado vivo y autoritativo del equipo?

Aunque actualmente contenga solamente un `fmc_model_t`, agrega una propiedad
semantica importante: ownership.

Sin esta capa, probablemente se terminaria utilizando algo similar a:

```c
extern fmc_model_t g_fmc_model;
```

y multiples modulos modificarian directamente:

```c
g_fmc_model.acm.pulses += delta;
g_fmc_model.ttl.pulses += delta;
```

Esto puede funcionar al principio, pero con el crecimiento del firmware resulta
dificil determinar:

- que modulo puede modificar el estado;
- quien garantiza sus invariantes;
- quien sincroniza el acceso;
- que ocurre cuando se agrega persistencia;
- que ocurre cuando la UI lee mientras adquisicion actualiza;
- que funciones pueden ejecutarse desde una interrupcion.

Con `fmc_service`, la regla puede ser:

```text
El runtime posee una instancia de fmc_service_t.
Los demas modulos interactuan con ella mediante su contrato publico.
```

## 2. Centraliza las reglas de modificacion

`FMC_SERVICE_AddPulseDelta()` no se limita a sumar un numero. Implementa una
regla del producto:

> Cada pulso medido debe acumularse simultaneamente en ACM y TTL.

Los dos totalizadores reciben el mismo caudal medido. Su diferencia principal no
es la fuente de pulsos, sino la politica de puesta a cero.

La implementacion verifica primero que ninguno de los dos contadores pueda
desbordarse. Solo despues modifica ambos totalizadores.

Esto preserva una propiedad importante:

```text
O se actualizan ACM y TTL, o no se actualiza ninguno.
```

La capa `service` es un lugar apropiado para estas reglas porque no pertenecen a:

- la conversion de unidades;
- el hardware;
- la pantalla;
- el RTOS.

Son invariantes del estado vivo del producto.

## 3. Mantiene un unico estado canonico

El diseno guarda los pulsos como estado canonico.

El volumen visible no se almacena permanentemente. Se calcula a partir de:

```text
pulsos acumulados
configuracion de medicion
unidad activa
calibracion
```

Esto evita estados duplicados como:

```c
uint64_t pulses;
double volume_liters;
double volume_m3;
```

Si se almacenaran todas esas variables, tarde o temprano podrian dejar de
coincidir.

Por ejemplo, si la unidad activa cambia de litros a metros cubicos, no es
necesario modificar el total acumulado. Los pulsos permanecen iguales y el
volumen visible se vuelve a calcular.

```text
Estado canonico: 250 000 pulsos

Vista en litros:  2 500 L
Vista en m3:      2,5 m3
```

Existe un unico dato fisico acumulado y diferentes representaciones derivadas.

## 4. El snapshot entrega una vista coherente

La funcion conceptualmente mas importante es `FMC_SERVICE_GetSnapshot()`.

Primero copia el modelo y luego calcula los volumenes utilizando esa copia. Esto
significa que todos los valores del snapshot corresponden al mismo estado
logico:

```text
modelo copiado
ACM calculado desde ese modelo
TTL calculado desde ese modelo
```

No se calcula ACM leyendo una version del estado y TTL leyendo una version
posterior.

Ademas, el snapshot no contiene punteros hacia el servicio. Despues de retornarlo
la pantalla puede utilizar `snapshot` aunque el servicio continue actualizandose.

### Por que no conviene que la pantalla lea directamente el modelo

Supongamos esta secuencia:

```text
1. La UI lee ACM.
2. Entra una actualizacion de pulsos.
3. La UI lee TTL.
4. La UI calcula ambos volumenes.
```

La pantalla podria terminar mostrando datos tomados en instantes diferentes.

Con un snapshot:

```text
1. El runtime obtiene una copia coherente.
2. Libera el servicio.
3. La UI trabaja solamente con la copia.
```

Esto tambien reduce el tiempo durante el cual un futuro mutex o seccion critica
deberia permanecer tomado.

## 5. Ejemplo sencillo inventado

Imaginemos un pequeno surtidor con:

```text
Calibracion: 100 pulsos por litro
ACM: total de la operacion actual
TTL: total historico del equipo
```

### Inicializacion

```c
fmc_service_t service;

FMC_SERVICE_Init(&service);
```

Estado:

```text
ACM = 0 pulsos
TTL = 0 pulsos
```

### Primera entrega

La adquisicion detecta 250 pulsos desde la ultima actualizacion:

```c
FMC_SERVICE_AddPulseDelta(&service, 250);
```

Estado canonico:

```text
ACM = 250 pulsos
TTL = 250 pulsos
```

Snapshot visible:

```text
ACM = 2,5 L
TTL = 2,5 L
```

### Fin de la primera entrega

El operador puede poner a cero ACM:

```c
FMC_SERVICE_ResetTotal(&service, FMC_MODEL_TOTAL_ACM);
```

Estado:

```text
ACM = 0 pulsos
TTL = 250 pulsos
```

### Segunda entrega

Ingresan otros 100 pulsos:

```c
FMC_SERVICE_AddPulseDelta(&service, 100);
```

Estado:

```text
ACM = 100 pulsos
TTL = 350 pulsos
```

Snapshot:

```text
ACM = 1,0 L
TTL = 3,5 L
```

El codigo que captura pulsos no necesita conocer calibracion, litros, metros
cubicos, politica de reset, pantalla ni contrasena de servicio. Solamente
informa cuantos pulsos ingresaron desde la ultima actualizacion.

## 6. Separa politica de mecanismo

`FMC_SERVICE_ResetTotal()` puede resetear ACM o TTL, pero no verifica una
contrasena ni sabe que menu esta abierto.

Esto es correcto.

El modelo informa la politica mediante `FMC_MODEL_GetResetPolicy()`. El flujo
conceptual seria:

```text
UI solicita reset de TTL
        |
        v
Runtime consulta politica
        |
        v
UI valida contrasena o privilegios
        |
        v
Runtime llama FMC_SERVICE_ResetTotal()
```

El servicio implementa el mecanismo de reset. La UI o el runtime aplican la
autorizacion.

Si `fmc_service` conociera contrasenas, teclado o menus, quedaria acoplado a una
implementacion particular de interfaz.

## 7. Mantiene el codigo independiente de ThreadX

El servicio no contiene:

- mutex;
- colas;
- eventos;
- timers;
- llamadas HAL;
- esperas bloqueantes;
- memoria dinamica.

Mas adelante, ThreadX podria utilizar una arquitectura como:

```text
ISR de captura
    |
    v
cola de adquisicion
    |
    v
fmc_runtime thread
    |
    +--> FMC_SERVICE_AddPulseDelta()
    +--> FMC_SERVICE_ResetTotal()
    +--> FMC_SERVICE_GetSnapshot()
```

En ese esquema, el runtime seria el responsable de garantizar que un unico
contexto modifique el servicio.

Tambien podria utilizarse inicialmente sin RTOS:

```c
while (1)
{
    runtime_process_events();
    runtime_update_service();
    runtime_update_display();
}
```

`fmc_service` no tendria que cambiar.

## 8. Facilita tests precisos

El modulo puede probarse sin placa, pantalla ni generador de pulsos.

### Inicializacion

```c
FMC_SERVICE_Init(&service);
```

### Acumulacion

```c
status = FMC_SERVICE_AddPulseDelta(&service, 10);
```

### Overflow

```c
service.model.acm.pulses = UINT64_MAX;

status = FMC_SERVICE_AddPulseDelta(&service, 1);
```

### Independencia del snapshot

```c
FMC_SERVICE_GetSnapshot(&service, &snapshot_1);
FMC_SERVICE_AddPulseDelta(&service, 100);
```

## 9. No es solamente un wrapper innecesario

Actualmente podria parecer que `fmc_service` agrega poco codigo. Pero su valor
no esta en la cantidad de instrucciones. Esta en establecer:

- quien posee el estado;
- por donde se modifica;
- que invariantes se garantizan;
- como se obtiene una vista coherente;
- que responsabilidades quedan afuera;
- donde se conectara el runtime futuro.

Una buena frontera arquitectonica suele comenzar siendo pequena.

## Observaciones tecnicas sobre la implementacion actual

### 1. Sincronizacion externa

El servicio no es thread-safe.

En un STM32 de 32 bits, una lectura o escritura de `uint64_t` no debe asumirse
atomica. Por lo tanto, no seria seguro que:

```text
ISR actualiza pulsos
mientras
thread de UI toma snapshot
```

La solucion deberia estar en el runtime, por ejemplo:

- un unico thread propietario;
- mensajes con deltas de pulsos;
- mutex;
- seccion critica breve;
- intercambio de buffers.

No conviene agregar sincronizacion directamente dentro de `fmc_service`, porque
lo acoplaria prematuramente a un RTOS o a un modelo de ejecucion concreto.

### 2. El modelo sigue siendo publico

Actualmente puede hacerse:

```c
service.model.acm.pulses = 123;
```

Esto facilita tests y una futura restauracion del estado, pero tambien permite
saltarse las operaciones del servicio.

Cuando el contrato se estabilice, podrian evaluarse alternativas como un tipo
opaco o funciones especificas para restauracion y configuracion.

### 3. Salida parcialmente escrita en `GetSnapshot`

Actualmente se copia el modelo, se calcula ACM y luego TTL.

Si el calculo de TTL falla, el snapshot ya contiene el modelo y posiblemente el
volumen ACM.

No corrompe el servicio y el caller recibe un error, pero para una semantica
completamente transaccional podria utilizarse una variable local y copiarla a la
salida solamente cuando todos los calculos hayan finalizado correctamente.

## Conclusion

Implementar `fmc_service` es una buena decision porque crea una frontera
explicita alrededor del estado vivo del FMC-320U.

```text
fmc_model    = datos canonicos y semantica
fmc_volume   = calculos puros
fmc_service  = propiedad y operaciones sobre el estado vivo
fmc_runtime  = tiempo, eventos, concurrencia y RTOS
port / BSP   = hardware
UI           = presentacion y autorizacion
```

Para este firmware, que todavia debe integrar ThreadX, adquisicion,
persistencia y bajo consumo, el modulo no representa burocracia adicional.

Representa la frontera que evita que todas esas responsabilidades vuelvan a
mezclarse.

## Archivos relacionados

- `src/product/fmc/fmc_model.h`
- `src/product/fmc/fmc_model.c`
- `src/product/fmc/fmc_volume.h`
- `src/product/fmc/fmc_volume.c`
- `src/product/fmc/fmc_service.h`
- `src/product/fmc/fmc_service.c`
- `src/product/fmc/README.md`
- `docs/roadmaps/fmc_refactoring.md`
- `WORKING_CONTEXT.md`, when an active workstream exists
