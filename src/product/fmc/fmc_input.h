/**
 * @file    fmc_input.h
 * @brief   FMC semantic input event contract.
 *
 * This header defines product input identity after hardware-specific details
 * have been translated away. It does not depend on GPIO, EXTI, HAL, BSP,
 * ThreadX, queues, or timers.
 *
 * The current product contract supports mechanical keys and external
 * pushbuttons. Mechanical keys support `SHORT` and `LONG`. External
 * pushbuttons support `SHORT` on release only; they do not produce a
 * three-second `LONG` event.
 *
 * This module owns no debounce, timing, wake, backlight, menu, password, or UI
 * state. Those are consequences of accepted semantic input and belong to later
 * runtime/UI/power slices.
 */
#ifndef FMC_INPUT_H
#define FMC_INPUT_H

/**
 * @brief Product-level input key identity.
 *
 * These values name operator inputs, not board pins. Board-specific keys and
 * external buttons must be translated before creating an `fmc_input_event_t`.
 */
typedef enum
{
    FMC_INPUT_KEY_DOWN = 0,
    FMC_INPUT_KEY_UP,
    FMC_INPUT_KEY_ENTER,
    FMC_INPUT_KEY_ESC,
    FMC_INPUT_KEY_EXT_1,
    FMC_INPUT_KEY_EXT_2,
    FMC_INPUT_KEY_COUNT
} fmc_input_key_t;

/**
 * @brief Product-level input action identity.
 *
 * For mechanical keys, `SHORT` is executed on release when no long action has
 * already fired, and `LONG` represents the single action emitted after the
 * configured hold threshold. External pushbuttons use `SHORT` on release only.
 * Recognizing those conditions is a producer responsibility, not this type's
 * responsibility.
 */
typedef enum
{
    FMC_INPUT_ACTION_SHORT = 0,
    FMC_INPUT_ACTION_LONG,
    FMC_INPUT_ACTION_COUNT
} fmc_input_action_t;

/**
 * @brief One semantic input event delivered to product runtime.
 *
 * Both fields must form a valid key/action combination. The event does not
 * carry hardware edge, pin, timing, or ISR context details.
 */
typedef struct
{
    fmc_input_key_t key;
    fmc_input_action_t action;
} fmc_input_event_t;

#endif /* FMC_INPUT_H */
