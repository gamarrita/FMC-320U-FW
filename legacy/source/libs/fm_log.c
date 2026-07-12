/**
 * @file fm_log.c
 * @brief	Gestiona los datos guardados en la BACKUP RAM de manera directa.
 * 			Para la memoria flash comprueba los limites y datos a guardar, la
 * 			escritura de la flash se hace con una API externa.
 *
 */

#include "fm_log.h"
#include "fm_rtc.h"
#include "main.h"
#include "fm_flash.h"
#include "fm_debug.h"
#include "fm_fmc.h"
#include "fmx.h"
#include "fm_log_policy.h"

// --- Constants ---

/*
 * La siguiente seccion ajusta el tamaño, cantidad de bytes, que se loggean.
 * La BACK RAM, tal como se la llama en los data sheets del STM32U5, es de 2KB:
 * 	- 1KB esta reservado para varibles de entorno.
 * 	- 1KB esta reservado para no escribir directamente sobre flash.
 * La potilica de logeo, y esto es imporante, no permite en promedio escribir mas de un dato cada 15 minutos.
 * Llamamos LOG_DATA_SIZE, al tamaño en bytes, que se loguea.
 * El espacio de BACKUP RAM permite hacer buffer de 16 unidades de loggeo,
 * que equivale a escribir la flash cada 240 minutos maximo.
 *
 */
#define LOG_DATA_SIZE             (64u)
#define BACKUP_RAM_SIZE           (1024u)
#define DATA_BACKUP_RAM_LENGTH    (BACKUP_RAM_SIZE / LOG_DATA_SIZE)		//
#define DATA_FLASH_LENGTH         (FM_FLASH_LOG_SIZE / LOG_DATA_SIZE) 	// Equivale a 16128 registros

// --- Logger state ---

static uint32_t log_data_address = FM_FLASH_LOG_START;
static fm_log_data_t data_buffer[DATA_BACKUP_RAM_LENGTH] __attribute__((section(".RAM_BACKUP_Section")));

// --- Private functions ---

static void LogFlash(void);


/**
 *
 */
void FM_LOG_Init()
{
	memset(data_buffer, 0, sizeof(data_buffer));
}


/**
 *
 */
fmx_status_t FM_LOG_NewEvent(fmx_ack_t ack)
{
    static uint8_t data_index = 0u;
	static uint32_t time_now;

	if(FM_LOG_POLICY_Step(ack))
	{
		// Se loggeara un nuevo dato.
		time_now = FM_RTC_GetUnixTime();
	}
	else
	{
		// No se permite loggear
		return FMX_STATUS_ERROR;
	}

	data_buffer[data_index].ack        = ack;
	data_buffer[data_index].time_unix  = time_now;
	data_buffer[data_index].ttl_pulses = FM_FMC_TtlPulseGet();
	data_buffer[data_index].acm_pulses = FM_FMC_AcmGetPulse();
	data_buffer[data_index].factor_cal = FM_FMC_FactorCalGet();
	data_buffer[data_index].rate	   = FM_FMC_RateGet();

    data_index++;

    if (data_index >= DATA_BACKUP_RAM_LENGTH)
    {
          data_index = 0u;
          LogFlash();
    }
    return FMX_STATUS_OK;
}

/**
 *
 */
static void LogFlash(void)
{

    FM_FLASH_Write(log_data_address, (const uint8_t *)data_buffer, BACKUP_RAM_SIZE);
    log_data_address += BACKUP_RAM_SIZE;

    if (log_data_address >= FM_FLASH_LOG_END) {
        log_data_address = FM_FLASH_LOG_START;
    }
 }

// --- API ---

/**
 * Reads log entries from Flash in reverse chronological order.
 * @param data_index Relative index: 1 returns the latest entry, etc.
 * @param data_ptr Destination buffer to copy the entry into.
 * @return Number of bytes read or an error code.
 */
fmx_status_t FM_LOG_ReadLog(uint32_t data_index, uint8_t *data_ptr)
{
    fmx_status_t ret_status = FMX_STATUS_OK;
    uint32_t memory_address;
    uint32_t data_offset_index;

    if (data_index >= DATA_FLASH_LENGTH) {
        return FMX_STATUS_OUT_OF_RANGE;
    }

    data_offset_index = (log_data_address - FM_FLASH_LOG_START) / LOG_DATA_SIZE;

    if (data_index <= data_offset_index) {
        memory_address = log_data_address;
        memory_address -= (data_offset_index * LOG_DATA_SIZE);
    } else {
        data_offset_index -= (data_index + 1u);
        memory_address = FM_FLASH_LOG_END - (data_offset_index * LOG_DATA_SIZE);
    }

    FM_FLASH_Read(memory_address, data_ptr, LOG_DATA_SIZE);
    return ret_status;
}
