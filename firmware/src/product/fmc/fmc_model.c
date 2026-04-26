/**
 * @file    fmc_model.c
 * @brief   FMC canonical model implementation.
 *
 * This file implements only the structural helpers declared by `fmc_model.h`.
 * It intentionally does not own unit conversion, visible-volume calculation,
 * rate calculation, presentation, persistence, or RTOS synchronization.
 */
#include "fmc_model.h"

#include <stddef.h>

static fmc_model_total_state_t *fmc_model_get_total_state_(fmc_model_t *p_model,
                                                           fmc_model_total_t p_total);
static const fmc_model_total_state_t *fmc_model_get_total_state_const_(
    const fmc_model_t *p_model,
    fmc_model_total_t p_total);

void FMC_MODEL_Init(fmc_model_t *p_model)
{
    if (p_model == NULL)
    {
        return;
    }

    p_model->measurement.calibration_pulses_per_unit =
        FMC_MODEL_CALIBRATION_PULSES_PER_UNIT_DEFAULT;
    p_model->measurement.calibration_volume_unit =
        FMC_MODEL_CALIBRATION_UNIT_DEFAULT;
    p_model->measurement.active_volume_unit = FMC_MODEL_VOLUME_UNIT_L;
    p_model->measurement.active_time_base = FMC_MODEL_TIME_BASE_SECOND;

    p_model->acm.pulses = 0U;
    p_model->ttl.pulses = 0U;
}

fmc_model_reset_policy_t FMC_MODEL_GetResetPolicy(fmc_model_total_t p_total)
{
    switch (p_total)
    {
    case FMC_MODEL_TOTAL_ACM:
        return FMC_MODEL_RESET_USER_ALLOWED;

    case FMC_MODEL_TOTAL_TTL:
    default:
        return FMC_MODEL_RESET_PRIVILEGED;
    }
}

fmc_model_total_state_t *FMC_MODEL_GetTotalState(fmc_model_t *p_model,
                                                 fmc_model_total_t p_total)
{
    return fmc_model_get_total_state_(p_model, p_total);
}

const fmc_model_total_state_t *FMC_MODEL_GetTotalStateConst(
    const fmc_model_t *p_model,
    fmc_model_total_t p_total)
{
    return fmc_model_get_total_state_const_(p_model, p_total);
}

fmc_model_status_t FMC_MODEL_ResetTotal(fmc_model_t *p_model,
                                        fmc_model_total_t p_total)
{
    fmc_model_total_state_t *total = fmc_model_get_total_state_(p_model,
                                                                p_total);

    if (total == NULL)
    {
        return FMC_MODEL_EINVAL;
    }

    total->pulses = 0U;

    return FMC_MODEL_OK;
}

static fmc_model_total_state_t *fmc_model_get_total_state_(fmc_model_t *p_model,
                                                           fmc_model_total_t p_total)
{
    if (p_model == NULL)
    {
        return NULL;
    }

    switch (p_total)
    {
    case FMC_MODEL_TOTAL_ACM:
        return &p_model->acm;

    case FMC_MODEL_TOTAL_TTL:
        return &p_model->ttl;

    default:
        return NULL;
    }
}

static const fmc_model_total_state_t *fmc_model_get_total_state_const_(
    const fmc_model_t *p_model,
    fmc_model_total_t p_total)
{
    if (p_model == NULL)
    {
        return NULL;
    }

    switch (p_total)
    {
    case FMC_MODEL_TOTAL_ACM:
        return &p_model->acm;

    case FMC_MODEL_TOTAL_TTL:
        return &p_model->ttl;

    default:
        return NULL;
    }
}
