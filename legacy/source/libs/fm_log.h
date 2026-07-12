/**
 * @file fm_log.h
 * @brief Flow event and diagnostic logger for the FMC-320U.
 */

#ifndef FM_LOG_H_
#define FM_LOG_H_

#include "fm_fmc.h"
#include "fmx.h"

typedef enum {
	FM_LOG_ACT_RATE_TO_OFF,
	FM_LOG_ACT_RATE_TO_ON,
}fm_log_act_t;


typedef struct {
    uint64_t	ttl_pulses;
    uint64_t    acm_pulses;
    uint32_t	rate;
    uint32_t    factor_cal;	// Factor de calbracion calculado manualmente en fabrica, tipicamente en litros.
    uint32_t    time_unix;
    uint16_t    temp_ext;
    uint16_t    temp_int;
    fmx_ack_t 	ack;		// Motivo del log
    uint8_t		reserver[30];
} fm_log_data_t;

_Static_assert(sizeof(fm_log_data_t) == 64, "size must be 64");


void FM_LOG_Init();
fmx_status_t FM_LOG_ReadLog(uint32_t data_index, uint8_t *data_ptr);
fmx_status_t FM_LOG_NewEvent(fmx_ack_t ack);

#endif // FM_LOG_H_

