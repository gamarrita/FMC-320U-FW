# STYLE.md

## Purpose

This file defines the coding style, naming rules, and file structure conventions
for `firmware/`.

This is the source of truth for:

- naming
- public and private split
- callbacks and wrappers
- `.h` and `.c` responsibility split
- minimal module structure

If another document conflicts with this one on code style or naming,
this file wins.

---

## General principles

- Prefer consistency over novelty.
- Keep code simple and readable.
- Keep responsibilities separated.
- Keep public APIs minimal.
- Keep private implementation details local.
- Avoid decorative abstraction.
- Use existing repository patterns when available.

---

## Naming overview

### Public symbols

Public symbols should use a module-prefixed, explicit style.

Recommended form:

```c
FM_Module_Action(void);
```

Examples:

```c
void FM_Uart_Init(void);
void FM_App_Start(void);
void FM_Service_Process(void);
```

Use PascalCase after the module prefix for public functions.

Public names should:
- be explicit
- reflect responsibility
- remain stable if part of a reusable module API

---

### Private functions

Private functions should remain local to the `.c` file and use `static`.

Recommended form:

```c
static void fm_module_action_(void);
```

Examples:

```c
static void fm_uart_configure_(void);
static void fm_app_run_state_(void);
static void fm_service_update_(void);
```

Use lowercase with underscores for private functions.

---

## Final underscore rule

Use a trailing underscore only for private internal functions and private internal helpers
that are local to the implementation file.

Use trailing `_` for:

- static helper functions
- static internal state handlers
- static local orchestration helpers
- static internal conversion or translation helpers

Do not use trailing `_` for:

- public APIs
- exported functions
- types
- macros
- enum values
- public callbacks exposed as API
- filenames

Examples:

```c
void FM_Module_Init(void);
static void fm_module_init_hw_(void);
static bool fm_module_is_ready_(void);
```

---

## Public versus private rule

### Public functions

Make a function public only if another module must call it.

A public function should:
- belong in the module header
- have a stable and intentional name
- expose a real module capability

### Private functions

Make a function private if it is:

- only used inside one `.c` file
- only part of local sequencing
- only part of internal state progression
- only an implementation detail

Do not expose internal helpers just for convenience.

---

## Header and source split

### Header file should contain

- include guard or `#pragma once` according to project convention
- required public includes only
- public types
- public constants if truly needed
- public function declarations
- minimal API comments if the project uses them consistently

### Header file should not contain

- internal static helper declarations
- private macros unrelated to API
- internal sequencing details
- unnecessary includes
- implementation-only state

### Source file should contain

- module includes
- required private includes
- private macros
- private types
- private state
- private helper declarations
- public function definitions
- private function definitions

---

## File organization pattern

Recommended source file structure:

```c
#include "fm_module.h"

#include <stdbool.h>
#include <stdint.h>

#include "dependency_a.h"
#include "dependency_b.h"

/* Private macros */

/* Private types */

/* Private constants */

/* Private variables */

/* Private function declarations */

static void fm_module_prepare_(void);
static void fm_module_update_state_(void);

/* Public function definitions */

void FM_Module_Init(void)
{
    fm_module_prepare_();
}

/* Private function definitions */

static void fm_module_prepare_(void)
{
    /* ... */
}

static void fm_module_update_state_(void)
{
    /* ... */
}
```

Keep ordering consistent across modules.

---

## Module naming

Use the filename and the exported API as the naming anchor.

If the module is `fm_uart.c` and `fm_uart.h`, expected names are:

```c
void FM_Uart_Init(void);
void FM_Uart_Process(void);

static void fm_uart_configure_(void);
static void fm_uart_flush_rx_(void);
```

Avoid mixing unrelated prefixes in the same module.

---

## Callback naming

### HAL or framework callbacks

Use the expected external callback name exactly when the framework or HAL requires it.

Do not rename required callback entry points arbitrarily.

If project-local logic is needed, keep the required callback thin and forward to a local helper.

Example:

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    fm_uart_handle_rx_complete_(huart);
}

static void fm_uart_handle_rx_complete_(UART_HandleTypeDef *huart)
{
    /* project logic */
}
```

Rule:
- required HAL or framework callback keeps required external name
- project logic moves to a private helper with local naming convention

---

## Wrapper naming

Use wrappers when adapting an external API to repository-local semantics.

Recommended form for public wrappers:

```c
void FM_Uart_StartRx(void);
void FM_Gpio_Write(void);
uint32_t FM_Time_GetMs(void);
```

Recommended form for private wrappers:

```c
static void fm_uart_start_rx_hw_(void);
static void fm_gpio_write_pin_(void);
```

Wrapper names should describe the repository-facing action,
not repeat vendor names unnecessarily unless that distinction matters.

---

## Semi-public functions inside a module family

Sometimes a function is not globally public but is still shared inside one module family
or subdomain.

Rule:

- if it is used across translation units, it is public to that boundary and must be declared in a header
- if it is only local to one `.c`, keep it private and static
- do not create "kind of public" functions without an explicit header boundary

Recommended approach:

- use a dedicated internal header only when a real multi-file internal module boundary exists
- keep internal headers rare and intentional

Examples:

Good:
- `fm_uart.h` exposes stable UART module APIs
- `fm_uart_internal.h` exists only if multiple UART implementation units truly need shared internals

Bad:
- exposing helpers in the public header just because another file happened to need them once

---

## Variable naming

Keep variable names clear and proportional to scope.

Guidelines:
- short names for tight local scope are acceptable
- broader scope requires more explicit names
- booleans should read as booleans
- avoid cryptic abbreviations unless already established in the domain

Examples:

```c
bool is_ready;
uint32_t timeout_ms;
size_t rx_count;
```

---

## Macro naming

Use uppercase with module prefix for public or file-level constants or macros that behave as constants.

Examples:

```c
#define FM_UART_RX_BUFFER_SIZE    128U
#define FM_APP_TASK_PERIOD_MS     10U
```

Avoid function-like macros unless there is a strong reason.

Prefer `static inline` or real functions when appropriate.

---

## Enum and type naming

Use repository-consistent, module-scoped names.

Examples:

```c
typedef enum
{
    FM_UART_STATE_IDLE = 0,
    FM_UART_STATE_BUSY,
    FM_UART_STATE_ERROR
} FM_UartState_t;
```

Type names should be:
- explicit
- module-scoped
- not overly abbreviated

---

## Includes

Guidelines:

- include the module's own header first
- then standard headers
- then project dependencies
- avoid unused includes
- do not rely on indirect includes

Example:

```c
#include "fm_uart.h"

#include <stdbool.h>
#include <stdint.h>

#include "fm_time.h"
#include "stm32f4xx_hal_uart.h"
```

---

## Comments

Comment to clarify intent, assumptions, or non-obvious behavior.

Do not comment what the code already says clearly.

Good comment targets:

- why a sequence matters
- hardware-specific caveat
- ordering dependency
- protocol assumption
- temporary limitation with justification

Avoid noisy comments.

---

## Error handling

Use simple, explicit error handling.
Match existing repository style where it already exists.

Do not introduce complex error frameworks unless the repository already uses one.

When reporting or propagating errors:

- keep behavior predictable
- keep naming explicit
- keep ownership of recovery clear

---

## Responsibilities and boundaries

Do not mix these casually in one file:

- app orchestration
- board-specific control
- vendor init details
- reusable service logic
- hardware adaptation glue

If a file already mixes concerns, prefer containment first,
then split only when the task justifies it.

---

## Examples are reference, not law

`style-examples/` exists to show how conventions look in code.

Use it for:

- layout
- naming
- public and private split
- callback forwarding style
- wrapper shape

Do not use it as a reason to copy architecture blindly.

---

## Decision rule when unsure

When unsure between two acceptable options:

1. choose the option closer to existing repository code
2. choose the option with smaller API exposure
3. choose the option with clearer ownership
4. choose the option with less future cleanup cost

---

## Summary

Use these defaults unless a stronger repository-local pattern already exists:

- Public functions: `FM_Module_Action(void)`
- Private functions: `static void fm_module_action_(void);`
- HAL or framework callbacks: keep required external name, forward to private helper
- Wrappers: name by repository-facing behavior
- Public API: minimal
- Private helpers: local and static
- Headers: public surface only
- Source files: implementation details stay local
