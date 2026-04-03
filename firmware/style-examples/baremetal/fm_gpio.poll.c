/*
 * @file    fm_gpio_poll.c
 * @brief   Ejemplo simple de filtrado por estabilidad en polling.
 */

#include "fm_gpio_poll.h"

/* =========================== Private Prototypes ========================== */
static fm_gpio_poll_level_t sample_to_level_(bool pin_high);

/* =========================== Private Bodies ============================== */
static fm_gpio_poll_level_t sample_to_level_(bool pin_high)
{
    return (pin_high == true) ? FM_GPIO_POLL_LEVEL_HIGH
                              : FM_GPIO_POLL_LEVEL_LOW;
}

/* =========================== Public Bodies =============================== */
void FM_GPIO_POLL_GetDefaultConfig(fm_gpio_poll_config_t *config)
{
    if (config == NULL)
    {
        return;
    }

    config->stable_count_threshold = FM_GPIO_POLL_STABLE_COUNT_DEFAULT;
}

void FM_GPIO_POLL_Init(fm_gpio_poll_context_t *context,
                       const fm_gpio_poll_config_t *config)
{
    if ((context == NULL) || (config == NULL))
    {
        return;
    }

    context->stable_level = FM_GPIO_POLL_LEVEL_LOW;
    context->sampled_level = FM_GPIO_POLL_LEVEL_LOW;
    context->stable_count = config->stable_count_threshold;
    context->changed_flag = false;
    context->initialized_flag = true;
}

void FM_GPIO_POLL_UpdateSample(fm_gpio_poll_context_t *context, bool pin_high)
{
    fm_gpio_poll_level_t new_level;

    if ((context == NULL) || (context->initialized_flag == false))
    {
        return;
    }

    new_level = sample_to_level_(pin_high);

    if (new_level == context->sampled_level)
    {
        if (context->stable_count < 0xFFu)
        {
            context->stable_count++;
        }
    }
    else
    {
        context->sampled_level = new_level;
        context->stable_count = 1u;
    }

    if ((context->stable_level != context->sampled_level) &&
        (context->stable_count >= FM_GPIO_POLL_STABLE_COUNT_DEFAULT))
    {
        context->stable_level = context->sampled_level;
        context->changed_flag = true;
    }
}

bool FM_GPIO_POLL_HasChanged(fm_gpio_poll_context_t *context)
{
    bool changed_flag = false;

    if ((context == NULL) || (context->initialized_flag == false))
    {
        return false;
    }

    changed_flag = context->changed_flag;
    context->changed_flag = false;

    return changed_flag;
}

fm_gpio_poll_level_t FM_GPIO_POLL_GetLevel(const fm_gpio_poll_context_t *context)
{
    if ((context == NULL) || (context->initialized_flag == false))
    {
        return FM_GPIO_POLL_LEVEL_LOW;
    }

    return context->stable_level;
}

/*** end of file ***/
