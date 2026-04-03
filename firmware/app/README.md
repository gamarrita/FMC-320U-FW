# app

## Purpose
`app/` contains application behavior and use-case level logic.

For this first iteration, it hosts the minimal `hello world` flow in
[`fm_app_hello_world.c`](./fm_app_hello_world.c):
- decide when the heartbeat changes,
- keep the small application state,
- call lower layers through a narrow API.

## What `app/` should contain
- product-visible behavior,
- use-case level state machines,
- coordination of lower-layer capabilities.

## What `app/` should NOT contain
- direct HAL or Cube includes,
- register access,
- board pin names or Cube-generated symbols,
- code that must live in generated or regenerable files.

## Layer dependency
Expected direction for this repository:

`app -> bsp -> port -> cube/HAL`

For this first validation, `bsp/` is intentionally not used yet because the
current `hello world` only needs one LED and a time base. The current flow is:

`app -> port -> cube/HAL`

## Where this first hello world lives
- `app/`
  `fm_app_hello_world.c/.h` contains the non-blocking blink logic and timing.
- `port/`
  `fm_port_platform.c/.h` exposes the minimal platform primitives used by the
  app: `GetTickMs()` and `SetStatusLed()`.
- `cube/`
  The STM32Cube project still owns startup, clocks, GPIO/RTC init, link
  script, and the executable target. `main.c` only initializes Cube and calls
  into `port/` and `app/`.

## Build
Current executable target:

`firmware/cube/scratch/01_helloworld/01_hello_world`

From that directory:

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

Output:

`build/Debug/01_hello_world.elf`

## Run / flash
If `STM32_Programmer_CLI` is available in the environment, the current flow is:

```powershell
STM32_Programmer_CLI -c port=SWD mode=UR -w build/Debug/01_hello_world.elf -v -rst
```

That command programs the existing STM32 target and resets the MCU so the app
starts running.

## Current limitations
- The executable Cube project still lives in `cube/scratch/`; there is no
  official target under `cube/targets/` yet.
- LED mapping still lives in `port/` to keep this first step minimal.
- `bsp/` is still empty and should only be introduced once board-specific
  responsibilities become clearer than this single-LED example.
