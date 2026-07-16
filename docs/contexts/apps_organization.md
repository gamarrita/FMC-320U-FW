# Apps Organization Refactor

## Purpose

This context tracks the `src/apps/` organization refactor.

The goal is to keep selectable firmware profiles discoverable as the project
grows without introducing a large test framework or unnecessary app taxonomy.

## Decisions

- `src/apps/product/` owns product-oriented runnable profiles.
- `src/apps/bringups/` owns human-observed or hardware-facing validation apps.
- `src/apps/tests/` owns automatic UART `PASS`/`FAIL` regression apps.
- `src/apps/template/` remains a minimal copyable app reference.
- No `demos/` category is created yet.
- The old `panic_demo` behavior is treated as `bringups/debug_panic` because it
  validates debug, panic, error-handler, and fault paths.
- The old `fmc_model_units_test` behavior is treated as `tests/regression`
  because it now covers multiple authored pure modules.

## Validation Rule

After this refactor, app selection must still use the canonical build flow with
`FM_ACTIVE_APP`.

At minimum, validate:
- `tests/regression` builds, flashes, and reports UART `REGRESSION_TEST:PASS`
- representative bringups still build

## Validation Result

Validated after the refactor:
- cross references to old app paths were removed
- `tests/regression` builds, flashes, and reports UART `REGRESSION_TEST:PASS`
- `bringups/display_format_lcd`, `bringups/lcd`, `bringups/lcd_blink`,
  `bringups/debug_panic`, `product/main`, and `template` build

## Non-goals

- Do not introduce a generic test framework yet.
- Do not split the regression harness into suites in this refactor.
- Do not change hardware behavior or product logic.
