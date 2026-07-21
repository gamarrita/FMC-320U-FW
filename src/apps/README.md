# apps

## Intent

Contain selectable firmware applications used through `FM_ACTIVE_APP`.

Each folder below this tree represents one runnable firmware profile for the
same firmware base.

The active runtime is ThreadX. The selected app still provides
`APP_ENTRY_Run()`, but that entry point is called from the shared ThreadX app
harness after the kernel starts. `APP_ENTRY_Run()` may block forever.

## Categories

- `product/`
  Firmware intended to become or approximate the real product application.
- `bringups/`
  Human-observed or hardware-facing validation apps. These may require looking
  at UART, LEDs, LCD glass, debugger state, or external instruments.
- `tests/`
  Automatic regression apps. These should report deterministic `PASS`/`FAIL`
  output over UART.
- `template/`
  Minimal copyable reference for creating a new selectable app.

Do not add a new top-level category for a single app. Prefer one of the current
categories until a distinct family has at least a few likely members.

## App Structure

Each selectable app should normally contain:
- `CMakeLists.txt`
- `app_entry.h`
- `app_entry.c`

If the app has real logic beyond a tiny stub, add one app-local module such as:
- `fm_<name>.h`
- `fm_<name>.c`

`app_entry.c` should stay thin and delegate to the app-local module.

## ThreadX Harness

`fm_app_threadx.*` adapts the selected app to the CubeMX ThreadX bootstrap. It
creates one generic app task and calls `APP_ENTRY_Run()` from that task.

This harness preserves the selectable-app workflow during the ThreadX bootstrap
baseline. For `product/main`, this same `FM_APP` thread runs
`FM_MAIN_Main()` and is the owner loop for the live `fmc_runtime_t`; that
function does not return during normal operation. Keyboard ISRs only publish
events into the app-level queue, and only the owner loop dispatches them to
`fmc_runtime`.

Add extra ThreadX threads only when a concrete concurrent responsibility needs
separate blocking, priority, or lifetime ownership.

Generated startup code must reach `MX_ThreadX_Init()` before app code runs.
Do not call a selected app directly from generated `main.c`.

## Selecting The Active App

Edit `FM_ACTIVE_APP_DEFAULT` in the repository `CMakeLists.txt` to choose which
app folder is built by the canonical flow.

Examples:
- `set(FM_ACTIVE_APP_DEFAULT "product/main")`
- `set(FM_ACTIVE_APP_DEFAULT "template")`
- `set(FM_ACTIVE_APP_DEFAULT "bringups/debug_panic")`
- `set(FM_ACTIVE_APP_DEFAULT "bringups/lcd")`
- `set(FM_ACTIVE_APP_DEFAULT "bringups/lcd_blink")`
- `set(FM_ACTIVE_APP_DEFAULT "bringups/display_format_lcd")`
- `set(FM_ACTIVE_APP_DEFAULT "bringups/keyboard_input")`
- `set(FM_ACTIVE_APP_DEFAULT "tests/regression")`

Do not select the normal app through a cached `-DFM_ACTIVE_APP=...` configure
argument. Multi-app validation should use an isolated build directory or restore
the canonical app selection before handing the workspace back.
