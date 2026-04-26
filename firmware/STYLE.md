# STYLE.md

## Purpose

This file defines the default coding style for authored firmware code.

It is the source of truth for:
- naming
- public/private API split
- header/source responsibility
- callbacks and wrappers
- module boundaries

When this file conflicts with another style note, this file wins.

---

## Principles

- Prefer consistency over novelty.
- Keep public APIs small and intentional.
- Keep private implementation details local to the `.c` file.
- Use the existing module style unless there is a clear reason to improve it.
- Do not copy legacy naming or structure only because it exists.
- Choose clarity of ownership over decorative abstraction.

---

## Naming

Use the module name as the C namespace.

The module name comes from the filename or from an already established module
family. Do not add a repo-wide `FM_` prefix by default when the module name is
already a clear namespace.

Examples:

```c
/* fmc_model.h */
void FMC_MODEL_Init(void);
fmc_model_t model;
FMC_MODEL_OK;
static bool fmc_model_is_valid_(void);

/* fm_lcd.h, existing namespace */
void FM_LCD_Clear(void);
fm_lcd_status_t status;
FM_LCD_OK;
static void fm_lcd_compose_visible_ram_(void);
```

Public functions:
- use `MODULE_Action`
- are declared only in the module header
- expose real module capability, not convenience internals

Public types:
- use lowercase module prefix and `_t`
- example: `fmc_model_status_t`

Public enum values and public macros:
- use uppercase module prefix
- example: `FMC_MODEL_VOLUME_UNIT_L`

Private functions:
- are `static`
- use lowercase module prefix
- end with `_`
- example: `static bool fmc_model_total_is_valid_(void);`

Variables:
- do not repeat the module name when scope already makes ownership clear
- use short names for tight local scope
- use explicit names for broader scope
- booleans should read as booleans

---

## Headers And Sources

Header files contain only the public surface:
- required includes
- public types
- public constants when truly needed
- public function declarations
- short API comments when useful

Header files do not contain:
- private helper declarations
- implementation-only macros
- internal sequencing details
- unnecessary includes

Source files contain:
- the module header first
- standard includes
- project dependencies
- private macros/types/state/helpers
- public function definitions
- private function definitions

Preferred source layout:

```c
#include "module.h"

#include <stdbool.h>
#include <stdint.h>

/* Private macros */
/* Private types */
/* Private constants */
/* Private variables */
/* Private function declarations */

/* Public function definitions */
/* Private function definitions */
```

---

## Callbacks And Wrappers

Framework or HAL callbacks keep the required external name.

Keep required callbacks thin and forward to a private helper:

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    fm_uart_handle_rx_complete_(huart);
}
```

Wrappers should describe the repository-facing action, not merely repeat the
vendor call name.

---

## Module Boundaries

Make a function public only when another module must call it.

If a helper is used only inside one `.c`, keep it `static`.

Use an internal header only when multiple implementation files truly share an
internal boundary. Keep internal headers rare.

Do not mix these responsibilities casually:
- app orchestration
- board-specific control
- vendor init details
- product domain logic
- service/runtime ownership
- hardware adaptation glue

---

## Comments And Errors

Comment intent, assumptions, ordering constraints, and non-obvious behavior.
Do not comment what the code already says.

Use simple, explicit error handling. Keep ownership of recovery clear.

---

## Decision Rule

When unsure between acceptable options:

1. choose the option closer to current authored code
2. choose the option with smaller API exposure
3. choose the option with clearer ownership
4. choose the option with less future cleanup cost
