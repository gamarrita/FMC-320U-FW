/*
 * @brief este proyecto se crea para poder usar el LPTIM 1, como unica fuente de interrupcion y
 * actualizacion del reloj del systema del ThreadX
 *
 * El ThreadX funciona en stop mode 2 durante su tiempo de idle, en lugar de seguir en run mode,
 * como es lo estandar.
 * Al activar "Low Power Suppor" para el ThreadX se crean 4 funciones:
 * App_ThreadX_LowPower_Timer_Setup();
 * App_ThreadX_LowPower_Enter(void);
 * App_ThreadX_LowPower_Exit(void);
 * App_ThreadX_LowPower_Timer_Adjust(void);
 * De las funciones anteriores el programador debe completar para implementar:
 * Programar un timer que despierte al MCU, ya que se ira a en bajo consumo.
 * Enviar el MCU a bajo consumo.
 * Ejecutar el código necesario al salir del bajo consumo.
 * Corregir el sysclok del ThreadX ya que se detuvo en el bajo consumo.
 * necesario de:
 *
 * Como es practica recomendada, no se escribe código en los archivos que toca CubeMX automatica.
 * Las funciones anteriores hace llamadas a sus funciones homonimas en este archivo
 *
 * Se usa el LPTIM 1 como fuente de interrupción, se usa el canal 1 en modo compare, el
 * dato cargado en el registro de compere sera el tiempo que el MCU debe dormir para
 * equiparar el tiempo de inactividad (idle time RTOS).
 * Se usa  el contador del LPTIM para actualizar el sysclock del TheadX. Notar que si la
 * salida del bajo consumo se produce en el valor del registro compare, el conteo del
 * LPTIM!->CNT, tiempo que estuvo dormido coinciden, pero este no tiene que ser siempre el
 * caso, si se despierta por otro evento, el valor del LPTIM->CNT es el unico valido para
 * actualizar el reloj de ThreadX.
 *
 *
 * Versión 1
 * Autor: Daniel H Sagarra
 * Fecha: 10/11/2024
 * Modificaciones: version inicial.
 *
 */

// Includes.
#include "main.h"
#include "fmx_lp.h"
#include "lptim.h"
#include "fm_debug.h"

// Typedef.

// Ingresa en bajo consumo o lo simula, depentiendo el valor de la variable.
enum
{
    MCU_STOP_MODE_DISABLE = 0U, MCU_STOP_MODE_ENABLE
} mcu_stop_mode = MCU_STOP_MODE_ENABLE;

// Const data.

// Defines.
#define LSE_CLK 			32768 // Para reloj de sistema se usa el LSE de 32.768Hz
#define LSE_DIV 			16 // Para reloj de sistema se usa el LSE de 32.768Hz
#define TX_TICK_PER_SECOND 	100	  // EL ThreadX se ajusta a 64 tick por segundo

/*
 * Multiplicador para convertir ticks del ThreadX a ticks del LSE. Tambien funciona como divisor
 * para convertir tick del LSE a ticks del ThreadX.
 */
#define	TICKS_TO_TICKS ((LSE_CLK / LSE_DIV) / TX_TICK_PER_SECOND)

// Debug.

// Project variables, non-static, at least used in other file.

// External variables.

// Global variables, statics.
uint16_t lptim1_stop;
uint16_t lptim1_ticks;

// Private function prototypes.

// Private function bodies.

// Public function bodies.

/*

 *
 *
 * @param
 * @retval
 */
void FMX_LP_Setup(ULONG count)
{
    LPTIM_OC_ConfigTypeDef config =
    { .OCPolarity = LPTIM_OCPOLARITY_HIGH, .Pulse = 65535 };

    //FM_DEBUG_UartInt32(count);
    lptim1_ticks = (count * TICKS_TO_TICKS);
    config.Pulse = (lptim1_ticks);

    if (HAL_LPTIM_OC_ConfigChannel(&hlptim1, &config, LPTIM_CHANNEL_1) != HAL_OK)
    {
        FM_DEBUG_LedError(1);
    }
}

/*
 * @brief
 * @note
 * @param
 * @retval
 */
void FMX_LP_Enter(void)
{
    // Antes de ingresar al modo de bajo consumo apago LED_2_ACTIVE.

    HAL_LPTIM_PWM_Start_IT(&hlptim1, LPTIM_CHANNEL_1);

    FM_DEBUG_LedActive(0);

    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
    HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

    FM_DEBUG_LedActive(1);
}

/*
 * @brief
 * @note
 * @param
 * @retval
 */
void FMX_LP_Exit(void)
{
    lptim1_stop = LPTIM1->CNT;
    HAL_LPTIM_PWM_Stop_IT(&hlptim1, LPTIM_CHANNEL_1);
}

/*
 * @brief
 * @note
 * @param
 * @retval
 */
ULONG FMX_LP_Adjust(void)
{
    ULONG tx_ticks;
    ULONG lse_ticks;
    ULONG drift = 0;

    lse_ticks = lptim1_stop + drift;
    drift = lse_ticks % TICKS_TO_TICKS;
    tx_ticks = lse_ticks / TICKS_TO_TICKS;

    return tx_ticks;
}

/**
 * @brief	Función de retardo que simula actividad
 * @note	Esta función no va a idle como tx_sleep, esta ultima no serviria para simular actividad.
 * 			HAL_delay, se ve afectada por las interrupciones del RTOS scheduler.
 * @param
 * @retval None
 */
void FMX_LP_Delay(ULONG counter)
{
    ULONG initial_time = tx_time_get();
    while ((tx_time_get() - initial_time) < counter);
}

// Interrupts

/*** end of file ***/
