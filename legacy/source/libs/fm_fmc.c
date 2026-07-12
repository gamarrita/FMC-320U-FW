/**
 * @file fm_fmc.c
 * @brief Registro de caudal y totalizadores del computador de flujo FMC-320U.
 *
 * El modulo guarda contadores de runtime en SRAM de respaldo y ofrece rutinas
 * para calcular volumen acumulado, totales de viaje y caudal instantaneo.
 * Sincroniza calculos con LPTIM3 (32.768 kHz) y la interfaz LCD.
 */

// --- Inclusiones ---
#include "fm_fmc.h"
#include "fm_factory.h"
#include "fm_lcd.h"
#include "fm_debug.h"
#include "fmx.h"
#include <stdio.h>
// --- Tipos ---

/*
 * Los tipos nuevos (struct, union, enum) deben usar minusculas, guiones bajos
 * y terminar con _t.
 *
 * Cada estructura, union y enumeracion nueva se declara mediante typedef.
 */

// --- Datos constantes ---
// Segundos por unidad de tiempo; coincide con fm_fmc_time_unit_t.
const uint32_t seconds_in[] =
{ 1, 60, 3600, 86400, };

/*
 * Cada entrada coincide con fm_fmc_vol_unit_t y define el factor versus litros.
 */
fm_fmc_vol_data_t vol_unit_list[] = {
    { .unit_convert = 1,          .name = "00" },  // Marcador sin dimension.
    { .unit_convert = 158.987304, .name = "BL" }, // Litros por barril.
    { .unit_convert = 3.78541,    .name = "GL" }, // Litros por galon estadounidense.
    { .unit_convert = 1,          .name = "KG" },
    { .unit_convert = 1,          .name = "LT" },
    { .unit_convert = 1000,       .name = "M3" },
    { .unit_convert = 1,          .name = "ME" },
};


// --- Debug ---

// Variables de proyecto no static usadas en otros modulos.

// Variables externas.

// Variables globales estaticas.

/*
 * Entorno en runtime guardado en SRAM de respaldo (alimentada por VBAT).
 * Verificar que las variables nuevas entren en los 2 KB de .RAM_BACKUP_Section.
 */
fm_fmc_totalizer_t totalizer __attribute__((section(".RAM_BACKUP_Section")));

// Prototipos de funciones privadas.

// Cuerpos de funciones privadas.

// Cuerpos de funciones publicas.

/**
 * @brief Inicializa el estado global del totalizador.
 * @param sensor Perfil de fabrica a cargar.
 * @note Tambien recalcula factores K y de caudal segun la configuracion activa.
 * @details
 * Copia el preset desde la RAM de respaldo y recalcula factor_k.
 * Ademas actualiza factor_r segun la base de tiempo antes de habilitar la UI.
 */
void FM_FMC_Init(sensors_list_t sensor)
{
    totalizer = FM_FACTORY_TotalizerGet(sensor);
    totalizer.factor_k = FM_FMC_FactorKCalc(totalizer.factor_cal, totalizer.vol_unit);
    totalizer.rate.factor_r = FM_FMC_FactorRateCalc(totalizer.factor_k, totalizer.time_unit);
}

/**
 * @brief Calcula el volumen acumulado (ACM) desde los contadores de pulsos.
 * @return Volumen en punto fijo (x1000).
 * @details
 * Usa los pulsos acumulados en backup SRAM y aplica factor_k.
 * Convierte el resultado a ufp3_t (x1000) y actualiza totalizer.acm en cache.
 */
ufp3_t FM_FMC_AcmCalc()
{
    double acm;

    acm = totalizer.pulse_acm;
    acm /= totalizer.factor_k;
    acm *= 1000;
    totalizer.acm = (ufp3_t) acm;

    return totalizer.acm;
}

/**
 * @brief Devuelve el volumen ACM almacenado en cache.
 * @note No recalcula; invocar FM_FMC_AcmCalc luego de sumar nuevos pulsos.
 */
ufp3_t FM_FMC_AcmGet()
{
    return (totalizer.acm);
}

/**
 * @brief Devuelve el acumulador de pulsos asociado al ACM.
 * @note Persistido en backup SRAM para soportar reinicios alimentados por VBAT.
 */
uint64_t FM_FMC_AcmGetPulse()
{
    return (totalizer.pulse_acm);
}

/**
 * @brief Limpia el acumulador de ACM y su contador de pulsos.
 * @details
 * Borra el valor en punto fijo y los pulsos base para reiniciar la sesion.
 */
void FM_FMC_AcmReset()
{
    totalizer.acm = 0;
    totalizer.pulse_acm = 0;
}

/**
 * @brief Guarda el delta de pulsos y de tiempo para los calculos de caudal.
 * @details
 * La capa de interrupcion activa esta rutina.
 * FM_FMC_RateCalc consume los datos
 * fuera de la ISR.
 */
void FM_FMC_CaptureSet(uint16_t pulse, uint16_t time)
{
    totalizer.rate.delta_p = pulse;
    totalizer.rate.delta_t = time;
}

/**
 * @brief Devuelve el factor de calibracion en uso.
 * @note Expresado como pulsos por litro en formato ufp3_t.
 */
ufp3_t FM_FMC_FactorCalGet()
{
    return totalizer.factor_cal;
}

/**
 * @brief Actualiza el factor de calibracion si esta dentro de limites.
 * @param factor_cal Nuevo factor de calibracion (pulsos por litro x1000).
 * @return 1 si se acepta el factor, 0 en caso contrario.
 * @details
 * Valida contra los limites MIN/MAX antes de tocar el entorno persistido.
 */
uint32_t FM_FMC_FactorCalSet(ufp3_t factor_cal)
{
    // Limites verificados antes de tocar el entorno persistido.

    if ((factor_cal >= FM_FMC_FACTOR_CAL_MIN) && (factor_cal <= FM_FMC_FACTOR_CAL_MAX))
    {
        totalizer.factor_cal = factor_cal;
    }
    else
    {
        return 0;
    }
    return 1;
}

/**
 * @brief Calcula el factor K a partir de calibracion y unidad activa.
 * @param factor_cal Factor de calibracion (pulsos por litro x1000).
 * @param unit Unidad de volumen activa.
 * @return Factor K expresado en la unidad seleccionada.
 * @details
 * Usa vol_unit_list para escalar la calibracion base en litros.
 * Luego divide por 1000 para recuperar la representacion en double.
 */
double FM_FMC_FactorKCalc(ufp3_t factor_cal, fm_fmc_vol_unit_t unit)
{
    double factor_k;

    // Convierte la calibracion en litros a la unidad mostrada.
    factor_k = factor_cal;

    // El factor K usa la calibracion en litros segun la unidad seleccionada.
    factor_k *= vol_unit_list[unit].unit_convert;

    factor_k /= 1000;  // Necesario para convertir de ufp_t a double.

    return factor_k;
}

/**
 * @brief Devuelve el factor K almacenado en el entorno.
 * @note Representa pulsos por unidad de volumen segun la configuracion actual.
 */
double FM_FMC_FactorKGet()
{
    return totalizer.factor_k;
}

/**
 * @brief Actualiza el factor K cuando el valor queda dentro del rango.
 * @param factor_k Nuevo factor K en formato de punto fijo.
 * @return 1 si se acepta el valor, 0 en caso contrario.
 * @details
 * Mantiene el entorno consistente validando contra los limites de calibracion.
 */
uint32_t FM_FMC_FactorKSet(ufp3_t factor_k)
{
    // Limites verificados antes de tocar el entorno persistido.

    if ((factor_k > FM_FMC_FACTOR_CAL_MIN) && (factor_k < FM_FMC_FACTOR_CAL_MAX))
    {
        totalizer.factor_k = factor_k;
    }
    else
    {
        return 0;
    }
    return 1;
}

/**
 * @brief Convierte una frecuencia de pulsos en unidades de caudal volumetrico.
 * @param factor_k Factor K expresado en la unidad activa.
 * @param time_unit Base de tiempo usada para el caudal.
 * @return Factor de conversion de pulsos/s a volumen/tiempo.
 * @details
 * Parte de 32768 pulsos por segundo generados por el LPTIM.
 * Aplica factor_k para
 * obtener volumen por pulso y escala por seconds_in[time_unit].
 */
double FM_FMC_FactorRateCalc(double factor_k, fm_fmc_time_unit_t time_unit)
{
    double factor_r;

    factor_r = 32768.0;
    factor_r /= factor_k;
    factor_r *= seconds_in[time_unit];

    return factor_r;
}

/**
 * @brief Guarda un factor de conversion de caudal precomputado.
 * @param factor_rate Factor de conversion de pulsos a volumen/tiempo.
 * @return FMX_STATUS_OK si es positivo, FMX_STATUS_ERROR en caso contrario.
 * @details
 * En caso de recibir un valor no positivo fuerza un fallback seguro igual a 1.
 */
fmx_status_t FM_FMC_FactorRateSet(double factor_rate)
{
    fmx_status_t status = FMX_STATUS_OK;

    if (factor_rate > 0)
    {
        totalizer.rate.factor_r = factor_rate;
    }
    else
    {
        totalizer.rate.factor_r = 1;
        status = FMX_STATUS_ERROR;
    }

    return status;
}

/**
 * @brief Devuelve una copia del entorno actual del totalizador.
 * @details
 * La copia es por valor; modificarla no altera el estado persistido.
 * El respaldo real sigue en backup SRAM.
 */
fm_fmc_totalizer_t FM_FMC_GetEnviroment(void)
{
    return totalizer;
}

/**
 * @brief Suma un delta de pulsos a los acumuladores de ACM y TTL.
 * @details
 * Mantiene sincronizados los contadores antes de recalcular ACM o TTL.
 */
void FM_FMC_PulseAdd(uint32_t pulse_delta)
{
    totalizer.pulse_acm += pulse_delta;
    totalizer.pulse_ttl += pulse_delta;
}

/**
 * @brief Calcula el caudal instantaneo usando la ultima captura.
 * @return Caudal en punto fijo (x1000).
 * @details
 * Usa delta_p del sensor y delta_t del LPTIM para obtener pulsos por segundo,
 * los escala con factor_r y guarda el resultado en cache.
 * @warning delta_t debe ser mayor que 1 para evitar division por cero.
 */
ufp3_t FM_FMC_RateCalc()
{
    double rate;

    if ((totalizer.rate.delta_t <= 1u) || (totalizer.rate.factor_r <= 0)) {
        totalizer.rate.rate = 0;
        return totalizer.rate.rate;
    }

    rate = totalizer.rate.delta_p;
    rate /= (totalizer.rate.delta_t-1);
    rate *= totalizer.rate.factor_r;

    // Convierte a punto fijo con tres decimales.
    rate *= 1000;
    totalizer.rate.rate = (ufp3_t) rate;

    return (totalizer.rate.rate);
}

/**
 * @brief Devuelve la posicion decimal usada para mostrar el caudal.
 * @note Compartida por la UI y los reportes del sistema.
 */
uint8_t FM_FMC_RateFpSelGet()
{
    return totalizer.rate.rate_pf_sel;
}

/**
 * @brief Recorre las posiciones decimales disponibles para el caudal.
 * @details
 * Incrementa el selector y vuelve a FM_FMC_FP_SEL_0 cuando alcanza el limite.
 */
void FM_FMC_RateFpInc()
{
    if (totalizer.rate.rate_pf_sel < FM_FMC_FP_SEL_3)
    {
        totalizer.rate.rate_pf_sel++;
    }
    else
    {
        totalizer.rate.rate_pf_sel = FM_FMC_FP_SEL_0;
    }
}

/**
 * @brief Devuelve el caudal instantaneo almacenado en cache.
 * @note Valor actualizado por FM_FMC_RateCalc.
 */
ufp3_t FM_FMC_RateGet()
{
    return totalizer.rate.rate;
}

/**
 * @brief Limpia el cache del caudal instantaneo.
 * @details
 * Se usa al cambiar presets para evitar mostrar valores obsoletos.
 */
void FM_FMC_RateClear()
{
    totalizer.rate.rate = 0;
}

/**
 * @brief Calcula el total de viaje (TTL) a partir de los contadores de pulsos.
 * @return Volumen en punto fijo (x1000).
 * @details
 * Convierte los pulsos acumulados con factor_k y guarda el resultado en cache.
 */
ufp3_t FM_FMC_TtlCalc()
{
    double ttl = 0;

    ttl = totalizer.pulse_ttl;
    ttl /= totalizer.factor_k;

    // Ajusta a punto fijo con tres decimales.
    ttl *= 1000;
    totalizer.ttl = (ufp3_t) ttl;

    return (totalizer.ttl);
}

/**
 * @brief Devuelve el total de viaje (TTL) almacenado en cache.
 * @note Requiere llamar a FM_FMC_TtlCalc para reflejar nuevos pulsos.
 */
ufp3_t FM_FMC_TtlGet()
{
    return (totalizer.ttl);
}

/**
 * @brief Devuelve el acumulador crudo de pulsos usado para el TTL.
 * @note Se mantiene en backup SRAM para conservar el historico.
 */
uint64_t FM_FMC_TtlPulseGet()
{
    return totalizer.pulse_ttl;
}

/**
 * @brief Devuelve la unidad de volumen configurada en el totalizador.
 * @note Util para reflejar la unidad actual en la interfaz.
 */
fm_fmc_vol_unit_t FM_FMC_TotalizerVolUnitGet()
{
    return (totalizer.vol_unit);
}

/**
 * @brief Actualiza la unidad de volumen si el valor solicitado es valido.
 * @param vol_unit Unidad de volumen a aplicar.
 * @return 1 si la unidad se acepta, 0 en caso contrario.
 * @details
 * Solo cambia la enumeracion.
 * El llamador debe recalcular factor_k segun la unidad.
 */
uint32_t FM_FMC_TotalizerVolUnitSet(fm_fmc_vol_unit_t vol_unit)
{
    const char debug_error[] = "FM_FMC_TotalizerVolUnitSet ERROR";

    if ((vol_unit >= VOL_UNIT_BLANK) && (vol_unit < VOL_UNIT_END))
    {
        totalizer.vol_unit = vol_unit;
    }
    else
    {
        FM_DEBUG_LedError(1);
        FM_DEBUG_UartMsg(debug_error, sizeof(debug_error));
        return 0;
    }
    return 1;
}

/**
 * @brief Devuelve la unidad de tiempo que usa el totalizador.
 * @note Refleja la base que escalaron los calculos de caudal.
 */
fm_fmc_time_unit_t FM_FMC_TotalizerTimeUnitGet()
{
    return (totalizer.time_unit);
}

/**
 * @brief Actualiza la unidad de tiempo si el valor es valido.
 * @param time_unit Unidad de tiempo solicitada.
 * @return FMX_STATUS_OK si se acepta, FMX_STATUS_ERROR en caso contrario.
 * @details
 * Solo ajusta la enumeracion.
 * Recalcular factor_r segun la nueva base queda a cargo del llamador.
 */
fmx_status_t FM_FMC_TotalizerTimeUnitSet(fm_fmc_time_unit_t time_unit)
{
    const char debug_error[] = "FM_FMC_TotalizerTimeUnitSet ERROR";

    if ((time_unit >= TIME_UNIT_SECOND) && (time_unit < TIME_UNIT_END))
    {
        totalizer.time_unit = time_unit;
    }
    else
    {
        FM_DEBUG_LedError(1);
        FM_DEBUG_UartMsg(debug_error, sizeof(debug_error));
        return FMX_STATUS_ERROR;
    }
    return FMX_STATUS_OK;
}

/**
 * @brief Limpia el acumulador y el contador de pulsos del TTL.
 * @details
 * Reinicia el recorrido del viaje sin afectar el volumen acumulado total.
 */
void FM_FMC_TtlReset()
{
    totalizer.ttl = 0;
    totalizer.pulse_ttl = 0;
}

/**
 * @brief Devuelve la posicion decimal usada para mostrar ACM/TTL.
 * @note Se usa para alinear el LCD y las exportaciones.
 */
uint8_t FM_FMC_TotalizerFpSelGet()
{
    return totalizer.vol_pf_sel;
}

void FM_FMC_Ufp3ToString(char *buffer,
                         size_t buffer_size,
                         ufp3_t value,
                         uint8_t sel)
{
    uint32_t p_integer;
    uint32_t p_frac;

    if ((buffer == NULL) || (buffer_size == 0u))
    {
        return;
    }

    p_integer = value / 1000u;
    p_frac = value % 1000u;

    switch (sel)
    {
    case FM_FMC_FP_SEL_0:
        snprintf(buffer, buffer_size, "%lu", (unsigned long) p_integer);
        break;
    case FM_FMC_FP_SEL_1:
        p_frac /= 100u;
        snprintf(buffer,
                 buffer_size,
                 "%lu.%01lu",
                 (unsigned long) p_integer,
                 (unsigned long) p_frac);
        break;
    case FM_FMC_FP_SEL_2:
        p_frac /= 10u;
        snprintf(buffer,
                 buffer_size,
                 "%lu.%02lu",
                 (unsigned long) p_integer,
                 (unsigned long) p_frac);
        break;
    case FM_FMC_FP_SEL_3:
        snprintf(buffer,
                 buffer_size,
                 "%lu.%03lu",
                 (unsigned long) p_integer,
                 (unsigned long) p_frac);
        break;
    default:
        FM_DEBUG_LedError(1);
        snprintf(buffer, buffer_size, "%lu", (unsigned long) p_integer);
        break;
    }
}

/**
 * @brief Recorre las posiciones decimales para mostrar ACM/TTL.
 * @details
 * Incrementa el selector y reinicia en FM_FMC_FP_SEL_0 al llegar al limite.
 */
void FM_FMC_TotalizerFpInc()
{
    if (totalizer.vol_pf_sel < FM_FMC_FP_SEL_3)
    {
        totalizer.vol_pf_sel++;
    }
    else
    {
        totalizer.vol_pf_sel = FM_FMC_FP_SEL_0;
    }
}

/**
 * @brief Devuelve la etiqueta corta asociada a una unidad de volumen.
 * @param string Puntero donde se entrega la etiqueta.
 * @param vol_unit Unidad de volumen solicitada.
 * @details
 * La etiqueta corresponde a vol_unit_list y se usa para la UI del LCD.
 */
void FM_FMC_TotalizerStrUnitGet(char **string, fm_fmc_vol_unit_t vol_unit)
{
    *string = vol_unit_list[vol_unit].name;
}

/**
 * @brief Destaca el simbolo de la unidad de tiempo activa en el LCD.
 * @param sel Unidad de tiempo a activar.
 * @details
 * Borra el estado previo y habilita el simbolo correspondiente al nuevo valor.
 */
void FM_FMC_TotalizerTimeUnitSel(fm_fmc_time_unit_t sel)
{
    static fm_fmc_time_unit_t sel_old = -1;

    // Limpia los simbolos previos.
    if (sel != sel_old)
    {
        FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_S, 0);
        FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_M, 0);
        FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_H, 0);
        FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_D, 0);
    }

    sel_old = sel;

    switch (sel)
    {
    case TIME_UNIT_SECOND:
        FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_S, 1);
        break;
    case TIME_UNIT_MINUTE:
        FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_M, 1);
        break;
    case TIME_UNIT_HOUR:
        FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_H, 1);
        break;
    case TIME_UNIT_DAY:
        FM_LCD_LL_SymbolWrite(FM_LCD_LL_SYM_D, 1);
        break;
    default:
        break;
    }
}

/**
 * @brief Incrementa y devuelve el numero de ticket para reportes.
 * @details
 * El contador vive en backup SRAM para mantener consecutividad tras reinicios.
 */
uint16_t FM_FMC_TicketNumberGet()
{
    totalizer.ticket_number++;
    return totalizer.ticket_number;
}

// Interrupciones

/*** FIN DEL ARCHIVO ***/











