/**
 * @file    fmc_service.c
 * @brief   FMC live-state service boundary implementation.
 */
#include "fmc_service.h"

#include <stddef.h>
#include <stdint.h>

#include "fmc_volume.h"

void FMC_SERVICE_Init(fmc_service_t *p_service)
{
    if (p_service == NULL)
    {
        return;
    }

    FMC_MODEL_Init(&p_service->model);
}

fm_status_t FMC_SERVICE_AddPulseDelta(fmc_service_t *p_service,
                                      uint64_t p_pulse_delta)
{
    if (p_service == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    if (((UINT64_MAX - p_service->model.acm.pulses) < p_pulse_delta) ||
        ((UINT64_MAX - p_service->model.ttl.pulses) < p_pulse_delta))
    {
        return FM_STATUS_ERANGE;
    }

    p_service->model.acm.pulses += p_pulse_delta;
    p_service->model.ttl.pulses += p_pulse_delta;

    return FM_STATUS_OK;
}

fm_status_t FMC_SERVICE_ResetTotal(fmc_service_t *p_service,
                                   fmc_model_total_t p_total)
{
    if (p_service == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    return FMC_MODEL_ResetTotal(&p_service->model, p_total);
}

fm_status_t FMC_SERVICE_GetSnapshot(const fmc_service_t *p_service,
                                    fmc_service_snapshot_t *p_snapshot)
{
    fm_status_t status;

    if ((p_service == NULL) || (p_snapshot == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    p_snapshot->model = p_service->model;

    status = FMC_VOLUME_CalcByTotalRole(&p_snapshot->model,
                                        FMC_MODEL_TOTAL_ACM,
                                        &p_snapshot->acm_volume);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = FMC_VOLUME_CalcByTotalRole(&p_snapshot->model,
                                        FMC_MODEL_TOTAL_TTL,
                                        &p_snapshot->ttl_volume);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    return FM_STATUS_OK;
}
