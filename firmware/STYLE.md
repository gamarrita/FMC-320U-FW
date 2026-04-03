# STYLE.md

## Scope
Applies to all new code and modified code.
Goal: consistent, readable, safe embedded C.

Priority:
1. External APIs and required callback names (`HAL_*`, `tx_*`, `TX_*` types)
2. This document
3. `.clang-format`

---

## Function naming

### Public functions
Use:

```c
FM_MODULE_Action()
FMX_MODULE_Action()
```

Rules:
- Prefix with module name.
- Public API uses uppercase module prefix and uppercase module token.
- Action name uses `CamelCase`.
- Public names must be grep-friendly and unambiguous.

Examples:

```c
void FM_MAIN_SIMPLE_Init(void);
void FM_GPIO_POLL_UpdateSample(fm_gpio_poll_context_t *context, bool pin_high);
void FM_ADC_BASIC_ReadRaw(fm_adc_basic_context_t *context, uint16_t *raw_value);
void FMX_KERNEL_BASIC_Init(fmx_kernel_basic_objs_t *objs,
                           void (*entry_main)(ULONG),
                           void *stack_ptr,
                           ULONG stack_size,
                           UINT priority);
```

### Private static functions
Use:

```c
static void fm_module_action_(void);
```

Rules:
- Always lowercase.
- Always snake_case.
- Always prefixed with full module name.
- Never use `CamelCase` for private functions.
- Never use generic prefixes like `app_`, `process_`, `helper_` without module prefix.

Examples:

```c
static void fm_main_simple_init_services_(void);
static void fm_main_simple_service_heartbeat_(void);
static void fm_gpio_poll_update_sample_(fm_gpio_poll_context_t *context,
                                        bool pin_high);
static uint16_t fm_adc_basic_read_hw_raw_(void);
static void fmx_kernel_basic_create_objects_(fmx_kernel_basic_objs_t *objs,
                                             void (*entry_main)(ULONG),
                                             void *stack_ptr,
                                             ULONG stack_size,
                                             UINT priority);
```

### When to use trailing underscore `_`
Use trailing underscore only for private static functions that are local to one `.c` file.

Use `_` at the end when:
- the function is `static`
- the function is implementation-only
- the name is not part of a public header

Do not use `_` at the end for:
- public API
- HAL callbacks
- RTOS callbacks required by external API
- function pointers exposed in headers
- test hooks intentionally exposed as public API

Examples:

```c
void FM_ADC_BASIC_Init(fm_adc_basic_context_t *context);
static void fm_adc_basic_init_context_(fm_adc_basic_context_t *context);
void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin);
void FM_ADC_BASIC_OnConversionComplete(uint16_t raw_value);
```

### HAL callbacks vs wrappers

#### HAL / RTOS required callbacks
Keep exact external name.
Do not rename them.

Examples:

```c
void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);
void tx_application_define(void *first_unused_memory);
```

#### Internal wrappers called from callbacks
Use normal private naming.

Examples:

```c
void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin)
{
    fm_exti_flag_on_gpio_interrupt_(gpio_pin);
}

static void fm_exti_flag_on_gpio_interrupt_(uint16_t gpio_pin);
```

Rule:
- callback keeps vendor/RTOS name
- real module logic lives in `fm_module_action_()` or public wrapper `FM_MODULE_Action()`
- callback should stay short
- heavy work must be deferred

### Semi-public functions
A semi-public function is callable from other project files, but not intended as the main user-facing API of the module. This is common in drivers, ISR handoff, scheduler integration, or lifecycle hooks.

Use normal public naming, but reserve specific action patterns:

```c
FM_MODULE_Init()
FM_MODULE_Deinit()
FM_MODULE_Process()
FM_MODULE_Task()
FM_MODULE_OnInterrupt()
FM_MODULE_OnTick()
FM_MODULE_OnEvent()
FM_MODULE_ReadRaw()
FM_MODULE_WriteRaw()
```

Guidelines:
- If another `.c` file needs it, it is public and must go in the header.
- Do not invent a third visibility class with special punctuation.
- Distinguish by purpose and header grouping, not by inconsistent naming.

Recommended header grouping:

```c
/* Public API */
void FM_ADC_BASIC_Init(fm_adc_basic_context_t *context);
uint16_t FM_ADC_BASIC_ReadRaw(fm_adc_basic_context_t *context);

/* Integration Hooks */
void FM_ADC_BASIC_OnConversionComplete(uint16_t raw_value);
void FM_ADC_BASIC_Process(void);
```

Meaning:
- `Init`, `Read`, `Write`, `Get`, `Set` -> normal API
- `OnInterrupt`, `OnTick`, `OnEvent`, `Process`, `Task` -> integration hooks / semi-public
- keep same naming convention as other public functions
- avoid private-looking names in headers

---

## Types

- Public types: `fm_module_name_t`, `fmx_module_name_t`
- Use `stdint.h` fixed-width integers
- Keep ownership explicit

Examples:

```c
typedef struct
{
    uint16_t last_raw;
    bool initialized_flag;
} fm_adc_basic_context_t;
```

---

## Macros and constants

- Macros: `FM_MODULE_NAME_VALUE`
- Use suffixes for units when useful: `_MS`, `_SEC`, `_BYTES`
- Replace repeated magic numbers with named constants

Examples:

```c
#define FM_MAIN_SIMPLE_LOOP_PERIOD_MS      (10u)
#define FM_ADC_BASIC_MAX_RAW               (4095u)
#define FMX_KERNEL_BASIC_QUEUE_DEPTH       (8u)
```

---

## File structure

Use fixed sections when relevant:

```c
/* Includes */
/* Public Macros */
/* Public Types */
/* Public API */
```

In `.c` files:

```c
/* Includes */
/* Private Defines */
/* Private Types */
/* Private Data */
/* Private Prototypes */
/* Private Bodies */
/* Public Bodies */
/* Interrupts */
```

Rules:
- One module per file pair.
- Header exposes only what other files need.
- Private helpers stay in `.c`.

---

## Comments

- Comment intent, assumptions, and safety-relevant behavior.
- Do not comment obvious code.
- Public functions may use short Doxygen-style comments when useful.
- Internal functions use short regular comments only when they add value.

Bad:

```c
/* Increment counter */
counter++;
```

Good:

```c
/* Ignore repeated edge until input remains stable for N samples. */
```

---

## Embedded C safety rules

- Validate pointers at module boundaries.
- Initialize persistent context explicitly.
- Avoid hidden shared state unless clearly justified.
- Keep ISR work minimal.
- No blocking inside ISR.
- No dynamic allocation in examples.
- Use explicit status returns where failure matters.
- Keep functions small and deterministic.

---

## Exceptions

Allowed only when required by:
- vendor API
- RTOS API
- generated code interface
- legacy compatibility explicitly accepted by the project

Document the reason near the code when non-obvious.