/**
 * @file    fm_main_ui.c
 * @brief   Product-main runtime/UI coordination implementation.
 */
#include "fm_main_ui.h"

#include <stddef.h>

static bool fm_main_ui_state_is_user_(fmc_ui_state_t state);
static fm_status_t fm_main_ui_acknowledge_(fmc_runtime_t *p_runtime);

fm_status_t FM_MAIN_UI_MakeSnapshot(
    const fmc_runtime_t *p_runtime,
    fmc_ui_snapshot_t *p_snapshot)
{
    fmc_service_snapshot_t service_snapshot;
    fm_status_t status;

    if ((p_runtime == NULL) || (p_snapshot == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    status = FMC_RUNTIME_GetSnapshot(p_runtime, &service_snapshot);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = FMC_RUNTIME_GetRateState(p_runtime, &p_snapshot->rate);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    p_snapshot->acm = service_snapshot.acm_volume;
    p_snapshot->ttl = service_snapshot.ttl_volume;
    p_snapshot->volume_unit =
        service_snapshot.model.measurement.active_volume_unit;
    p_snapshot->rate_time_base =
        service_snapshot.model.measurement.active_time_base;
    p_snapshot->volume_fractional_digits =
        FMC_UI_VALUE_FRACTIONAL_DIGITS;
    p_snapshot->rate_fractional_digits =
        FMC_UI_VALUE_FRACTIONAL_DIGITS;

    return FM_STATUS_OK;
}

fm_status_t FM_MAIN_UI_HandleInput(
    fmc_runtime_t *p_runtime,
    fmc_ui_t *p_ui,
    const fmc_input_event_t *p_input,
    fm_main_ui_result_t *p_result)
{
    fmc_runtime_event_t runtime_event;
    fmc_ui_request_t request;
    fmc_ui_snapshot_t snapshot;
    fm_status_t status;

    if (p_result != NULL)
    {
        p_result->previous_state = FMC_UI_STATE_NOT_STARTED;
        p_result->current_state = FMC_UI_STATE_NOT_STARTED;
        p_result->presentation_performed = false;
        p_result->acm_reset_executed = false;
    }

    if ((p_runtime == NULL) || (p_ui == NULL) || (p_input == NULL) ||
        (p_result == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    p_result->previous_state = FMC_UI_GetState(p_ui);
    p_result->current_state = p_result->previous_state;

    status = FM_MAIN_UI_MakeSnapshot(p_runtime, &snapshot);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = FMC_UI_HandleInput(p_ui, p_input, &snapshot, &request);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    p_result->current_state = FMC_UI_GetState(p_ui);
    p_result->presentation_performed =
        p_result->current_state != p_result->previous_state;

    if (request.kind == FMC_UI_REQUEST_RESET_ACM)
    {
        runtime_event.kind = FMC_RUNTIME_EVENT_RESET_ACM;
        status = FMC_RUNTIME_Dispatch(p_runtime, &runtime_event);
        if (status != FM_STATUS_OK)
        {
            return status;
        }

        p_result->acm_reset_executed = true;
        status = FM_MAIN_UI_MakeSnapshot(p_runtime, &snapshot);
        if (status != FM_STATUS_OK)
        {
            return status;
        }

        status = FMC_UI_Refresh(p_ui, &snapshot);
        if (status != FM_STATUS_OK)
        {
            return status;
        }

        p_result->presentation_performed = true;
    }
    else if (request.kind != FMC_UI_REQUEST_NONE)
    {
        return FM_STATUS_EINVAL;
    }

    if (p_result->presentation_performed &&
        fm_main_ui_state_is_user_(p_result->current_state))
    {
        return fm_main_ui_acknowledge_(p_runtime);
    }

    return FM_STATUS_OK;
}

fm_status_t FM_MAIN_UI_Refresh(
    fmc_runtime_t *p_runtime,
    fmc_ui_t *p_ui)
{
    fmc_ui_snapshot_t snapshot;
    fm_status_t status;

    if ((p_runtime == NULL) || (p_ui == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    status = FM_MAIN_UI_MakeSnapshot(p_runtime, &snapshot);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    status = FMC_UI_Refresh(p_ui, &snapshot);
    if (status != FM_STATUS_OK)
    {
        return status;
    }

    return fm_main_ui_acknowledge_(p_runtime);
}

static bool fm_main_ui_state_is_user_(fmc_ui_state_t state)
{
    return (state >= FMC_UI_STATE_TTL_RATE) &&
           (state <= FMC_UI_STATE_DATE_TIME);
}

static fm_status_t fm_main_ui_acknowledge_(fmc_runtime_t *p_runtime)
{
    return FMC_RUNTIME_ClearPresentationUpdatePending(p_runtime);
}
