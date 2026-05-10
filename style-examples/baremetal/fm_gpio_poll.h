/*
 * @file    fm_gpio_poll.h
 * @brief   Ejemplo simple de lectura por polling.
 */

#ifndef FM_GPIO_POLL_H_
#define FM_GPIO_POLL_H_

/* =========================== Includes ==================================== */
#include <stdint.h>
#include <stdbool.h>

/* =========================== Public Macros =============================== */
#define FM_GPIO_POLL_STABLE_COUNT_DEFAULT   (3u)

/* =========================== Public Types ================================ */
typedef enum
{
    FM_GPIO_POLL_LEVEL_LOW = 0,
    FM_GPIO_POLL_LEVEL_HIGH
} fm_gpio_poll_level_t;

typedef struct
{
    uint8_t stable_count_threshold;
} fm_gpio_poll_config_t;

typedef struct
{
    fm_gpio_poll_level_t stable_level;
    fm_gpio_poll_level_t sampled_level;
    uint8_t              stable_count;
    bool                 changed_flag;
    bool                 initialized_flag;
} fm_gpio_poll_context_t;

/* =========================== Public API ================================== */
void FM_GPIO_POLL_GetDefaultConfig(fm_gpio_poll_config_t *config);
void FM_GPIO_POLL_Init(fm_gpio_poll_context_t *context,
                       const fm_gpio_poll_config_t *config);
void FM_GPIO_POLL_UpdateSample(fm_gpio_poll_context_t *context, bool pin_high);
bool FM_GPIO_POLL_HasChanged(fm_gpio_poll_context_t *context);
fm_gpio_poll_level_t FM_GPIO_POLL_GetLevel(const fm_gpio_poll_context_t *context);

#endif /* FM_GPIO_POLL_H_ */

/*** end of file ***/