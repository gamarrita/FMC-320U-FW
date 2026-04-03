# CONTRIBUTING.md

## Branches

Use:

- `feat/`
- `fix/`
- `refactor/`
- `perf/`
- `docs/`
- `test/`

Format:

```text
<type>/<short-description>
```

Examples:

```text
feat/pulse-filter
fix/exti-debounce
refactor/adc-basic-example
```

---

## Commits

Format:

```text
[AREA] type: message
```

Examples:

```text
[FIRMWARE] fix: prevent repeated pulse count on unstable edge
[FIRMWARE] refactor: align example private function naming
[DOCS] docs: simplify style examples and naming rules
```

Rules:
- imperative mood
- short and specific
- separate behavior changes from pure style changes when practical

---

## Pull Requests

Include:
- what changed
- why it changed
- how it was checked

Keep PRs focused.
Do not mix unrelated cleanup with functional change unless justified.

---

## Style and tooling

Before commit:
- run formatter
- check naming consistency
- keep public vs private naming aligned with `STYLE.md`
- avoid introducing new naming variants without updating the style doc

---

## Safety mindset

- prefer explicit behavior
- avoid hidden assumptions
- keep interrupt code minimal
- validate inputs at module boundaries
- keep state transitions easy to audit

---

# examples/README.md

```md
# Examples

This directory contains small style examples for FMC-320U.
Each example focuses on one case only.

## Content

- `baremetal/fm_main_simple.h/.c`
  - Minimal baremetal app with `Init()` and `Main()`.
- `baremetal/fm_gpio_poll.h/.c`
  - Simple polling-based input module.
- `rtos/fmx_kernel_basic.h/.c`
  - Minimal ThreadX integration.
- `rtos/fm_task_simple.h/.c`
  - Simple periodic task.
- `interrupts/fm_exti_flag.h/.c`
  - Minimal interrupt event handling.
- `drivers/fm_adc_basic.h/.c`
  - Simple ADC driver example.

## Naming model used in examples

- Public functions:
  - `FM_MODULE_Action()`
  - `FMX_MODULE_Action()`
- Private static functions:
  - `static void fm_module_action_(void);`
- Required external callbacks keep vendor/RTOS names.
```