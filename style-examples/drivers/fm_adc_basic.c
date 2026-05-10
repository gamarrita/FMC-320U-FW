/*
 * @file    fm_adc_basic.c
 * @brief   Simple ADC driver example.
 */

#include "fm_adc_basic.h"

/* =========================== Private Data ================================ */
static uint16_t g_adc_last_raw;

/* =========================== Private Prototypes ========================== */
static void fm_adc_basic_init_context_(fm_adc_basic_context_t *context);
static uint16_t fm_adc_basic_read_hw_raw_(void);

/* =========================== Private Bodies ============================== */
static void fm_adc_basic_init_context_(fm_adc_basic_context_t *context)
{
    context->last_raw = 0u;
    context->initialized_flag = true;
}

static uint16_t fm_adc_basic_read_hw_raw_(void)
{
    return g_adc_last_raw;
}

/* =========================== Public Bodies =============================== */
void FM_ADC_BASIC_Init(fm_adc_basic_context_t *context)
{
    if (context == NULL)
    {
        return;
    }

    fm_adc_basic_init_context_(context);
}

fm_adc_basic_status_t FM_ADC_BASIC_ReadRaw(fm_adc_basic_context_t *context,
                                           uint16_t *raw_value)
{
    uint16_t sample_raw;

    if ((context == NULL) || (raw_value == NULL))
    {
        return FM_ADC_BASIC_ERROR;
    }

    if (context->initialized_flag == false)
    {
        return FM_ADC_BASIC_ERROR;
    }

    sample_raw = fm_adc_basic_read_hw_raw_();

    if (sample_raw > FM_ADC_BASIC_MAX_RAW)
    {
        return FM_ADC_BASIC_ERROR;
    }

    context->last_raw = sample_raw;
    *raw_value = sample_raw;

    return FM_ADC_BASIC_OK;
}

fm_adc_basic_status_t FM_ADC_BASIC_ReadMv(fm_adc_basic_context_t *context,
                                          uint16_t *mv_value)
{
    uint16_t raw_value;
    uint32_t scaled_mv;

    if (mv_value == NULL)
    {
        return FM_ADC_BASIC_ERROR;
    }

    if (FM_ADC_BASIC_ReadRaw(context, &raw_value) != FM_ADC_BASIC_OK)
    {
        return FM_ADC_BASIC_ERROR;
    }

    scaled_mv = ((uint32_t)raw_value * FM_ADC_BASIC_REF_MV) / FM_ADC_BASIC_MAX_RAW;
    *mv_value = (uint16_t)scaled_mv;

    return FM_ADC_BASIC_OK;
}

void FM_ADC_BASIC_OnConversionComplete(uint16_t raw_value)
{
    g_adc_last_raw = raw_value;
}

/* =========================== Interrupts ================================== */
/*
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    (void)hadc;
    FM_ADC_BASIC_OnConversionComplete(2048u);
}
*/

/*** end of file ***/