/*
 * @file    fm_adc_basic.h
 * @brief   Simple ADC driver example.
 */

#ifndef FM_ADC_BASIC_H_
#define FM_ADC_BASIC_H_

/* =========================== Includes ==================================== */
#include <stdint.h>
#include <stdbool.h>

/* =========================== Public Macros =============================== */
#define FM_ADC_BASIC_MAX_RAW   (4095u)
#define FM_ADC_BASIC_REF_MV    (3300u)

/* =========================== Public Types ================================ */
typedef enum
{
    FM_ADC_BASIC_OK = 0,
    FM_ADC_BASIC_ERROR
} fm_adc_basic_status_t;

typedef struct
{
    uint16_t last_raw;
    bool initialized_flag;
} fm_adc_basic_context_t;

/* =========================== Public API ================================== */
void FM_ADC_BASIC_Init(fm_adc_basic_context_t *context);
fm_adc_basic_status_t FM_ADC_BASIC_ReadRaw(fm_adc_basic_context_t *context,
                                           uint16_t *raw_value);
fm_adc_basic_status_t FM_ADC_BASIC_ReadMv(fm_adc_basic_context_t *context,
                                          uint16_t *mv_value);

/* =========================== Integration Hooks =========================== */
void FM_ADC_BASIC_OnConversionComplete(uint16_t raw_value);

#endif /* FM_ADC_BASIC_H_ */

/*** end of file ***/