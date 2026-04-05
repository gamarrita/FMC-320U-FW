# STM32Cube build environment

This repo uses the STM32 VS Code extensions as the source of `cube` and `cube-cmake`, but normal shells do not inherit that environment automatically.

This first pass only defines a reproducible build contract outside the IDE. It does not change the workspace clangd setup.

## Reproduce the original failure

PowerShell:

```powershell
where.exe cube
where.exe cube-cmake
where.exe cmake
echo $env:CUBE_BUNDLE_PATH
cmake --preset Debug
```

Git Bash:

```bash
where.exe cube
where.exe cube-cmake
where.exe cmake
echo "$CUBE_BUNDLE_PATH"
cmake --preset Debug
```

Expected failure in a clean shell:
- `cube`, `cube-cmake`, or `cmake` are not found.
- `CUBE_BUNDLE_PATH` is empty.
- `cmake --preset Debug` fails because the STM32 toolchain environment was never prepared.

## Bootstrap the same environment outside VS Code

PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
```

`Set-ExecutionPolicy -Scope Process Bypass` only affects the current PowerShell process. Use it when the default policy blocks local `.ps1` scripts.

Git Bash:

```bash
source tools/stm32cube-env.sh
```

Both scripts:
- discover `cube.exe` and `cube-cmake.exe` from the installed STM32 VS Code extensions without hardcoding exact extension versions
- read `cube_bundle_path` and `cmsis_pack_root` from `cube`
- read the exported toolchain environment from `cube --resolve cmake`
- export `CUBE_BUNDLE_PATH`, `CMSIS_PACK_ROOT`, `CMAKE_ROOT`, `NINJA_ROOT`, `TOOLCHAIN_ROOT`, `GCC_TOOLCHAIN_ROOT`, `CUBE_PROGRAMMER_PATH`, and `PATH`

## Canonical build commands

Validated canonical entrypoint:

```powershell
cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

Comparison entrypoint:

```powershell
cube cmake --preset Debug
cube cmake --build --preset Debug
```

Why `cube-cmake` is canonical in this pass:
- the STM32 workspace already points `cmake.cmakePath` to `cube-cmake`
- `cube-cmake --preset Debug` and `cube cmake --preset Debug` resolve the same generator, toolchain file, CMake executable, and Ninja path in `CMakeCache.txt`
- keeping `cube-cmake` canonical matches the actual extension entrypoint more directly

Direct `cmake --preset Debug` and `cmake --build --preset Debug` are supported only after bootstrap, but they are not treated as the canonical STM32 entrypoint.

## Discover absolute paths from the CLI

Once `cube` is reachable:

```powershell
cube --get-current-value cube_bundle_path
cube --get-current-value cmsis_pack_root
cube --resolve cmake
```

Once the bootstrap is loaded:

```powershell
cube-cmake --version
cube cmake --version
```

Useful facts:
- `cube.exe` and `cube-cmake.exe` live under versioned `.vscode/extensions/...` directories.
- STM32 bundles live under `%LOCALAPPDATA%\stm32cube\bundles`.
- `cube --resolve cmake` is the most reliable source for the toolchain paths that the STM32 tooling expects.

## Equivalence checks

Treat the CLI environment as equivalent to STM32 tooling only when all of these match:
- same preset: `Debug`
- same generator: `Ninja`
- same toolchain file: `cmake/gcc-arm-none-eabi.cmake`
- same effective `CMAKE_COMMAND`, `CMAKE_MAKE_PROGRAM`, and compiler paths in `build/Debug/CMakeCache.txt`
- same bundle roots reported by `cube --resolve cmake`
- same resulting build artifact characteristics after a clean build, at minimum the presence and size of `build/Debug/fmc-320u-v2.elf`

Useful checks:

```powershell
cube --resolve cmake
cube-cmake --preset Debug
Get-Content build/Debug/CMakeCache.txt | Select-String "CMAKE_GENERATOR:INTERNAL|CMAKE_MAKE_PROGRAM:FILEPATH|CMAKE_C_COMPILER:FILEPATH|CMAKE_COMMAND:INTERNAL|CMAKE_TOOLCHAIN_FILE:FILEPATH"
cube cmake --preset Debug
Get-Content build/Debug/CMakeCache.txt | Select-String "CMAKE_GENERATOR:INTERNAL|CMAKE_MAKE_PROGRAM:FILEPATH|CMAKE_C_COMPILER:FILEPATH|CMAKE_COMMAND:INTERNAL|CMAKE_TOOLCHAIN_FILE:FILEPATH"
cube-cmake --build --preset Debug
(Get-Item build/Debug/fmc-320u-v2.elf).Length
cube cmake --build --preset Debug
(Get-Item build/Debug/fmc-320u-v2.elf).Length
```

## Optional persistent Windows setup

The repo bootstrap is the stable option. It re-discovers the current extension versions each time.

If you still want a shell that is ready without sourcing the repo scripts:
- set user environment variables for `CUBE_BUNDLE_PATH` and `CMSIS_PACK_ROOT`
- optionally set `GCC_TOOLCHAIN_ROOT` to the currently selected GNU bundle `bin` directory
- add the current `cube.exe` directory and `cube-cmake.exe` directory to your user `PATH`

Important caveat:
- `cube.exe` and `cube-cmake.exe` are stored under versioned VS Code extension folders, so a global `PATH` entry may need to be updated when the STM32 extensions are upgraded.
- Because of that, the repo scripts are the recommended source of truth for agents and humans.
