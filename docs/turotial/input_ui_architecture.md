> Human reference only.
>
> This tutorial is not part of the active agent route, does not define the
> current workstream, and is not a product, hardware, or public API contract.
>
> For agent work, start with `AGENTS.md` and, when present,
> `WORKING_CONTEXT.md`, then follow the applicable roadmap, product documents,
> specifications, local READMEs, and implemented public headers.
>
> The durable Phase 8 architecture is owned by
> `docs/roadmaps/fmc_refactoring.md`. Visible behavior is owned by the product
> documents under `docs/product/fmc/`. Public implemented contracts remain in
> their headers.
>
> This file explains the architecture for humans. Examples of future
> configuration, Bluetooth, printing, or persistence illustrate boundaries;
> they do not approve those product behaviors or APIs.
>
> Do not apply `docs/workflow/doc_closure.md` to delete or compress this file
> unless the user explicitly asks to curate human tutorial/reference material.

# Arquitectura de entradas, UI y funciones del producto

## Proposito

Este tutorial explica como se conectan las piezas centrales que reciben una
accion fisica, interpretan la intencion del operador, actualizan la interfaz y
ejecutan una operacion sobre el equipo.

El objetivo arquitectonico no es solamente resolver las cinco pantallas de
Phase 8. La misma separacion debe permitir incorporar, cuando sus fases lo
autoricen:

- configuracion de medicion;
- RTC y calendario;
- persistencia en Backup SRAM y Flash;
- temperatura;
- descarga de registros;
- Bluetooth;
- impresion de tickets.

Esto no exige crear ahora todos esos modulos. La arquitectura establece
fronteras que permiten agregarlos sin convertir la UI o `fmc_runtime` en un
componente que conoce todo el firmware.

## Estado actual y arquitectura objetivo

Los diagramas principales de este tutorial describen la arquitectura Phase 8
aprobada. El HEAD actual todavia conserva una ruta transitoria:

```text
Estado implementado antes de 8B/8C

reconocedor
    |
    v
fmc_runtime_event_t INPUT
    |
    +--> fmc_presentation
    |
    +--> fmc_runtime conserva last_input de forma pasiva
```

Las siguientes slices cambian esa ruta de manera incremental:

```text
8B
  fmc_presentation -> fmc_ui
  UI pura, cinco estados, frames y requests

8C
  reconocedor -> fmc_input_event_t
  product-main -> fmc_ui
  eliminar INPUT y last_input del runtime
```

Por lo tanto:

- los headers actuales siguen describiendo el codigo implementado;
- el roadmap describe la arquitectura aprobada que deben alcanzar 8B y 8C;
- este tutorial explica principalmente ese destino y marca los ejemplos
  futuros como ilustrativos.

## El problema que se quiere evitar

En el firmware legacy, una misma funcion de menu puede:

- decidir la pantalla activa;
- interpretar una tecla;
- modificar una variable;
- guardar configuracion;
- escribir directamente el LCD;
- iniciar Bluetooth;
- imprimir;
- esperar a que finalice una operacion.

El resultado es una maquina de estados dificil de probar y extender:

```text
                    MENU LEGACY
                         |
       +---------+-------+-------+----------+
       |         |               |          |
       v         v               v          v
  navegacion   edicion          LCD      perifericos
       |         |               |          |
       +---------+-------+-------+----------+
                         |
                    estado global
```

El problema no es que exista un menu central. El problema es que ese menu es
simultaneamente propietario de responsabilidades que tienen reglas, tiempos y
fallos diferentes.

## Idea central

La arquitectura aprobada conserva un unico orden de ejecucion para la
interaccion, pero separa la interpretacion, la UI y los efectos:

```text
 Fuentes fisicas y temporales
 GPIO / EXTI / timers / workers futuros
                  |
                  v
       cola privada de product-main
                  |
                  v
        reconocedores de entrada
                  |
                  v
          fmc_input_event_t
          intencion del operador
                  |
                  v
               fmc_ui
       contexto visible y navegacion
                  |
                  v
            solicitud de UI
                  |
                  v
             product-main
       serializa, enruta y ejecuta
          +-------+--------+----------+
          |                |          |
          v                v          v
     fmc_runtime      RTC/config   BT/impresion
      medicion          futuros      futuros
          |                |          |
          +-------+--------+----------+
                  |
           snapshot / estado
                  |
                  v
               fmc_ui
                  |
          frame semantico
                  |
                  v
           adaptador de LCD
```

Hay una sola ruta ordenada, pero no un unico modulo que implemente todas las
responsabilidades.

## Terminologia

### Ownership o propiedad

Ownership no significa necesariamente que un modulo haya reservado la memoria
con `malloc` o que sea el unico que pueda leerla.

En esta arquitectura significa:

> El modulo propietario define las invariantes del estado y es el unico que
> puede autorizar sus cambios.

Por ejemplo:

```text
fmc_runtime posee ACM
    otros modulos pueden recibir un snapshot de ACM
    solo la operacion autorizada del runtime puede resetear ACM
```

### Runtime general

Es todo el firmware mientras esta ejecutandose: threads, callbacks,
interrupciones, timers y servicios.

### `fmc_runtime`

Es un modulo concreto del producto. En la direccion Phase 8 posee:

- estado de medicion;
- RATE y su calidad;
- totalizadores ACM y TTL mediante `fmc_service`;
- primitivas de medicion como reset autorizado de ACM.

No es sinonimo del runtime general y no es el bus universal del producto.

### Product-main

Es la composicion principal de la aplicacion. Posee:

- la cola principal;
- el orden de procesamiento;
- los timers ThreadX de la composicion;
- el enrutamiento entre UI y dominios;
- la ejecucion exactamente una vez de las solicitudes aceptadas.

Ser el propietario de la orquestacion no obliga a acumular toda la
implementacion en `fm_main.c`. Cuando una funcion concreta sea suficientemente
compleja, product-main puede delegarla en un controlador local de aplicacion
sin perder el ownership del orden y del enrutamiento.

### Evento fisico

Describe lo observado en hardware, por ejemplo:

```text
tecla ENTER, flanco rising
EXT_1, nivel bajo
timeout del temporizador de hold
```

Puede contener identidad de placa, flanco o informacion temporal. No debe
llegar directamente al dominio de producto.

### Evento de aplicacion

Es un mensaje privado de la composicion `product/main`. Permite serializar
fuentes diferentes en una misma cola:

```text
FM_MAIN_EVENT_KEYBOARD
FM_MAIN_EVENT_PERIODIC_REFRESH
FM_MAIN_EVENT_KEY_HOLD_TIMEOUT
```

Su forma actual de cuatro `ULONG` y la profundidad de la cola son detalles de
implementacion, no un contrato permanente para todas las fases futuras.

### Entrada semantica

`fmc_input_event_t` expresa que accion reconocida realizo el operador:

```text
{
    key:    FMC_INPUT_KEY_ENTER,
    action: FMC_INPUT_ACTION_LONG
}
```

No contiene:

- pin;
- polaridad;
- flanco;
- tiempo de debounce;
- contexto de ISR;
- resultado de Bluetooth;
- tick periodico;
- delta de pulsos.

Estos ultimos elementos no son entradas del operador aunque tambien sean
eventos en sentido general.

### Reconocedor

Transforma observaciones fisicas y temporales en una entrada semantica.

Ejemplos:

- combina press, release y timeout para producir SHORT o LONG;
- aplica debounce y rearme a un boton externo;
- descarta una secuencia anormal sin contaminar la UI con flancos.

El reconocedor conoce la politica temporal de la entrada, pero no sabe que
pantalla esta visible ni que hace ENTER en esa pantalla.

### Contexto visible

Es el lugar de la UI que determina que ve e interpreta actualmente el
operador. Puede ser:

- una vista temporal de startup;
- una pantalla normal;
- un subflujo de configuracion;
- una pantalla de progreso o error.

Phase 8 implementa startup y cinco estados de usuario. Eso no significa que el
contrato de UI quede limitado para siempre a cinco estados.

### Navegacion

Decide que contexto o pantalla esta activo:

```text
TTL_RATE -> ACM_RATE -> PRINT -> LOG_DOWNLOAD -> DATE_TIME
```

La navegacion no debe validar un factor K, escribir Flash o conectarse por
Bluetooth.

### Edicion

Es la interaccion local para construir un valor candidato:

- valor borrador;
- digito o campo seleccionado;
- incremento o decremento;
- avance de cursor;
- parpadeo o feedback;
- solicitud de confirmar o cancelar.

Edicion no significa modificar inmediatamente la configuracion activa.

### Borrador o candidato

Es una copia provisional que la UI puede cambiar sin alterar el
funcionamiento vigente:

```text
configuracion activa ----copia----> borrador de UI
       ^                                  |
       |                                  |
       +------ aplicar tras validar <-----+
```

Si el operador cancela o la validacion falla, la configuracion activa no debe
haber cambiado.

### Estado canonico o activo

Es la representacion autorizada que gobierna el comportamiento real del
equipo. Su servicio propietario mantiene sus invariantes.

Puede coexistir con:

- un borrador de UI;
- un snapshot de lectura;
- una copia preparada para persistencia;
- valores de fabrica.

Esas copias no se vuelven canonicas por existir.

### Solicitud, intencion o request de UI

Es la descripcion de un efecto que la UI quiere que product-main intente
ejecutar:

```text
RESET_ACM
APPLY_CONFIGURATION       ejemplo futuro
START_PRINT               ejemplo futuro
```

La UI emite una intencion; no ejecuta el efecto directamente.

Durante Phase 8, el vocabulario autorizado contiene solamente:

```c
typedef enum
{
    FMC_UI_REQUEST_NONE = 0,
    FMC_UI_REQUEST_RESET_ACM,
    FMC_UI_REQUEST_COUNT
} fmc_ui_request_kind_t;

typedef struct
{
    fmc_ui_request_kind_t kind;
} fmc_ui_request_t;
```

La estructura Phase 8 contiene exactamente `kind`. No incluye:

- union de payloads;
- campos reservados;
- callback;
- identificador de transaccion;
- datos de capacidades futuras.

El handler de entrada inicializa siempre `request.kind` a
`FMC_UI_REQUEST_NONE` antes de interpretar la accion. De ese modo, un no-op y
una salida temprana valida no dejan una solicitud anterior en la memoria del
caller.

Este conjunto acotado no declara que toda funcion futura deba caber en un enum
sin datos. Cuando aparezca una capacidad con valores candidatos, su fase debe
decidir deliberadamente si extiende o reemplaza el contrato con un payload
tipado. Phase 8 no crea ahora una union especulativa de todos los comandos
futuros.

### Snapshot

Es una copia coherente y de solo lectura del estado necesario para una
operacion o presentacion.

```text
estado vivo                 snapshot
ACM = 123.4  ----------->   ACM = 123.4
RATE = 8.2                  RATE = 8.2
calidad = VALID             calidad = VALID
```

La UI usa el snapshot para formar una vista. No obtiene permiso para modificar
el estado vivo.

### Frame semantico

Describe que debe verse sin expresar todavia registros, bits o segmentos
fisicos:

```text
fila superior: ACM 123.4
fila inferior: RATE 8.2
unidad: Lt/min
POINT: on
```

El adaptador de LCD convierte ese significado al mapa fisico del vidrio.

### Servicio de dominio

Es el propietario de una capacidad y de sus reglas:

```text
medicion       -> fmc_runtime / fmc_service
RTC            -> servicio futuro de RTC
configuracion  -> servicio futuro de configuracion
impresion      -> servicio/controlador futuro de impresion
Bluetooth      -> servicio futuro de comunicacion
```

No toda capacidad necesita un thread. Servicio describe responsabilidad, no
modelo de concurrencia.

### Controlador de aplicacion

Coordina una secuencia que cruza tiempo o varios servicios, por ejemplo:

```text
encender Bluetooth
     -> conectar impresora
     -> enviar ticket
     -> esperar resultado
     -> apagar transporte
```

Se crea cuando una funcion real lo justifica. No se agregan controladores
vacios para capacidades todavia diferidas.

### Capacidad vertical

Es una funcion incorporada de extremo a extremo:

```text
requisito
  -> dominio
  -> integracion
  -> UI necesaria
  -> persistencia necesaria
  -> pruebas
  -> evidencia en target
```

Agregar Bluetooth o impresion como capacidad vertical evita dejar mitad del
flujo escondido en un menu generico.

## Cinco vocabularios que no deben confundirse

```text
1. Evento fisico
   "PD3 produjo falling"
             |
             v
2. Evento privado de aplicacion
   "hay una observacion de EXT_1 para procesar"
             |
             v
3. Entrada semantica
   "el operador hizo EXT_1 SHORT"
             |
             v
4. Solicitud de UI
   "la UI solicita RESET_ACM"
             |
             v
5. Operacion de dominio
   "fmc_runtime resetea ACM"
```

No siempre aparecen los cinco. Una tecla DOWN que solo navega produce entrada
semantica y cambia la UI, pero no necesita una operacion de dominio.

De manera inversa, un delta periodico de pulsos actualiza medicion sin ser una
entrada del operador.

## Recorrido de una tecla mecanica

```text
IRQ observa press
       |
       v
publica evento KEYBOARD
       |
       v
product-main lo consume
       |
       v
reconocedor inicia timer LONG
       |
       +--------------------------+
       |                          |
       | release antes de LONG    | vence timeout LONG
       v                          v
    SHORT                       LONG
       |                          |
       +-------------+------------+
                     |
                     v
             fmc_input_event_t
                     |
                     v
                   fmc_ui
```

El timer pertenece a la composicion de aplicacion. `fmc_ui` solamente recibe
el resultado SHORT o LONG.

## Recorrido de un boton externo

Los botones externos Phase 8 son activos en bajo. Su accion ocurre en el press
aceptado y el release estable solo rearma:

```text
estado armado
    |
    | falling: press
    v
emitir SHORT y desarmar
    |
    | rising: posible release
    v
iniciar ventana estable de 100 ms
    |
    +-- falling antes de vencer --> cancelar candidato
    |
    +-- sigue liberado al vencer --> armar nuevamente
```

Cada boton tiene estado y timer independientes. Un boton mantenido bajo al
arranque no genera una accion fantasma.

La inicializacion tambien aplica debounce al nivel liberado:

```text
inicialmente liberado
        |
        v
esperar high estable durante 100 ms
        |
        v
armado

inicialmente presionado
        |
        v
permanecer desarmado
        |
        | rising: release
        v
esperar high estable durante 100 ms
        |
        v
armado
```

## Recorrido del backlight

El backlight es una consecuencia transversal de actividad fisica, no una
accion semantica de navegacion:

```text
press fisico valido
       |
       +--> reconocedor / accion semantica
       |
       +--> solicitud de backlight
                  |
                  v
          rearmar timer a 10 s
                  |
          TX_SUCCESS requerido
                  |
                  v
           encender backlight
```

Los objetos `TX_TIMER` de product-main son estaticos. Un error al crear,
cambiar, activar o desactivar el timer no representa una condicion operacional
normal ni falta de memoria para encender el backlight. Phase 8 lo trata como
error fatal de contrato mediante el panic comun de product-main; no agrega
alarma de usuario, retry o modo degradado propio.

La proteccion contra una expiracion antigua si es necesaria porque corresponde
a concurrencia temporal normal:

```text
intervalo anterior vence ----+
                             +--> no debe apagar un intervalo nuevo
nuevo press rearma ----------+
```

La politica global que en el futuro pueda convertir un panic en reset por
watchdog pertenece a `ROBUST-1`, no al controlador de backlight.

## Recorrido de una navegacion sin efecto de dominio

Ejemplo conceptual: DOWN SHORT desde TTL/RATE.

```text
DOWN SHORT
    |
    v
fmc_ui consulta contexto actual
    |
    v
cambia TTL_RATE -> ACM_RATE
    |
    v
forma frame ACM/RATE con snapshot fresco
    |
    v
product-main presenta mediante adaptador LCD
```

No hay razon para enviar esa entrada a `fmc_runtime`: el runtime no es
propietario de la pantalla activa.

La misma regla se aplica al navegar hacia PRINT, LOG_DOWNLOAD o DATE_TIME: el
placeholder intenta presentar su frame inmediatamente y no espera al siguiente
ciclo periodico. Ese intento no elimina la siguiente presentacion periodica.

## Recorrido del reset de ACM

```text
ENTER LONG o EXT_2 SHORT
             |
             v
       product-main
             |
             v
          fmc_ui
 contexto actual = ACM_RATE?
        |             |
       no            si
        |             |
      no-op      request RESET_ACM
                      |
                      v
                product-main
              consume una sola vez
                      |
                      v
                 fmc_runtime
                 resetea ACM
                      |
                      v
                snapshot fresco
                      |
                      v
             fmc_ui -> frame -> LCD
```

La UI autoriza el significado contextual. El runtime ejecuta la primitiva de
medicion. Product-main garantiza el orden y evita duplicar la solicitud.

## Navegacion y edicion son maquinas de estado distintas

Una UI de configuracion tiene, como minimo, dos niveles:

```text
Maquina de navegacion global
|
+-- USER_TTL_RATE
+-- USER_ACM_RATE
+-- CONFIGURATION
    |
    +-- editor PASSWORD
    +-- editor FACTOR_K
    +-- editor UNITS
    +-- editor DATE_TIME
```

Dentro de un editor puede existir otra maquina:

```text
FACTOR_K
|
+-- digito seleccionado = 0
+-- digito seleccionado = 1
+-- digito seleccionado = 2
+-- ...
```

La navegacion global decide que flujo esta activo. El editor activo interpreta
UP, DOWN y ENTER dentro de ese flujo.

Esto no obliga a que toda la logica viva en un unico archivo `fmc_ui.c`. Cuando
una fase futura demuestre complejidad suficiente, `fmc_ui` puede delegar el
estado local a un componente puro especializado.

## Ejemplo conceptual de edicion de factor K

Este ejemplo explica ownership; no decide las teclas ni la politica de
guardado del producto futuro.

```text
1. Entrada al editor

   servicio de configuracion
   factor activo = 141.700
             |
             | snapshot
             v
   editor UI
   borrador = 141.700

2. Interaccion

   UP / DOWN  -> modifica el digito del borrador
   ENTER      -> selecciona otro campo

   El factor activo sigue siendo 141.700.

3. Confirmacion conceptual

   editor UI
   candidato = 151.700
             |
             | request APPLY_CONFIGURATION
             v
   product-main
             |
             v
   servicio de configuracion
      valida rango
      valida consistencia
      aplica atomicamente o rechaza
             |
          resultado
             |
             v
           fmc_ui
      muestra aceptado o error
```

Persistir en Flash es otra responsabilidad. Aplicar al estado vivo y guardar
de forma no volatil pueden tener politicas y fallos distintos.

## Operaciones sincronas y asincronas

### Operacion sincrona corta

El reset de ACM puede ejecutarse durante el manejo de una solicitud:

```text
UI request -> product-main -> runtime -> resultado -> snapshot -> UI
```

### Operacion asincrona

Imprimir o abrir una conexion Bluetooth puede tardar y fallar:

```text
UI request START_PRINT
          |
          v
product-main / controlador de impresion
          |
          +--> inicia transporte
          |
          +--> retorna sin bloquear la UI

tiempo despues:

worker o servicio
    |
    | publica estado compacto
    v
cola de product-main
    |
    v
controlador actualiza PRINTING / DONE / ERROR
    |
    v
fmc_ui recibe estado y forma el frame
```

Un worker futuro no debe escribir directamente el LCD ni modificar la
navegacion. Tampoco debe disfrazar `PRINT_DONE` como una tecla.

## Por que `fmc_runtime` no debe ser el bus universal

Resulta tentador agregar todo a un unico enum:

```text
PULSE_DELTA
KEY_INPUT
RTC_TICK
FLASH_DONE
BLUETOOTH_CONNECTED
PRINT_ERROR
TEMPERATURE_RESULT
...
```

Eso parece simple al principio, pero obliga al runtime de medicion a conocer
dominios sin relacion entre si. Tambien hace crecer una union global y mezcla
reglas de error, tiempos y ownership incompatibles.

La regla es:

```text
fmc_runtime_event_t
    eventos y operaciones propios de medicion

fm_main_event_t
    mensajes privados necesarios para serializar la aplicacion

eventos o estados futuros
    contratos del dominio que realmente los necesita
```

Una cola compartida de aplicacion no implica un vocabulario de dominio
compartido.

## Como evitar que product-main se convierta en otro monolito

Product-main debe coordinar, no absorber todas las implementaciones.

```text
product-main
|
+-- recibe y ordena eventos
+-- llama reconocedores
+-- enruta entradas a UI
+-- ejecuta requests
+-- obtiene snapshots
+-- coordina presentacion
|
+-- puede delegar cuando exista una necesidad concreta
    |
    +-- controlador de impresion futuro
    +-- controlador de conexion futuro
    +-- coordinador de persistencia futuro
```

Una delegacion sigue siendo parte de la aplicacion. No mueve HAL, ThreadX o
Flash dentro de `fmc_ui`.

Indicadores de que una delegacion ya esta justificada:

- existe una maquina de estados asincrona;
- intervienen varios servicios;
- hay timeout, cancelacion, retry o recuperacion;
- las pruebas del flujo pueden aislarse con claridad;
- `fm_main.c` ya solo reenviaria eventos a esa misma responsabilidad.

No se crea un modulo solamente para anticipar una posibilidad.

## Datos que vuelven hacia la UI

No todo lo que llega a la UI es una tecla.

```text
Operador                         Producto
   |                                |
   | fmc_input_event_t              | snapshot / estado
   v                                v
                 fmc_ui
                   |
                   v
             frame semantico
```

Ejemplos de datos de producto:

- snapshot coherente de ACM, TTL y RATE;
- validez del RTC;
- estado de conexion;
- progreso o error de impresion;
- resultado de validacion de configuracion;
- calidad de una medicion de temperatura.

La API concreta para cada dato debe definirse cuando se implemente su
capacidad. No es necesario inventar ahora un `fmc_ui_event_t` que contenga
todas las posibilidades futuras.

## Coherencia de snapshots

Una pantalla que muestra ACM y RATE debe obtenerlos de una observacion
coherente:

```text
product-main procesa un evento
          |
          v
runtime queda en estado estable
          |
          v
se toma un snapshot
          |
          v
UI compone un frame completo
```

La UI no debe leer varios campos vivos en instantes arbitrarios mientras otro
contexto los modifica.

Para una funcion asincrona futura hay dos opciones validas, a decidir en su
fase:

- el resultado completo viaja por valor en un evento;
- el evento notifica un cambio y product-main obtiene un snapshot protegido
  del servicio propietario.

Se deben evitar punteros a datos temporales enviados por una cola sin una vida
util claramente definida.

## Fallos y limites de recuperacion

Cada propietario decide los fallos de su dominio:

```text
reconocedor
    secuencia fisica invalida -> descarta/reportar

fmc_ui
    input invalido o estado imposible -> error de contrato

fmc_runtime
    operacion de medicion invalida -> rechaza sin corromper estado

servicio de configuracion futuro
    candidato fuera de rango -> rechazo tipado

controlador Bluetooth futuro
    timeout -> estado recuperable para UI

adaptador LCD
    fallo de presentacion -> politica de presentacion
```

La UI puede mostrar un error, pero no debe inventar como recupera un servicio.
Product-main coordina la consecuencia transversal cuando sea necesario.

## Concurrencia

La regla principal es:

> IRQ, timers y workers producen observaciones; el owner loop de product-main
> decide su orden y ejecuta los efectos coordinados.

```text
ISR GPIO --------+
timer hold ------+
timer periodico -+--> cola --> owner loop --> cambios coordinados
worker futuro ---+
```

Esto reduce carreras sobre:

- pantalla activa;
- requests pendientes;
- snapshots presentados;
- estado de un flujo asincrono.

No significa que todo el firmware deba ejecutarse en un unico thread. Un
servicio puede necesitar un worker, pero su comunicacion con la UI pasa por
una frontera explicita y serializada.

## Fronteras de prueba

La separacion permite pruebas enfocadas:

```text
reconocedor
    entradas: edges + timeouts
    salidas: SHORT/LONG + acciones de timer

fmc_ui
    entradas: input semantico + snapshot/estado
    salidas: estado visible + frame + request

fmc_runtime
    entradas: eventos/operaciones de medicion
    salidas: estado y snapshots de medicion

product-main
    entradas: secuencia de eventos de aplicacion
    evidencia: orden, exactamente una ejecucion, enrutamiento

adaptador LCD
    entrada: frame semantico
    salida: operaciones sobre segmentos
```

Una prueba de navegacion no necesita ThreadX o LCD fisico. Una prueba de
debounce no necesita un runtime de medicion. Una prueba de reset verifica
ademas la integracion exactamente una vez.

## Que se implementa en Phase 8

```text
SI
  startup y cinco pantallas de usuario
  navegacion semantica
  reset contextual de ACM
  POINT del menu de usuario
  botones externos
  backlight
  snapshots y frames necesarios

NO
  editores de configuracion
  RTC funcional
  Bluetooth funcional
  impresion funcional
  logging funcional
  bus generico de comandos
  controladores especulativos
```

Los placeholders PRINT, LOG_DOWNLOAD y DATE_TIME reservan posiciones visibles.
No autorizan a iniciar sus funciones futuras.

Los tres usan la misma fila inferior semantica de siete posiciones:

```text
posiciones: [ ][ ][ ][ ][O][F][F]
frame:      "    OFF"
```

## Reglas para incorporar una capacidad futura

Antes de agregar una funcion:

1. Identificar su requisito y autoridad de producto.
2. Definir que estado canonico posee.
3. Decidir si necesita servicio, worker o controlador.
4. Definir que ve y puede solicitar la UI.
5. Mantener los borradores separados del estado activo.
6. Definir resultados, fallos, timeout y cancelacion.
7. Integrar sus eventos en product-main sin convertirlos en teclas.
8. Proporcionar snapshots coherentes.
9. Agregar persistencia solo si la capacidad la necesita.
10. Verificar el flujo vertical completo.

La pregunta guia es:

```text
Quien define la regla?
Quien posee el estado?
Quien ejecuta el efecto?
Quien solamente lo presenta?
```

Si las cuatro respuestas quedan escondidas dentro de una funcion de menu, la
separacion se ha perdido.

## Resumen visual

```text
HARDWARE
   |
   | edges / niveles
   v
BSP Y PORTS
   |
   | eventos fisicos
   v
PRODUCT-MAIN + RECONOCEDORES
   |
   | fmc_input_event_t
   v
FMC_UI
   | \
   |  \ frame semantico -----------------> ADAPTADOR LCD
   |
   | request
   v
PRODUCT-MAIN
   |
   +--> FMC_RUNTIME --------+
   +--> SERVICIO RTC -------+
   +--> CONFIGURACION ------+--> snapshots / estados --> FMC_UI
   +--> BLUETOOTH ----------+
   +--> IMPRESION ----------+

Regla:
  UI interpreta y presenta.
  Product-main ordena y coordina.
  Cada dominio valida y modifica su propio estado.
  Hardware y RTOS no atraviesan esas fronteras.
```

## Fuentes relacionadas

Autoridades y contratos:

- `AGENTS.md`;
- `WORKING_CONTEXT.md`;
- `docs/roadmaps/fmc_refactoring.md`;
- `docs/product/fmc/user_interface.md`;
- `docs/product/fmc/behavior.md`;
- `docs/product/fmc/requirements.md`;
- `src/product/fmc/fmc_input.h`;
- `src/product/fmc/fmc_runtime.h`;
- `src/product/fmc/fmc_service.h`;
- `src/apps/product/main/README.md`.

Evidencia historica, no autoridad:

- `legacy/analysis/fmc_user_navigation.md`;
- `legacy/source/FLOWMEET/fm_user.c`;
- `legacy/source/FLOWMEET/fm_setup.c`;
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`.
