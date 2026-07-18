/**
 * @file    fmc_runtime.c
 * @brief   RTOS-neutral FMC runtime event boundary implementation.
 */
#include "fmc_runtime.h"

#include <stdbool.h>
#include <stddef.h>

static fm_status_t fmc_runtime_reset_total_(fmc_runtime_t *p_runtime,
                                            fmc_model_total_t p_total);
static bool fmc_runtime_input_is_valid_(const fmc_input_event_t *p_input);
static bool fmc_runtime_input_key_is_valid_(fmc_input_key_t p_key);
static bool fmc_runtime_input_action_is_valid_(fmc_input_action_t p_action);

void FMC_RUNTIME_Init(fmc_runtime_t *p_runtime)
{
    if (p_runtime == NULL)
    {
        return;
    }

    FMC_SERVICE_Init(&p_runtime->service);
    p_runtime->presentation_update_pending = false;
    p_runtime->last_input.key = FMC_INPUT_KEY_COUNT;
    p_runtime->last_input.action = FMC_INPUT_ACTION_COUNT;
    p_runtime->last_input_valid = false;
}

fm_status_t FMC_RUNTIME_Dispatch(fmc_runtime_t *p_runtime,
                                 const fmc_runtime_event_t *p_event)
{
    fm_status_t status;

    if ((p_runtime == NULL) || (p_event == NULL))
    {
        return FM_STATUS_EINVAL;
    }

    switch (p_event->kind)
    {
    case FMC_RUNTIME_EVENT_NONE:
        return FM_STATUS_OK;

    case FMC_RUNTIME_EVENT_PULSE_DELTA:
        status = FMC_SERVICE_AddPulseDelta(&p_runtime->service,
                                           p_event->data.pulse_delta);
        if (status == FM_STATUS_OK)
        {
            p_runtime->presentation_update_pending = true;
        }
        return status;

    case FMC_RUNTIME_EVENT_RESET_ACM:
        return fmc_runtime_reset_total_(p_runtime, FMC_MODEL_TOTAL_ACM);

    case FMC_RUNTIME_EVENT_RESET_TTL:
        return fmc_runtime_reset_total_(p_runtime, FMC_MODEL_TOTAL_TTL);

    case FMC_RUNTIME_EVENT_INPUT:
        if (!fmc_runtime_input_is_valid_(&p_event->data.input))
        {
            return FM_STATUS_EINVAL;
        }
        p_runtime->last_input = p_event->data.input;
        p_runtime->last_input_valid = true;
        p_runtime->presentation_update_pending = true;
        return FM_STATUS_OK;

    case FMC_RUNTIME_EVENT_PRESENTATION_INVALIDATE:
        p_runtime->presentation_update_pending = true;
        return FM_STATUS_OK;

    default:
        return FM_STATUS_EINVAL;
    }
}

fm_status_t FMC_RUNTIME_GetSnapshot(const fmc_runtime_t *p_runtime,
                                    fmc_service_snapshot_t *p_snapshot)
{
    if (p_runtime == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    return FMC_SERVICE_GetSnapshot(&p_runtime->service, p_snapshot);
}

bool FMC_RUNTIME_PresentationUpdateIsPending(
    const fmc_runtime_t *p_runtime)
{
    if (p_runtime == NULL)
    {
        return false;
    }

    return p_runtime->presentation_update_pending;
}

fm_status_t FMC_RUNTIME_ClearPresentationUpdatePending(
    fmc_runtime_t *p_runtime)
{
    if (p_runtime == NULL)
    {
        return FM_STATUS_EINVAL;
    }

    p_runtime->presentation_update_pending = false;

    return FM_STATUS_OK;
}

static fm_status_t fmc_runtime_reset_total_(fmc_runtime_t *p_runtime,
                                            fmc_model_total_t p_total)
{
    fm_status_t status;

    status = FMC_SERVICE_ResetTotal(&p_runtime->service, p_total);
    if (status == FM_STATUS_OK)
    {
        p_runtime->presentation_update_pending = true;
    }

    return status;
}

static bool fmc_runtime_input_is_valid_(const fmc_input_event_t *p_input)
{
    return (p_input != NULL) &&
           fmc_runtime_input_key_is_valid_(p_input->key) &&
           fmc_runtime_input_action_is_valid_(p_input->action);
}

static bool fmc_runtime_input_key_is_valid_(fmc_input_key_t p_key)
{
    switch (p_key)
    {
    case FMC_INPUT_KEY_DOWN:
    case FMC_INPUT_KEY_UP:
    case FMC_INPUT_KEY_ENTER:
    case FMC_INPUT_KEY_ESC:
    case FMC_INPUT_KEY_EXT_1:
    case FMC_INPUT_KEY_EXT_2:
        return true;

    case FMC_INPUT_KEY_COUNT:
    default:
        return false;
    }
}

static bool fmc_runtime_input_action_is_valid_(fmc_input_action_t p_action)
{
    switch (p_action)
    {
    case FMC_INPUT_ACTION_SHORT:
    case FMC_INPUT_ACTION_LONG:
        return true;

    case FMC_INPUT_ACTION_COUNT:
    default:
        return false;
    }
}
