# apps

## Intent
Contain selectable firmware applications used through `FM_ACTIVE_APP`.

Each folder under `src/apps/` represents one runnable application profile for the
same firmware base.

## Naming convention
- `main`
  Product-oriented application path.
- `template`
  Minimal copyable reference for creating a new app.
- `panic_demo`
  Minimal app used to validate the fatal stop path exposed by `fm_debug`.
- `*_bringup`
  Focused app used to bring up or validate one device, bus, or subsystem.
- `*_diag`
  Focused app used to isolate and reproduce a problem.
- `*_test`
  Use only when the app represents a repeatable test scenario, not a generic sandbox.

## Practical rule
Keep `src/apps/` flat until the number of applications makes discovery difficult.

Prefer app names that express purpose, not folder type.
Since the parent folder is already `src/apps/`, avoid redundant suffixes like `_app`.

## Minimal structure
Each app should normally contain:
- `CMakeLists.txt`
- `app_entry.h`
- `app_entry.c`

If the app has real logic beyond a tiny stub, add one app-local module such as:
- `fm_<name>.h`
- `fm_<name>.c`

`app_entry.c` should stay thin and delegate to the app-local module.

## Selecting the active app
Use `FM_ACTIVE_APP` to choose which app folder is built.

Examples:
- `cube-cmake --fresh --preset Debug -DFM_ACTIVE_APP=main`
- `cube-cmake --fresh --preset Debug -DFM_ACTIVE_APP=template`
- `cube-cmake --fresh --preset Debug -DFM_ACTIVE_APP=panic_demo`
- `cube-cmake --fresh --preset Debug -DFM_ACTIVE_APP=lcd_bringup`

The default app is defined in the repository `CMakeLists.txt`.
When switching between apps, prefer a fresh configure so the build cache does
not keep state from the previous app.
