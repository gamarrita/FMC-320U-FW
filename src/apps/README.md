# apps

## Intent

Contain selectable firmware applications used through `FM_ACTIVE_APP`.

Each folder below this tree represents one runnable firmware profile for the
same firmware base.

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

## Selecting The Active App

Use `FM_ACTIVE_APP` to choose which app folder is built.

Examples:
- `cube-cmake --fresh --preset Debug -DFM_ACTIVE_APP=product/main`
- `cube-cmake --fresh --preset Debug -DFM_ACTIVE_APP=template`
- `cube-cmake --fresh --preset Debug -DFM_ACTIVE_APP=bringups/debug_panic`
- `cube-cmake --fresh --preset Debug -DFM_ACTIVE_APP=bringups/lcd`
- `cube-cmake --fresh --preset Debug -DFM_ACTIVE_APP=bringups/lcd_blink`
- `cube-cmake --fresh --preset Debug -DFM_ACTIVE_APP=bringups/display_format_lcd`
- `cube-cmake --fresh --preset Debug -DFM_ACTIVE_APP=tests/regression`

The default app is defined in the repository `CMakeLists.txt`.
When switching between apps, prefer a fresh configure so the build cache does
not keep state from the previous app.
