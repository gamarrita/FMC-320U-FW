/**
 * @file fmx.c
 * @brief Runtime principal y orquestacion de eventos de FMX.
 *
 * Administra recursos de ThreadX, la cola de eventos y el estado de caudal
 * para la aplicacion FLOWMEET.
 * @details Gestiona contingencias y traza REQ-FMX-INIT-001.
 * Actua como runtime equivalente a main.c y coordina temporizadores ThreadX.
 */

// --- Includes ---
#include "fmx.h"
#include "lptim.h"
#include "fm_debug.h"
#include "fm_fmc.h"
#include "fmx_lp.h"
#include "fm_lcd.h"
#include "fm_user.h"
#include "fm_setup.h"
#include "fm_log.h"
#include "fm_mxc.h"
#include "fm_cmd.h"
#include "fm_usart.h"
#include "tx_api.h"

// --- Defines ---
// Temporizadores y limites se alinean con REQ-FMX-TIMER-001.
// Cada valor respeta la granularidad de ThreadX.
#define TRUE                  (1u)
#define FALSE                 (0u)
// Mantener multiplo de 4 para alinear la cola con palabras ULONG.
#define QUEUE_EVENT_SIZE      (4u)
#define TIMER_BACKLIGHT_INIT  (1000u)
#define TIMER_BACKLIGHT_GUI   (500u)
#define TIMER_EXTI_DEBOUNCE   (25u)
#define FMX_DEBUG_LOCAL
#define TICKS_PER_SECOND   TX_TIMER_TICKS_PER_SECOND

// --- Globals ---
// Contador global mantiene vivo el refresco de la UI.
// Los modulos piden refrescos en ms (1000 ms equivale a 1 Hz).
ULONG global_menu_refresh = 0;

// --- Static Data ---
// Estado privado coordina ISR y temporizadores ThreadX.

// Mantiene el aislamiento de rebotes.
static uint8_t key_ext_debounce_flag = 0;
static volatile uint16_t lptim3_last_capture;
static volatile uint16_t lptim4_last_capture;
static volatile uint8_t rate_capture_valid = FALSE;
static TX_QUEUE event_queue;
// Buffer dimensionado en ULONG simplifica conversiones y mantiene la latencia.
static uint32_t queue_storage_event[QUEUE_EVENT_SIZE];
// Semaforo gestiona el traspaso entre la ISR y la tarea BT sin espera activa.
static TX_SEMAPHORE bluetooth_slave_semaphore;
static TX_THREAD main_thread;
static TX_TIMER key_long_timer;
static TX_TIMER backlight_off_timer;
static TX_TIMER debounce_timer;
static TX_THREAD bluetooth_slave_thread;
// El keypad dispara la accion primaria al liberar la tecla.
// Las variantes long press ejecutan una accion secundaria tras 3 s.
// Las flags evitan repetir la accion primaria cuando ya hubo long press.
uint8_t key_up_skip_next = FALSE;
uint8_t key_down_skip_next = FALSE;
uint8_t key_enter_skip_next = FALSE;
uint8_t key_esc_skip_next = FALSE;

fmx_ack_t fmx_rate_status = FMX_ACK_RATE_OFF;


// --- Static Prototypes ---
// Normaliza contadores de caudal antes de refrescar la GUI.
static void PulseUpdate(void);
// Temporizador de backlight evita parpadeos notables.
static void TimerEntryBackLightOff(ULONG timer_key);
// Temporizador de rebote cumple REQ-FMX-DEBOUNCE-003.
static void TimerEntryDebounce(ULONG timer_key);
// Temporizador de long press evita bloquear otras tareas.
static void TimerEntryKeyThreeSeconds(ULONG timer_key);
// Hilo principal coordina el estado de caudal y las colas ThreadX.
static void ThreadEntryMain(ULONG thread_input);

// --- Public API ---

/**
 * @brief Inicializa las tareas nucleo de FMX y sus colas.
 * @param memory_ptr Pool de bytes provisto por el arranque de ThreadX.
 * @return TX_SUCCESS o un codigo de error ThreadX.
 * @details Verifica cada asignacion, establece recursos deterministas y aplica
 * un fail-safe ante cualquier error.
 */
UINT FMX_Init(VOID *memory_ptr)
{
    UINT ret_status = TX_SUCCESS;
    CHAR *pointer;
    TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*) memory_ptr;

    // Verifica la memoria dinamica antes de crear los hilos.
    ret_status = tx_byte_allocate(byte_pool,
                                (VOID**)&pointer,
                                FMX_STACK_SIZE,
                                TX_NO_WAIT);
    // Ante un error critico se aplica un fail-safe inmediato.
    if (ret_status != TX_SUCCESS) {
        __disable_irq();
        FM_DEBUG_LedError(1);
        while (1);
    }

    ret_status = tx_thread_create(&main_thread,
                                  "MAIN_THREAD",
                                  ThreadEntryMain,
                                  0,
                                  pointer,
                                  FMX_STACK_SIZE,
                                  FMX_THREAD_PRIORITY_10,
                                  FMX_THRESHOLD_10,
                                  FMX_SLICE_0,
                                  TX_AUTO_START);
    if (ret_status != TX_SUCCESS) {
        __disable_irq();
        FM_DEBUG_LedError(1);
        while (1);
    }

    ret_status = tx_queue_create(&event_queue,
                                 "EVENT_QUEUE",
                                 1,
                                 queue_storage_event,
                                 sizeof(queue_storage_event));
    if (ret_status != TX_SUCCESS) {
        FM_DEBUG_LedError(1);
        while (1);
    }

    ret_status = tx_timer_create(&key_long_timer,
                                 "KEY_LONG_TIMER",
                                 TimerEntryKeyThreeSeconds,
                                 0x1234,
                                 300,
                                 100,
                                 TX_NO_ACTIVATE);
    if (ret_status != TX_SUCCESS) {
        FM_DEBUG_LedError(1);
        return TX_TIMER_ERROR;
        while (1);
    }

    ret_status = tx_timer_create(&backlight_off_timer,
                                 "BACKLIGHT_TIMER",
                                 TimerEntryBackLightOff,
                                 0x1234,
                                 TIMER_BACKLIGHT_INIT,
                                 0,
                                 TX_AUTO_ACTIVATE);
    if (ret_status != TX_SUCCESS) {
        FM_DEBUG_LedError(1);
        return TX_TIMER_ERROR;
        while (1);
    }

    ret_status = tx_timer_create(&debounce_timer,
                                 "DEBUNCE_TIMER",
                                 TimerEntryDebounce,
                                 0x1234,
                                 TIMER_EXTI_DEBOUNCE,
                                 TIMER_EXTI_DEBOUNCE,
                                 TX_NO_ACTIVATE);
    if (ret_status != TX_SUCCESS) {
        FM_DEBUG_LedError(1);
        return TX_TIMER_ERROR;
        while (1);
    }

    ret_status = tx_semaphore_create(&bluetooth_slave_semaphore,
                                     "BT_SLAVE_SEMAPHORE",
                                     0);
    FM_CMD_RtosInit(memory_ptr);
    FM_USART_RtosInit(memory_ptr);

    ret_status = tx_byte_allocate(byte_pool,
                                (VOID**)&pointer,
                                FMX_STACK_SIZE,
                                TX_NO_WAIT);
    if (ret_status != TX_SUCCESS) {
        __disable_irq();
        FM_DEBUG_LedError(1);
        while (1);
    }

    ret_status = tx_thread_create(&bluetooth_slave_thread,
                                  "BT_SLAVE",
                                  FM_USER_ThreadEntryBluetoothSlave,
                                  (ULONG)&bluetooth_slave_semaphore,
                                  pointer,
                                  FMX_STACK_SIZE,
                                  FMX_THREAD_PRIORITY_10,
                                  FMX_THRESHOLD_10,
                                  FMX_SLICE_0,
                                  TX_AUTO_START);
    if (ret_status != TX_SUCCESS) {
        __disable_irq();
        FM_DEBUG_LedError(1);
        while (1);
    }

    return ret_status;
}

/**
 * @brief Obliga a encender el backlight del LCD mientras hay interaccion.
 * @details Mantiene el backlight activo reprogramando el temporizador.
 * Evita parpadeos durante la interaccion del usuario.
 */
void FMX_LcdBackLightOn(void)
{
    HAL_GPIO_WritePin(LED_BACKLIGHT_GPIO_Port,
                          LED_BACKLIGHT_Pin,
                          GPIO_PIN_RESET);
    tx_timer_deactivate(&backlight_off_timer);
    tx_timer_change(&backlight_off_timer,
                    TIMER_BACKLIGHT_GUI,
                    TIMER_BACKLIGHT_GUI);
    tx_timer_activate(&backlight_off_timer);
}

/**
 * @brief Garantiza la presencia de un evento de refresco en la cola.
 * @details Mantiene al menos un refresco para cumplir REQ-FMX-GUI-002.
 * Evita inanicion de la interfaz.
 */
void FMX_RefreshEventTrue(void)
{
    ULONG event_new;
    if (event_queue.tx_queue_enqueued == 0) {
        event_new = FMX_EVENT_MENU_REFRESH;
        tx_queue_send(&event_queue, &event_new, TX_NO_WAIT);
    }
}

/**
 * @brief Despierta el hilo que gestiona la conexion Bluetooth esclava.
 * @details Usa un semaforo para reactivar el hilo sin hacer polling.
 */
void FMX_Trigger_BluetoothSlave(void)
{
    tx_semaphore_put(&bluetooth_slave_semaphore);
}

// --- Static Functions ---

/**
 * @brief Actualiza pulsos y caudal antes de ejecutar los calculos.
 */
static void PulseUpdate(void)
{
    static uint8_t blink = 1;
    // Variables para totalizacion: siempre usan el contador vivo del sensor.
    static uint16_t vol_pulse_old;
    static uint16_t vol_pulse_new;
    static uint16_t vol_pulse_delta;
    // Variables para rate: solo avanzan con una captura consistente ISR/LPTIM3.
    static uint16_t rate_pulse_old;
    static uint16_t rate_pulse_new;
    static uint16_t rate_pulse_delta;
    static uint16_t rate_tick_old;
    static uint16_t rate_tick_new;
    static uint16_t rate_tick_delta;
    uint16_t rate_pulse_capture;
    uint16_t rate_tick_capture;
    uint8_t rate_capture_ready = FALSE;
    //
    static ULONG time_last;
    static ULONG time_now;

  	// Si no paso un segundo no se calcula nuevo caudal o volumen.
	time_now = tx_time_get();
  	if ((time_now - time_last) < TICKS_PER_SECOND)
    {
    	return;
    }
	time_last = time_now;

    // Pulsos del sensor primario para totalizacion y deteccion de flujo.
  	vol_pulse_old = vol_pulse_new;
  	vol_pulse_new = LPTIM4->CNT;
  	vol_pulse_delta = (vol_pulse_new - vol_pulse_old);

    // La ISR publica una ventana consistente: snapshot de tiempo (LPTIM3)
    // y de pulsos (LPTIM4) tomado sobre el mismo flanco.
    __disable_irq();
    if (rate_capture_valid) {
        rate_tick_capture = lptim3_last_capture;
        rate_pulse_capture = lptim4_last_capture;
        rate_capture_valid = FALSE;
        rate_capture_ready = TRUE;
    }
    __enable_irq();

    if (rate_capture_ready) {
        rate_pulse_new = rate_pulse_capture;
        rate_pulse_delta = (rate_pulse_new - rate_pulse_old);
        rate_pulse_old = rate_pulse_new;

        rate_tick_new = rate_tick_capture;
        rate_tick_delta = (rate_tick_new - rate_tick_old);
        rate_tick_old = rate_tick_new;
    }

    switch(fmx_rate_status)
    {
    case FMX_ACK_RATE_OFF:
    	if(vol_pulse_delta != 0)
    	{
    		fmx_rate_status = FMX_ACK_RATE_STARTED;
    	}
    	break;
    case FMX_ACK_RATE_ON:
    	if(vol_pulse_delta == 0)
		{
    		fmx_rate_status= FMX_ACK_RATE_STOPED;
		}
    	break;
    case FMX_ACK_RATE_STARTED:
    	if(vol_pulse_delta !=0)
    	{
    		fmx_rate_status = FMX_ACK_RATE_ON;
    	}
    	else
    	{
			// Esta condicion es "anomala", indica el caudal se inicio y detuvo "instantaneamente".
    		fmx_rate_status = FMX_ACK_RATE_STOPED;
    	}
    	break;
    case FMX_ACK_RATE_STOPED:
    	if(vol_pulse_delta == 0)
		{
			fmx_rate_status = FMX_ACK_RATE_OFF;
		}
		else
		{
			// Esta condicion es "anomala", indica el caudal se detuvo y reanudo "instantaneamente".
			fmx_rate_status = FMX_ACK_RATE_STARTED;
		}
		break;
    default:
    	// Aqui deberia capturar el error. Por ahora cambio a uno de los estados posibles.
    	fmx_rate_status = FMX_ACK_RATE_OFF;
    	break;
    }

    if (vol_pulse_delta) {
        FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_POINT, blink);
        blink ^= 1;
    } else {
        FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_POINT, 0);
    }

    // Estrategia de medicion en bajo consumo:
    // - LPTIM4 se lee en vivo para totalizacion y nunca dejar pulsos pendientes.
    // - LPTIM3 sigue marcando la base temporal del rate con snapshots en ISR.
    // - Cada ventana de rate queda asociada a un par consistente tiempo/pulsos.
    // Si no llego una nueva captura desde el ultimo ciclo, no se recalcula rate.
    __HAL_LPTIM_ENABLE_IT(&hlptim3, LPTIM_IT_CC1);


    FM_LOG_NewEvent(fmx_rate_status);

    FM_FMC_PulseAdd(vol_pulse_delta);
    FM_FMC_TtlCalc();
    FM_FMC_AcmCalc();

    if (rate_capture_ready && (rate_tick_delta > 1u)) {
        FM_FMC_CaptureSet(rate_pulse_delta, rate_tick_delta);
        FM_FMC_RateCalc();
    } else {
        FM_FMC_CaptureSet(0, 0);
        FM_FMC_RateClear();
    }

}

/**
 * @brief Callback del timer que apaga el backlight tras inactividad.
 * @param timer_input Parametro del timer sin uso.
 */
static void TimerEntryBackLightOff(ULONG timer_input)
{
    tx_timer_deactivate(&backlight_off_timer);
    HAL_GPIO_WritePin(LED_BACKLIGHT_GPIO_Port, LED_BACKLIGHT_Pin, GPIO_PIN_SET);
}

/**
 * @brief Callback del timer que detecta pulsaciones de 3 s (long press).
 * @param timer_input Parametro del timer sin uso.
 */
static void TimerEntryKeyThreeSeconds(ULONG timer_input)
{
    fmx_events_t event_new;

    if (HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port, KEY_DOWN_Pin)) {
        event_new = FMX_EVENT_KEY_DOWN_LONG;
        if (tx_queue_send(&event_queue, &event_new, TX_NO_WAIT) != TX_SUCCESS) {
            FM_DEBUG_LedError(1);
        }
        key_down_skip_next = TRUE;
    }

    if (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin)) {
        event_new = FMX_EVENT_KEY_UP_LONG;
        if (tx_queue_send(&event_queue, &event_new, TX_NO_WAIT) != TX_SUCCESS) {
            FM_DEBUG_LedError(1);
        }
        key_up_skip_next = TRUE;
    }

    if (HAL_GPIO_ReadPin(KEY_ESC_GPIO_Port, KEY_ESC_Pin)) {
        event_new = FMX_EVENT_KEY_ESC_LONG;
        if (tx_queue_send(&event_queue, &event_new, TX_NO_WAIT) != TX_SUCCESS) {
            FM_DEBUG_LedError(1);
        }
        key_esc_skip_next = TRUE;
    }

    if (HAL_GPIO_ReadPin(KEY_ENTER_GPIO_Port, KEY_ENTER_Pin)) {
        event_new = FMX_EVENT_KEY_ENTER_LONG;
        if (tx_queue_send(&event_queue, &event_new, TX_NO_WAIT) != TX_SUCCESS) {
            FM_DEBUG_LedError(1);
        }
        key_enter_skip_next = TRUE;
    }
}

/**
 * @brief Limpia la bandera de rebote para teclas externas.
 * @param timer_input Parametro del timer sin uso.
 */
static void TimerEntryDebounce(ULONG timer_input)
{
    key_ext_debounce_flag = 0;
    tx_timer_deactivate(&debounce_timer);
}

/**
 * @brief Punto de entrada del hilo principal para menus y eventos.
 * @param thread_input Parametro del hilo sin uso.
 */
static void ThreadEntryMain(ULONG thread_input)
{
    uint32_t	received_event;
    uint8_t 	menu = 0;
    uint8_t 	menu_change;
    UINT 		tx_status;
    ULONG 		sleep_time = 1000;

    HAL_GPIO_WritePin(LED_BACKLIGHT_GPIO_Port,
                          LED_BACKLIGHT_Pin,
                          GPIO_PIN_RESET);
    tx_timer_activate(&backlight_off_timer);

    do {
        tx_status = tx_queue_receive(&event_queue, &received_event, TX_NO_WAIT);
    } while (tx_status != TX_QUEUE_EMPTY);

    received_event = FMX_EVENT_MENU_REFRESH;

    for (;;) {
        sleep_time = 1000;
        PulseUpdate();

        if ((received_event >= FMX_EVENT_MENU_REFRESH) &&
            (received_event < FMX_EVENT_TIME_OUT)) {
            switch (menu) {
            case 0:
                menu_change = FM_USER_MenuNav(received_event);
                if (menu_change) {
                    menu_change = 0;
                    menu = 1;
                    FMX_RefreshEventTrue();
                }
                break;
            case 1:
                if (received_event == FMX_EVENT_EMPTY) {
                    received_event = FMX_EVENT_MENU_REFRESH;
                }
                menu_change = FM_SETUP_MenuNav(received_event);
                if (menu_change) {
                    menu_change = 0;
                    menu = 0;
                    FMX_RefreshEventTrue();
                }
                break;
            default:
                break;
            }
            received_event = FMX_EVENT_EMPTY;
        }

        if (global_menu_refresh) {
            sleep_time = global_menu_refresh;
        }

        FM_LCD_LL_Refresh();

        tx_status = tx_queue_receive(&event_queue,
                                     &received_event,
                                     sleep_time / 10);

        if (tx_status != TX_SUCCESS) {
            received_event = FMX_EVENT_MENU_REFRESH;
        }

        if ((received_event >= FMX_EVENT_KEY_DOWN) &&
            (received_event <= FMX_EVENT_KEY_EXT_2)) {
            FMX_LcdBackLightOn();
        }

        global_menu_refresh = 0;
    }
}

// --- Interrupts ---

/**
 * @brief Callback EXTI de flanco descendente para teclas y botones.
 * @param GPIO_Pin Pin que disparo la interrupcion.
 */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    uint32_t event_new;
    UNUSED(GPIO_Pin);
    tx_timer_deactivate(&key_long_timer);

    switch (GPIO_Pin) {
    case KEY_ENTER_Pin:
        if (key_enter_skip_next) {
            key_enter_skip_next = FALSE;
        } else {
            event_new = FMX_EVENT_KEY_ENTER;
            if (tx_queue_send(&event_queue,
                                 &event_new,
                                 TX_NO_WAIT) != TX_SUCCESS) {
                FM_DEBUG_LedError(1);
            }
        }
        break;
    case KEY_DOWN_Pin:
        if (key_down_skip_next) {
            key_down_skip_next = FALSE;
        } else {
            event_new = FMX_EVENT_KEY_DOWN;
            if (tx_queue_send(&event_queue,
                                 &event_new,
                                 TX_NO_WAIT) != TX_SUCCESS) {
                FM_DEBUG_LedError(1);
            }
        }
        break;
    case KEY_ESC_Pin:
        if (key_esc_skip_next) {
            key_esc_skip_next = FALSE;
        } else {
            event_new = FMX_EVENT_KEY_ESC;
            if (tx_queue_send(&event_queue,
                                 &event_new,
                                 TX_NO_WAIT) != TX_SUCCESS) {
                FM_DEBUG_LedError(1);
            }
        }
        break;
    case KEY_UP_Pin:
        if (key_up_skip_next) {
            key_up_skip_next = FALSE;
        } else {
            event_new = FMX_EVENT_KEY_UP;
            if (tx_queue_send(&event_queue,
                                 &event_new,
                                 TX_NO_WAIT) != TX_SUCCESS) {
                FM_DEBUG_LedError(1);
            }
        }
        break;
    case KEY_EXT_1_Pin:
        if (key_ext_debounce_flag) break;
        key_ext_debounce_flag = 1;
        tx_timer_activate(&debounce_timer);
        event_new = FMX_EVENT_KEY_EXT_1;
        if (tx_queue_send(&event_queue, &event_new, TX_NO_WAIT) != TX_SUCCESS) {
            FM_DEBUG_LedError(1);
        }
        break;
    case KEY_EXT_2_Pin:
        if (key_ext_debounce_flag) break;
        key_ext_debounce_flag = 1;
        tx_timer_activate(&debounce_timer);
        event_new = FMX_EVENT_KEY_EXT_2;
        if (tx_queue_send(&event_queue, &event_new, TX_NO_WAIT) != TX_SUCCESS) {
            FM_DEBUG_LedError(1);
        }
        break;
    default:
        FM_DEBUG_LedError(1);
        break;
    }
}

/**
 * @brief Callback EXTI de flanco ascendente para teclas (inicia long press).
 * @param GPIO_Pin Pin que disparo la interrupcion.
 */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    UNUSED(GPIO_Pin);
    // La tecla ejecuta la accion primaria al soltar; aqui armamos la variante long press.
    // tx_timer_change garantiza que el primer periodo sea de 3 s y luego repita cada 150 ms.
    tx_timer_change(&key_long_timer, 250, 150);
    tx_timer_activate(&key_long_timer);
}

/**
 * @brief Callback de captura de LPTIM para medir caudal.
 * @param hlptim Puntero al handle de LPTIM.
 * @details
 * El front-end del pickup conecta al canal 1 de LPTIM3 en modo captura.
 * El modulo se habilita ~cada segundo para esperar el siguiente flanco.
 * Cada captura entrega:
 * - los ticks del LPTIM3 (32.768 kHz) transcurridos durante la ventana.
 * - los pulsos acumulados en LPTIM4 para caudal y volumen.
 * Esta combinacion mantiene la precision (<0.01%) sin sacrificar bajo consumo.
 */
void HAL_LPTIM_IC_CaptureCallback(LPTIM_HandleTypeDef *hlptim)
{
	__HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CC1);
    lptim3_last_capture = LPTIM3->CCR1;
    lptim4_last_capture = LPTIM4->CNT;
    rate_capture_valid = TRUE;
}

/*** END OF FILE ***/






