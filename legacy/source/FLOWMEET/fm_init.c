/*
 * @brief Conjunto de funciones del tipo ModuloXInit(), que deber ser llamdas antes de iniciar el RTOS.
 *
 * Versión 1
 * Autor: Daniel H Sagarra
 * Fecha: 10/11/2024
 * Modificaciones: version inicial.
 *
 */

// Includes.
#include <fm_log.h>
#include "fm_backup.h"
#include "fm_debug.h"
#include "fm_flash.h"
#include "fm_fmc.h"
#include "fm_init.h"
#include "fm_rtc.h"
#include "main.h"
#include "fm_mxc.h"


// Typedef.

// Const data.

// Defines.

// Debug.

// Project variables, non-static, at least used in other file.

// External variables.
extern RTC_HandleTypeDef hrtc;
extern LPTIM_HandleTypeDef hlptim1;
extern LPTIM_HandleTypeDef hlptim3;
extern LPTIM_HandleTypeDef hlptim4;

// Global variables, statics.

// Private function prototypes.

// Private function bodies.

// Public function bodies.

/*
 * @brief
 * @param
 * @retval
 *
 */
void FM_INIT_Init()
{
    flash_chip_info_t chip_info;
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;


    FM_LOG_Init();

    /*
     * El computador usa la backup ram, mantenida por la backup battery, para mantener datos importantes.
     * Ejemplo de estos datos son los pulsos acumulados. El Computador puede hacer reset por diferentes motivos,
     * ejemplo el usuario cambia la batería principal, se remueve la batería principal por viaje en avion, etc.
     * Si la batería principal se remueve datos importantes dejan de ser valido, principalmente pulsos
     * acumulado, fecha y hora.
     * El año del RTC toma valor 0, equivalente a año 2000, en el caso de que la batería de backup se
     * desconectara, se usara esto ara validar el resto de los datos guardados en la ram backup.
     *
     * Se toma la siguiente lógica:
     * - Si es el primer reset del computador se cargan una serie de valores iniciales.
     * - Si no es el primer reset y RTC tiene un año mayor a 2000, se toman los datos de la backup ram como
     * validos, si pasan un chequeo de contorno.
     * - Si el año del RTC es el 2000, se recuperan los datos del ultimo setup valido, si pasan los chequeos
     * de contorno.
     *
     *
     */
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

    FM_DEBUG_Init();

    FM_MXC_PowerOff();

    // El CubeMX configura el LPTIM pero es mi responsabilidad que arranque
    if (HAL_LPTIM_Counter_Start(&hlptim1) != HAL_OK)
    {
        FM_DEBUG_LedError(1);
    }

    // El CubeMX configura el LPTIM pero es mi responsabilidad que arranque
    if (HAL_LPTIM_IC_Start_IT(&hlptim3, LPTIM_CHANNEL_1) != HAL_OK)
    {
        FM_DEBUG_LedError(1);
    }

    // El CubeMX configura el LPTIM pero es mi responsabilidad que arranque
    if (HAL_LPTIM_Counter_Start(&hlptim4) != HAL_OK)
    {
        FM_DEBUG_LedError(1);
    }

    // Por  Por defecto el LPTIM se apaga en stop mode, lo habilito al bajo consumo.
    __HAL_RCC_LPTIM1_CLKAM_ENABLE();
    __HAL_RCC_LPTIM3_CLKAM_ENABLE();
    __HAL_RCC_LPTIM4_CLKAM_ENABLE();

    // Habilito la RAM BACKUP antes de usar.
    FM_BACKUP_Init();

    chip_info = FM_FLASH_ChipInfoRead();

    if (chip_info.reset_counter == 0) // ¿Es la primera vez que se enciende el chip?
    {
        FM_RTC_Init();  // 29 de Agosto del 2007

        // Datos típicos de algún sensor primario.
        FM_FMC_Init(FM_FACTORY_SENSOR_0);
    }
    else
    {
        if (date.Year > 0)  // ¿El RTC tiene año cargado mayor a 2000?
        {
            FM_FMC_Init(FM_FACTORY_RAM_BACKUP);  //FM_FACTORY_RAM_BACKUP
        }
        else // La batería de backup no esta presente o se removido durante el ultimo reset.
        {
            FM_RTC_Init();

            // Atención, esta función debe ser reemplaza con una que recupere valores de ultima configuración!!!!
            FM_FMC_Init(FM_FACTORY_SENSOR_0);
        }
    }
    FM_DEBUG_UartUint32(chip_info.reset_counter);

    FM_FLASH_NewReset();  // Registra un nuevo reset, por cualquier motivo.
}

// Interrupts

/*** end of file ***/
