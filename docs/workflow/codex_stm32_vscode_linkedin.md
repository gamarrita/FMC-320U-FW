# Using Codex in a Real STM32 Firmware Workflow

I have been experimenting with Codex not as a code generator in isolation, but
as an engineering collaborator inside a real embedded firmware repository.

The project is an STM32-based firmware developed in VS Code, using the STM32
tooling bundle, CMake, selectable firmware apps, hardware bring-ups, regression
tests, flashing, and UART output as part of the normal development loop.

What I wanted to test was simple:

> Can an AI coding agent work inside the same constraints that a firmware
> engineer actually cares about?

Not just:

- generate a function
- explain some C code
- suggest a refactor

But instead:

- understand the repository structure
- respect generated and protected STM32 files
- keep the refactor small
- update CMake and documentation
- use the canonical build flow
- build multiple firmware app profiles
- flash the board
- read UART output
- confirm the firmware actually runs

## The concrete task

The repository had a `src/apps` folder that had grown organically. It contained
product apps, LCD bring-ups, panic/debug experiments, display-format validation,
and what started as a unit test but had become a broader regression test.

The request to Codex was to reorganize that layout without over-designing it,
verify cross references, update documentation, compile using the canonical
STM32 flow, and confirm the test firmware over UART.

## What Codex changed

The app layout was reorganized by role:

```text
src/apps/
  product/
    main/
  bringups/
    lcd/
    lcd_blink/
    display_format_lcd/
    debug_panic/
  tests/
    regression/
  template/
```

Some naming decisions mattered:

- `panic_demo` became `bringups/debug_panic`, because functionally it validates
  debug, panic, error-handler, and fault paths.
- `fmc_model_units_test` became `tests/regression`, because it now covers
  several authored firmware modules.
- No `demos/` folder was created yet, because nothing currently needed that
  category strongly enough.
- No new test framework was introduced. UART `PASS`/`FAIL` remained the right
  size for this stage of the project.

## The important part: validation

Codex used the repository's canonical STM32 build flow and compiled the
relevant firmware app profiles:

- `tests/regression`
- `bringups/display_format_lcd`
- `bringups/lcd`
- `bringups/lcd_blink`
- `bringups/debug_panic`
- `product/main`
- `template`

Then it flashed the STM32 target and captured UART output from the regression
firmware.

The board reported:

```text
DEBUG_INIT:MSG=ENABLED LED=ENABLED
REGRESSION_TEST:START
REGRESSION_TEST:INIT_DEFAULTS:PASS
REGRESSION_TEST:TOTALS:PASS
REGRESSION_TEST:RATE_WINDOWS:PASS
REGRESSION_TEST:VOLUME_VALUES:PASS
REGRESSION_TEST:DISPLAY_FORMAT_VALUES:PASS
REGRESSION_TEST:DISPLAY_FORMAT_ERROR_PATHS:PASS
REGRESSION_TEST:PASS
```

The agent did not just produce plausible code. It closed the loop:

```text
engineering intent
  -> repository analysis
  -> constrained refactor
  -> documentation update
  -> canonical STM32 build
  -> board flash
  -> UART regression PASS
```

## Takeaway

For firmware teams exploring AI coding agents, the interesting question is not
only whether the agent can write embedded C.

The better question is whether it can participate in the real firmware workflow
without breaking the assumptions that make the project build, flash, and run.

