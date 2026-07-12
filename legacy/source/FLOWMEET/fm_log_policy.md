# fm_log_policy — Especificación del módulo (v1.0)

**Objetivo**: Este documento define una especificación *clara y generativa* para que otra IA o desarrollador
pueda producir los archivos `fm_log_policy.h` y `fm_log_policy.c` sin ambigüedades.

---

## 1. Descripción general

`fm_log_policy` decide **cuándo** y **qué** eventos deben registrarse para representar el comportamiento del flujo
(caudal) con buena fidelidad, minimizando escrituras a Flash y consumo.
El módulo **no** accede a la memoria Flash ni a buffers: solo devuelve **acciones** que el módulo `fm_log`
convertirá en registros concretos.

**Eventos que la política habilita**:
- Doble evento al **arranque de flujo**: un punto “cero” en `t-1s` y un punto **actual** en `t`.
- **Variaciones significativas** durante flujo (**ON**): si la variación supera un umbral en ppm (por defecto 2%). 
- **Parada de flujo** (**STOP**).
- **Heartbeat** periódico (por defecto, cada 60 minutos).

**Créditos/cuotas**: La política usa un sistema de créditos para limitar frecuencia de registros (máx 10).
Se recargan a razón de +1/min con flujo activo, y opcionalmente +1/min en pausa (herencia) hasta el tope.

---

## 2. Lista de funciones públicas (prototipos y descripción)

> Todas las funciones públicas utilizan el prefijo del módulo en MAYÚSCULAS + CamelCase, p.ej. `FM_LOG_POLICY_*`.

```c
// Inicializa el estado interno y parámetros de la política.
void FM_LOG_POLICY_Init(uint32_t boot_unix,
                        uint16_t reset_count,
                        const fm_log_policy_cfg_t *cfg_opt);
Parámetros:

boot_unix: timestamp UNIX (segundos) del arranque del sistema.

reset_count: contador de resets (informativo para capas superiores).

cfg_opt: puntero a configuración opcional (NULL → usar defaults).

Comportamiento: carga valores por defecto, setea contadores y arranca heartbeat alineado a boot_unix.

c
Copy code
// Evalúa la política para una muestra de entrada y devuelve UNA acción priorizada (o NONE).
fm_log_action_t FM_LOG_POLICY_Step(const fm_log_policy_inputs_t *in);
Parámetros: in contiene el tiempo actual, estado de caudal y caudal escalado (milli).

Retorno: una acción fm_log_action_t. La app puede invocar Step() repetidamente en el mismo instante
para drenar acciones múltiples (p. ej., TO_ON: primero ZERO_T_MINUS_1S, luego CURRENT).

c
Copy code
// Determina si la variación relativa supera un umbral en ppm (por defecto 20.000 ppm = 2%).
bool FM_LOG_POLICY_ShouldRecordVariation(uint32_t rate_prev_milli,
                                         uint32_t rate_now_milli,
                                         uint32_t variation_ppm);
Uso: utilitario para la capa superior al decidir si empuja una muestra en ON.

c
Copy code
// Telemetría y pruebas
uint8_t  FM_LOG_POLICY_GetCredits(void);
uint32_t FM_LOG_POLICY_GetLastHeartbeat(void);
void     FM_LOG_POLICY_ForceHeartbeatTick(uint32_t now_unix);
GetCredits: créditos disponibles (0..max).

GetLastHeartbeat: último timestamp de heartbeat.

ForceHeartbeatTick: fuerza que el próximo Step() dispare heartbeat si corresponde (testing).

3. Estructuras de datos necesarias
3.1 Tipos públicos
c
Copy code
// Entrada de evaluación por tick o evento de la app
typedef struct
{
    uint32_t          now_unix;     // timestamp actual (s)
    fmx_rate_status_t rate_state;   // OFF, TO_ON, ON, TO_OFF
    uint32_t          rate_milli;   // caudal * 1000 (opcional para variación)
} fm_log_policy_inputs_t;
c
Copy code
// Acciones que la política puede solicitar
typedef enum
{
    FM_LOG_ACT_NONE = 0,
    FM_LOG_ACT_PUSH_ZERO_T_MINUS_1S, // punto “0” en t-1s
    FM_LOG_ACT_PUSH_CURRENT,         // punto actual
    FM_LOG_ACT_PUSH_STOP,            // evento de parada
    FM_LOG_ACT_PUSH_HEARTBEAT        // marca de vida
} fm_log_action_t;
c
Copy code
// Configuración opcional (puede sobrescribir defaults)
typedef struct
{
    uint8_t   max_credits;     // default: 10
    uint32_t  heartbeat_sec;   // default: 3600 (60 min)
    uint32_t  variation_ppm;   // default: 20000 (2 %)
} fm_log_policy_cfg_t;
3.2 Estado interno (privado en .c)
No exponer en el header público; se incluye aquí para claridad de implementación.

c
Copy code
typedef struct
{
    uint8_t   credits;               // 0..max_credits
    uint32_t  last_flow_unix;        // última vez con flujo (para recarga)
    uint32_t  last_pause_unix;       // última pausa (para herencia)
    uint32_t  last_heartbeat_unix;   // marca de heartbeat
    uint32_t  last_rate_milli;       // cache para variación en ON
    bool      boot_seen;             // primer ciclo tras boot
    bool      flow_active;           // sesión ON abierta
} fm_log_policy_state_t;
4. Requisitos de inicialización y uso
Inicialización

Llamar a FM_LOG_POLICY_Init(boot_unix, reset_count, cfg_opt) durante el arranque de la app.

Defaults de compilación (valores sugeridos):

c
Copy code
#define FM_LOG_POLICY_MAX_CREDITS      (10u)
#define FM_LOG_POLICY_HEARTBEAT_SEC    (60u * 60u)   // 3600 s
#define FM_LOG_POLICY_VARIATION_PPM    (20000u)      // 2 %
cfg_opt == NULL → usar los defaults anteriores.

Se recomienda guardar boot_unix para alinear el primer heartbeat.

Ciclo de uso

En tu lazo principal o tarea, construir fm_log_policy_inputs_t in con:

now_unix (segundos desde RTC o epoch),

rate_state (OFF/TO_ON/ON/TO_OFF),

rate_milli (caudal × 1000), si se evalúa variación.

Invocar FM_LOG_POLICY_Step(&in) en bucle hasta que devuelva FM_LOG_ACT_NONE.

Por cada acción devuelta, el módulo consumidor (fm_log) debe:

Mapear la acción a un evento concreto y empujarlo al buffer/flash.

Ejemplos de mapeo:

PUSH_ZERO_T_MINUS_1S → evento con ts=now-1, rate=0.

PUSH_CURRENT → evento con ts=now, rate=rate_now.

PUSH_STOP → evento STOP con ts=now.

PUSH_HEARTBEAT → evento HEARTBEAT con ts=now.

Créditos

Tope: max_credits (default 10).

Recarga con flujo: +1 cada minuto en ON (en peldaños por cada 60 s).

Herencia en pausa (opcional y recomendado): +1 cada minuto en OFF hasta tope.

Variación significativa: consume 1 crédito al registrar PUSH_CURRENT en ON.

Arranque (TO_ON): idealmente consume 2 créditos (si hay), pero los dos eventos deben emitirse incluso sin créditos para no perder la forma de arranque (opción recomendada).

STOP: se registra siempre; no descuenta crédito (recomendado).

Variación (ppm)

Criterio: abs(now-prev)*1e6 / max(prev,1) >= variation_ppm.

Default: variation_ppm = 20000 (2 %).

Heartbeat

Periodicidad fija (heartbeat_sec, default 3600 s).

Independiente de créditos y del estado de caudal.

Prioridad baja frente a otras acciones del mismo tick.

Prioridad de acciones (en un mismo instante)

PUSH_ZERO_T_MINUS_1S (si TO_ON)

PUSH_CURRENT (segundo paso de TO_ON, o variación en ON)

PUSH_STOP

PUSH_HEARTBEAT

NONE

5. Dependencias con otros módulos
fmx: define fmx_rate_status_t con los estados OFF, TO_ON, ON, TO_OFF.

Recomendado: exponer un header fmx.h con ese enum público.

fm_log: consumidor de las acciones entregadas por la política.

Recibe acciones y construye fm_log_event_t con timestamp y payload (caudal).

Posee el buffer RAM y la lógica para escribir en Flash (vía fm_flash).

fm_flash: primitivas de escritura/borrado de Flash (no usado directamente por fm_log_policy).

Importante: fm_log_policy no debe incluir headers de HAL/Flash ni tocar registros.
Su dependencia es lógica con fmx y fm_log para tipos y coordinación.

6. Reglas de estilo (resumen mínimo para generación)
Públicas: FM_LOG_POLICY_* (prefijo en MAYÚSCULAS + CamelCase).

Tipos públicos: fm_log_policy_*_t (minúsculas + _t).

Macros: FM_LOG_POLICY_* en MAYÚSCULAS con sufijos de unidad (_SEC, _PPM, etc.).

Privadas: static + snake_case y opcional _ final, p.ej. policy_update_credits_().

Sin bloqueos en Step(); complejidad O(1) amortizada.

No usar en ISR; si se requiere, postear evento a la tarea.

7. Anexos (ejemplos de integración)
Bucle típico (pseudocódigo en la app):

c
Copy code
void App_LogTick(void)
{
    fm_log_policy_inputs_t in = {
        .now_unix   = RTC_GetUnix(),
        .rate_state = FMX_RATE_ON,      // según detección de flujo
        .rate_milli = get_flow_milli(), // si aplica
    };

    for (;;)
    {
        fm_log_action_t act = FM_LOG_POLICY_Step(&in);
        if (act == FM_LOG_ACT_NONE) break;

        switch (act)
        {
        case FM_LOG_ACT_PUSH_ZERO_T_MINUS_1S: fm_log_push_zero(in.now_unix - 1); break;
        case FM_LOG_ACT_PUSH_CURRENT:         fm_log_push_rate(in.now_unix, in.rate_milli); break;
        case FM_LOG_ACT_PUSH_STOP:            fm_log_push_stop(in.now_unix); break;
        case FM_LOG_ACT_PUSH_HEARTBEAT:       fm_log_push_heartbeat(in.now_unix); break;
        default: break;
        }
    }
}