# Reopen Check After Firmware Root Migration

## Purpose

Use this note after reopening VS Code from:

```text
D:\githubs\FMC-320U-FW\firmware
```

The goal is to verify that the firmware folder is now both:
- the STM32 workspace root
- the Git repository root

## What Changed

Git metadata was moved from:

```text
D:\githubs\FMC-320U-FW\.git
```

to:

```text
D:\githubs\FMC-320U-FW\firmware\.git
```

The repository contents were reindexed so paths now start at the firmware
project root. For example:

```text
src/product/fmc/fmc_units.h
```

not:

```text
firmware/src/product/fmc/fmc_units.h
```

## Known Good State

After migration, these checks passed:

```powershell
git rev-parse --show-toplevel
```

Expected:

```text
D:/githubs/FMC-320U-FW/firmware
```

```powershell
git status --short --branch
```

Expected after the migration commits:

```text
## main...origin/main [ahead 3]
```

Canonical STM32 build from the new root also passed:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
. .\tools\stm32cube-env.ps1
cube-cmake --preset Debug -DFM_ACTIVE_APP=fmc_model_units_test
cube-cmake --build --preset Debug
```

Observed result:

```text
-- Active app: fmc_model_units_test
-- Build files have been written to: D:/githubs/FMC-320U-FW/firmware/build/Debug
ninja: no work to do.
Build type: Debug
```

## Reopen Checklist

After reopening VS Code, verify from the integrated terminal:

```powershell
pwd
git rev-parse --show-toplevel
git status --short --branch
```

Expected:
- `pwd` should be `D:\githubs\FMC-320U-FW\firmware`
- Git top level should be `D:/githubs/FMC-320U-FW/firmware`
- `git status` should not show paths prefixed with `firmware/`

Then verify the STM32/CMake path:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
. .\tools\stm32cube-env.ps1
cube-cmake --preset Debug -DFM_ACTIVE_APP=fmc_model_units_test
cube-cmake --build --preset Debug
```

Optional cache sanity check:

```powershell
Get-Content build\Debug\CMakeCache.txt |
  Select-String "FM_ACTIVE_APP:|CMAKE_HOME_DIRECTORY|CMAKE_TOOLCHAIN_FILE"
```

Expected important lines:

```text
FM_ACTIVE_APP:UNINITIALIZED=fmc_model_units_test
CMAKE_HOME_DIRECTORY:INTERNAL=D:/githubs/FMC-320U-FW/firmware
CMAKE_TOOLCHAIN_FILE:FILEPATH=D:/githubs/FMC-320U-FW/firmware/cmake/gcc-arm-none-eabi.cmake
```

## Safety Notes

The old outer directory:

```text
D:\githubs\FMC-320U-FW
```

is now only a container folder. It is intentionally not a Git repository.

A physical backup of the previous Git metadata exists at:

```text
D:\githubs\FMC-320U-FW\.git.backup-before-firmware-root
```

A rescue branch also exists:

```text
backup/before-firmware-root
```

Do not delete either until the STM32 extension, Git operations, build, and debug
flow have all been confirmed from the reopened workspace.

## Prompt For Next Codex Session

```text
Estoy reabriendo el proyecto despues de migrar la raiz Git a firmware.
La carpeta correcta de trabajo ahora es:
D:\githubs\FMC-320U-FW\firmware

Por favor verifica que todo quedo bien sin modificar codigo:
1. lee AGENTS.md y docs/contexts/reopen_after_firmware_root_migration.md
2. confirma que git rev-parse --show-toplevel apunta a D:/githubs/FMC-320U-FW/firmware
3. confirma que git status no muestra paths con prefijo firmware/
4. ejecuta el build canonico:
   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
   . .\tools\stm32cube-env.ps1
   cube-cmake --preset Debug -DFM_ACTIVE_APP=fmc_model_units_test
   cube-cmake --build --preset Debug
5. revisa build\Debug\CMakeCache.txt para confirmar que CMAKE_HOME_DIRECTORY y
   CMAKE_TOOLCHAIN_FILE apuntan dentro de D:/githubs/FMC-320U-FW/firmware
6. reporta cualquier problema concreto y no hagas refactors ni cambios de codigo.
```
