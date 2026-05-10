/**
 * @file fm_fmc.h
 * @brief Utilidades de calculo de caudal: totalizador y matematicas de caudal.
 */

#ifndef FM_FMC_H_
#define FM_FMC_H_

#include "fm_lcd_ll.h"
#include "fmx.h"
#include <stddef.h>
#include <stdint.h>

/** Topo de dato punto fijo, sin signo, tres decimales. */
typedef uint32_t ufp3_t;

/** Fuente usada al cargar valores de fabrica. */
typedef enum {
    FM_FACTORY_RAM_BACKUP,
    FM_FACTORY_LAST_SETUP,
    FM_FACTORY_SENSOR_0,
    FM_FACTORY_AI_25,
    FM_FACTORY_AI_80,
} sensors_list_t;


/** Seleccion de punto decimal para mostrar caudal y volumen. */
typedef enum {
    FM_FMC_FP_SEL_0 = 0,
    FM_FMC_FP_SEL_1,
    FM_FMC_FP_SEL_2,
    FM_FMC_FP_SEL_3,
} fm_fmc_fp_sel;

/** Unidades de volumen aceptadas por el totalizador. */
typedef enum {
    VOL_UNIT_BLANK = 0, ///< Marcador sin dimension (debe quedar primero).
    VOL_UNIT_BR,        ///< Barril petrolero estadounidense.
    VOL_UNIT_GL,        ///< Galon estadounidense.
    VOL_UNIT_KG,        ///< Kilogramo.
    VOL_UNIT_LT,        ///< Litro (unidad primaria de calibracion).
    VOL_UNIT_M3,        ///< Metro cubico.
    VOL_UNIT_ME,        ///< Metro cubico equivalente (ej. gas equivalente).
    VOL_UNIT_END,
} fm_fmc_vol_unit_t;

/** Metadatos para cada unidad de volumen soportada. */
typedef struct {
    float unit_convert; ///< Factor de conversion relativo a litros.
    char  name[3];      ///< Etiqueta corta mostrada en el LCD.
} fm_fmc_vol_data_t;

/** Bases de tiempo usadas por los calculos de caudal y totalizador. */
typedef enum {
    TIME_UNIT_SECOND = 0,
    TIME_UNIT_MINUTE,
    TIME_UNIT_HOUR,
    TIME_UNIT_DAY,
    TIME_UNIT_END,
} fm_fmc_time_unit_t;

/** Datos en runtime asociados al caudal instantaneo. */
typedef struct {
    double  	factor_r;    // Factor de caudal (factor K + base de tiempo).
    ufp3_t		delta_t;     // Ultima medicion, en segundos resolucion 1 milisegundo, para calculo del rate.
    ufp3_t  	delta_p;     // Conteo de pulsos para el calulo del rate, durante delta_t.
    ufp3_t  	rate;        // Caudal almacenado en punto fijo.
    uint8_t 	rate_pf_sel; // Posicion del punto decimal mostrado en el LCD.
    ufp3_t 		limit_high;  // Limite nominal superior del caudal.
    ufp3_t  	limit_low;   // Limite nominal inferior del caudal.
    uint32_t 	filter;      // Configuracion adicional del filtro.
    fmx_ack_t 	ack;       // Estado actual del caudal.
} fm_fmc_rate_t;

/** Contadores y configuraciones agregados para el totalizador. */
typedef struct {
    ufp3_t            acm;          ///< Volumen acumulado (x1000).
    ufp3_t            ttl;          ///< Volumen de viaje (x1000).
    uint8_t           vol_pf_sel;   ///< Punto decimal para mostrar ACM/TTL.
    uint64_t          pulse_acm;    ///< Acumulador de pulsos para ACM.
    uint64_t          pulse_ttl;    ///< Acumulador de pulsos para TTL.
    ufp3_t            factor_cal;   ///< Factor de calibracion (pulsos/litro).
    double            factor_k;     ///< Factor K derivado de calibracion.
    fm_fmc_vol_unit_t vol_unit;     ///< Unidad de volumen activa.
    fm_fmc_time_unit_t time_unit;   ///< Base de tiempo activa.
    fm_fmc_rate_t     rate;         ///< Seguimiento del caudal instantaneo.
    uint16_t          ticket_number;///< Ticket secuencial para reportes.   
} fm_fmc_totalizer_t;

#define FM_FMC_FACTOR_CAL_MAX 99999999u
#define FM_FMC_FACTOR_CAL_MIN 1000u

// --- API ---

ufp3_t      FM_FMC_AcmCalc(void);
ufp3_t      FM_FMC_AcmGet(void);
uint64_t    FM_FMC_AcmGetPulse(void);
void        FM_FMC_AcmReset(void);

ufp3_t      FM_FMC_FactorCalGet(void);
uint32_t    FM_FMC_FactorCalSet(ufp3_t factor_cal);
double      FM_FMC_FactorKCalc(ufp3_t factor_k, fm_fmc_vol_unit_t unit);
double      FM_FMC_FactorKGet(void);
uint32_t    FM_FMC_FactorKSet(ufp3_t factor_k);
double      FM_FMC_FactorRateCalc(double factor_k, fm_fmc_time_unit_t unit);
fmx_status_t FM_FMC_FactorRateSet(double factor_rate);

fm_fmc_totalizer_t FM_FMC_GetEnviroment(void);
void              FM_FMC_Init(sensors_list_t sensor);

void     FM_FMC_CaptureSet(uint16_t pulse, uint16_t time);
ufp3_t   FM_FMC_RateCalc(void);
void     FM_FMC_RateClear(void);
ufp3_t   FM_FMC_RateGet(void);
uint8_t  FM_FMC_RateFpSelGet(void);
void     FM_FMC_RateFpInc(void);

uint8_t          FM_FMC_TotalizerFpSelGet(void);
void             FM_FMC_TotalizerFpInc(void);
void             FM_FMC_Ufp3ToString(char *buffer,
                                     size_t buffer_size,
                                     ufp3_t value,
                                     uint8_t sel);
void             FM_FMC_TotalizerStrUnitGet(char **string,
                                            fm_fmc_vol_unit_t unit);
void             FM_FMC_TotalizerTimeUnitSel(fm_fmc_time_unit_t sel);
fm_fmc_time_unit_t FM_FMC_TotalizerTimeUnitGet(void);
fmx_status_t      FM_FMC_TotalizerTimeUnitSet(fm_fmc_time_unit_t time_unit);
fm_fmc_vol_unit_t FM_FMC_TotalizerVolUnitGet(void);
uint32_t          FM_FMC_TotalizerVolUnitSet(fm_fmc_vol_unit_t unit);
uint16_t          FM_FMC_TicketNumberGet(void);

ufp3_t   FM_FMC_TtlCalc(void);
ufp3_t   FM_FMC_TtlGet(void);
uint64_t FM_FMC_TtlPulseGet(void);
void     FM_FMC_TtlReset(void);

void     FM_FMC_PulseAdd(uint32_t pulse_delta);

#endif // FM_FMC_H_

