# STM32 Build Environment for This Repo

[Go to Spanish version](#version-en-espanol)

This document is the short operational reference for using this repo's STM32 build outside the IDE.

Its purpose is not to explain the whole history of the problem or to serve as a prompt for Codex. Its role is more concrete:
- make explicit the minimum environment the project needs in order to build
- show how to reproduce the typical failure from an unprepared shell
- explain how to load the repo bootstrap
- define the canonical build entrypoint for this project
- provide a practical way to verify that the terminal and the STM32 tooling are using the same build contract

If you want a fuller explanation of the problem and the reasoning behind this setup, see:
- `docs/build_canonical_new_projects.md`

## What problem this document solves

This repo uses STM32 VS Code extensions as the source of `cube` and `cube-cmake`, but a normal terminal does not automatically inherit that environment.

That leads to a situation that looks contradictory:
- the project builds from VS Code
- the terminal cannot find `cmake`, `cube`, `cube-cmake`, or the ARM toolchain

The cause is usually not "CMake is not installed" in a generic sense. The real issue is that the build depends on the STM32 environment resolved by the workspace tooling, and that environment is not always exported to the current shell.

This document explains how to use that same environment from the terminal so the build is reproducible for both humans and agents.

## How to reproduce the original failure

In PowerShell:

```powershell
where.exe cube
where.exe cube-cmake
where.exe cmake
echo $env:CUBE_BUNDLE_PATH
cmake --preset Debug
```

In Git Bash:

```bash
where.exe cube
where.exe cube-cmake
where.exe cmake
echo "$CUBE_BUNDLE_PATH"
cmake --preset Debug
```

In a clean shell, one or more of these outcomes is expected:
- `cube`, `cube-cmake`, or `cmake` are not found
- `CUBE_BUNDLE_PATH` is empty
- `cmake --preset Debug` fails because the STM32 environment has not been prepared yet

## How to load the same environment outside VS Code

In PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
```

`Set-ExecutionPolicy -Scope Process Bypass` only affects the current PowerShell session. Use it when the local policy blocks `.ps1` scripts.

In Git Bash:

```bash
source tools/stm32cube-env.sh
```

Both scripts do the minimum needed to reproduce the repo's STM32 environment:
- discover `cube.exe` and `cube-cmake.exe` from the installed STM32 extensions without hardcoding exact versions
- query `cube_bundle_path` and `cmsis_pack_root` through `cube`
- read the environment exported by `cube --resolve cmake`
- export `CUBE_BUNDLE_PATH`, `CMSIS_PACK_ROOT`, `CMAKE_ROOT`, `NINJA_ROOT`, `TOOLCHAIN_ROOT`, `GCC_TOOLCHAIN_ROOT`, `CUBE_PROGRAMMER_PATH`, and `PATH`

## Canonical build entrypoint

The validated canonical entrypoint for this repo is:

```powershell
cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

This alternative was also compared:

```powershell
cube cmake --preset Debug
cube cmake --build --preset Debug
```

In this first pass, `cube-cmake` was kept as canonical because:
- the workspace already points to `cube-cmake` through `.vscode/settings.json`
- `cube-cmake --preset Debug` and `cube cmake --preset Debug` resolved the same generator, toolchain file, CMake executable, and Ninja path in `CMakeCache.txt`
- `cube-cmake` reflects the current workspace contract more directly

`cmake --preset Debug` and `cmake --build --preset Debug` are also valid after loading the bootstrap, but they are not treated as the main canonical entrypoint of the repo.

## How to discover the real paths from the CLI

Once `cube` is accessible:

```powershell
cube --get-current-value cube_bundle_path
cube --get-current-value cmsis_pack_root
cube --resolve cmake
```

Once the bootstrap is already loaded:

```powershell
cube-cmake --version
cube cmake --version
```

Important points:
- `cube.exe` and `cube-cmake.exe` live inside versioned `.vscode/extensions/...` directories
- STM32 bundles live under `%LOCALAPPDATA%\stm32cube\bundles`
- `cube --resolve cmake` is the most reliable source of truth for the tools and variables the STM32 tooling actually uses

## How to verify that the environment matches STM32 tooling

Treat the terminal as aligned with the STM32 tooling only if, at minimum, these match:
- the same preset: `Debug`
- the same generator: `Ninja`
- the same toolchain file: `cmake/gcc-arm-none-eabi.cmake`
- the same `CMAKE_COMMAND`
- the same `CMAKE_MAKE_PROGRAM`
- the same effective compilers
- the same bundle root reported by `cube --resolve cmake`
- comparable characteristics of the final artifact, at minimum the presence and size of `build/Debug/fmc-320u-v2.elf`

Useful verification commands:

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

The recommended option is still the repo bootstrap, because it rediscovers the installed versions each time and avoids relying on stale paths.

If you still want a shell that is ready without running the repo scripts:
- define `CUBE_BUNDLE_PATH` and `CMSIS_PACK_ROOT` as user environment variables
- optionally define `GCC_TOOLCHAIN_ROOT` pointing to the `bin` directory of the active GNU bundle
- add the `cube.exe` directory and the `cube-cmake.exe` directory to the user `PATH`

Important warning:
- `cube.exe` and `cube-cmake.exe` live in versioned VS Code extension directories
- if the STM32 extension is upgraded, those paths may change
- that is why the repo bootstrap remains the recommended source of truth for both humans and agents

---

## Version en espanol

# Entorno de build STM32 del repo

Este documento es la referencia operativa corta para usar el build STM32 de este repo fuera del IDE.

Su intención no es explicar toda la historia del problema ni servir como prompt para Codex. Su objetivo es más concreto:
- dejar explícito cuál es el entorno mínimo que necesita el proyecto para compilar
- mostrar cómo reproducir el fallo típico de una shell sin preparar
- indicar cómo cargar el bootstrap del repo
- fijar cuál es la entrada canónica del build en este proyecto
- dejar un criterio práctico para comprobar que la terminal y el tooling STM32 están usando el mismo contrato de build

Si buscas una explicación más completa del problema y del razonamiento detrás de este esquema, consulta:
- `docs/build_canonical_new_projects.md`

## Qué problema resuelve este documento

Este repo usa extensiones STM32 de VS Code como fuente de `cube` y `cube-cmake`, pero una terminal común no hereda automáticamente ese entorno.

Por eso puede pasar algo que parece contradictorio:
- el proyecto compila desde VS Code
- la terminal no encuentra `cmake`, `cube`, `cube-cmake` o el toolchain ARM

La causa no suele ser "falta instalar CMake" de manera genérica. El problema real es que el build depende del entorno STM32 que resuelve el tooling del workspace, y ese entorno no siempre está exportado a la shell actual.

Este documento deja claro cómo usar ese mismo entorno desde terminal para que el build sea reproducible tanto por humanos como por agentes.

## Cómo reproducir el fallo original

En PowerShell:

```powershell
where.exe cube
where.exe cube-cmake
where.exe cmake
echo $env:CUBE_BUNDLE_PATH
cmake --preset Debug
```

En Git Bash:

```bash
where.exe cube
where.exe cube-cmake
where.exe cmake
echo "$CUBE_BUNDLE_PATH"
cmake --preset Debug
```

En una shell limpia es esperable que ocurra una o más de estas cosas:
- `cube`, `cube-cmake` o `cmake` no aparecen
- `CUBE_BUNDLE_PATH` está vacía
- `cmake --preset Debug` falla porque el entorno STM32 todavía no fue preparado

## Cómo cargar el mismo entorno fuera de VS Code

En PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
```

`Set-ExecutionPolicy -Scope Process Bypass` solo afecta la sesión actual de PowerShell. Úsalo cuando la política local impida ejecutar scripts `.ps1`.

En Git Bash:

```bash
source tools/stm32cube-env.sh
```

Ambos scripts hacen lo mínimo necesario para reproducir el entorno STM32 del repo:
- descubren `cube.exe` y `cube-cmake.exe` desde las extensiones STM32 instaladas, sin hardcodear versiones exactas
- consultan `cube_bundle_path` y `cmsis_pack_root` usando `cube`
- leen el entorno exportado por `cube --resolve cmake`
- exportan `CUBE_BUNDLE_PATH`, `CMSIS_PACK_ROOT`, `CMAKE_ROOT`, `NINJA_ROOT`, `TOOLCHAIN_ROOT`, `GCC_TOOLCHAIN_ROOT`, `CUBE_PROGRAMMER_PATH` y `PATH`

## Entrada canónica del build

La entrada canónica validada para este repo es:

```powershell
cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

También se comparó esta variante:

```powershell
cube cmake --preset Debug
cube cmake --build --preset Debug
```

En esta primera pasada se dejó `cube-cmake` como entrada canónica porque:
- el workspace ya apunta a `cube-cmake` mediante `.vscode/settings.json`
- `cube-cmake --preset Debug` y `cube cmake --preset Debug` resolvieron el mismo generator, toolchain file, ejecutable de CMake y ruta de Ninja en `CMakeCache.txt`
- usar `cube-cmake` refleja de forma más directa el contrato actual del workspace

`cmake --preset Debug` y `cmake --build --preset Debug` también son válidos después de cargar el bootstrap, pero no se consideran la entrada canónica principal del repo.

## Cómo descubrir los paths reales desde CLI

Cuando `cube` ya es accesible:

```powershell
cube --get-current-value cube_bundle_path
cube --get-current-value cmsis_pack_root
cube --resolve cmake
```

Cuando el bootstrap ya está cargado:

```powershell
cube-cmake --version
cube cmake --version
```

Puntos importantes:
- `cube.exe` y `cube-cmake.exe` viven dentro de carpetas versionadas de `.vscode/extensions/...`
- los bundles STM32 viven bajo `%LOCALAPPDATA%\stm32cube\bundles`
- `cube --resolve cmake` es la fuente de verdad más confiable para saber qué herramientas y variables usa realmente el tooling STM32

## Cómo comprobar que el entorno es equivalente al tooling STM32

Considera que la terminal quedó alineada con el tooling STM32 solo si coinciden, como mínimo:
- el mismo preset: `Debug`
- el mismo generator: `Ninja`
- el mismo toolchain file: `cmake/gcc-arm-none-eabi.cmake`
- el mismo `CMAKE_COMMAND`
- el mismo `CMAKE_MAKE_PROGRAM`
- los mismos compiladores efectivos
- la misma raíz de bundles reportada por `cube --resolve cmake`
- características comparables del artefacto final, al menos la presencia y tamaño de `build/Debug/fmc-320u-v2.elf`

Comandos útiles de verificación:

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

## Configuración persistente opcional en Windows

La opción recomendada sigue siendo el bootstrap del repo, porque vuelve a descubrir las versiones instaladas cada vez y evita depender de rutas viejas.

Si aun así quieres una shell lista sin ejecutar los scripts del repo:
- define `CUBE_BUNDLE_PATH` y `CMSIS_PACK_ROOT` como variables de entorno de usuario
- opcionalmente define `GCC_TOOLCHAIN_ROOT` apuntando al `bin` del bundle GNU activo
- agrega al `PATH` de usuario la carpeta de `cube.exe` y la de `cube-cmake.exe`

Advertencia importante:
- `cube.exe` y `cube-cmake.exe` viven en carpetas versionadas de extensiones de VS Code
- si la extensión STM32 se actualiza, esas rutas pueden cambiar
- por eso el bootstrap del repo sigue siendo la fuente de verdad recomendada para humanos y agentes
