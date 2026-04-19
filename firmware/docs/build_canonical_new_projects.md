# Reusable STM32 Canonical Build Guide for New Projects

[Go to Spanish version](#version-en-espanol)

Operational source of truth for this repo:
- `docs/stm32cube-cli-workflow.md`

This document explains which problem was solved by making the STM32 environment reproducible outside the IDE, how to recognize that problem from a normal shell, and how to reuse the same pattern in new STM32 projects.

The central idea is this:
- the problem was not only "missing `cmake`"
- the real build depends on the tooling resolved by STM32 from VS Code
- a shell or an agent can fail even when the build works from the IDE

## 1. What problem was solved

In this repo, VS Code can build using the STM32 tooling even when a normal terminal cannot see that same environment.

That happens because the real flow does not depend only on finding an arbitrary `cmake.exe`. It also depends on:
- `cube.exe`
- `cube-cmake.exe`
- STM32 bundles installed locally
- variables and paths resolved by `cube`
- an ARM toolchain accessible from the shell

In other words:
- the IDE may have enough context to build
- a clean PowerShell session may not inherit that context
- an agent should not assume the system is ready just because the project builds from VS Code

What was solved here was making explicit:
- how to detect that mismatch
- how to load the same environment outside the IDE
- what the canonical build entrypoint is in this repo
- how a human should operate and how an agent should operate

## 2. How to reproduce the original problem

Open a normal PowerShell session, without loading the repo STM32 bootstrap, and run:

```powershell
where.exe cube
where.exe cube-cmake
where.exe cmake
where.exe ninja
where.exe arm-none-eabi-gcc
echo $env:CUBE_BUNDLE_PATH
cmake --preset Debug
```

On this machine, the observed result was:

```text
INFO: Could not find files for the given pattern(s).
INFO: Could not find files for the given pattern(s).
INFO: Could not find files for the given pattern(s).
INFO: Could not find files for the given pattern(s).
INFO: Could not find files for the given pattern(s).
cmake : The term 'cmake' is not recognized as the name of a cmdlet, function, script file, or operable program.
```

That reproduces the real problem:
- the shell cannot see `cube`
- the shell cannot see `cube-cmake`
- the shell cannot see `cmake`, `ninja`, or `arm-none-eabi-gcc`
- `CUBE_BUNDLE_PATH` is empty

Conclusion:
- this does not necessarily mean STM32 is installed incorrectly
- it means the shell does not share the environment used by the STM32 tooling inside VS Code

## 3. What to check first

Before assuming everything can be fixed by adding a `cmake.exe` to `PATH`, it helps to separate possible causes.

### `cmake` is not visible

The current shell cannot find `cmake`, even though it may exist inside the STM32 bundles.

### `cube-cmake` is not visible

The STM32 extension may be installed in VS Code, but the terminal does not have the versioned directory containing `cube-cmake.exe` in `PATH`.

### `CUBE_BUNDLE_PATH` is not exported

The STM32 tooling may know the bundle, but the variable may not be exported in the current shell.

### The IDE resolves the environment internally

The STM32 VS Code extension can resolve `cube`, `cube-cmake`, bundles, and toolchain without exposing all of that to an external terminal.

## 4. How to inspect the real STM32 tooling

Once `cube` is accessible, the source of truth for the environment is not `where.exe`, but the STM32 resolver.

Useful commands:

```powershell
cube --get-current-value cube_bundle_path
cube --get-current-value cmsis_pack_root
cube --resolve cmake
```

What each command tells you:

- `cube --get-current-value cube_bundle_path`
  Returns the STM32 bundle root used by the tooling.
- `cube --get-current-value cmsis_pack_root`
  Returns the CMSIS pack root configured for that environment.
- `cube --resolve cmake`
  Shows the effective `cmake.exe` and the variables exported by STM32 to invoke it.

Important note:
- in this iteration, `cube --resolve ninja` was not treated as a required documented command because it was not needed to prove the problem or justify the canonical entrypoint
- when a command was not validated, it should not be promoted as a central part of the contract

## 5. Before and after the bootstrap

The repo already includes an explicit PowerShell bootstrap:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
```

After loading it, you can verify the environment with:

```powershell
where.exe cube
where.exe cube-cmake
where.exe cmake
where.exe ninja
where.exe arm-none-eabi-gcc
echo $env:CUBE_BUNDLE_PATH
cube --get-current-value cube_bundle_path
cube --resolve cmake
```

On this machine, after the bootstrap, the observed output was:

```text
C:\Users\dhs\.vscode\extensions\stmicroelectronics.stm32cube-ide-core-1.2.1-win32-x64\resources\binaries\win32\x86_64\cube.exe
C:\Users\dhs\.vscode\extensions\stmicroelectronics.stm32cube-ide-build-cmake-1.44.0-win32-x64\resources\cube-cmake\win32\x86_64\cube-cmake.exe
C:\Users\dhs\AppData\Local\stm32cube\bundles\cmake\4.2.3+st.1\bin\cmake.exe
C:\Users\dhs\AppData\Local\stm32cube\bundles\ninja\1.13.2+st.1\bin\ninja.exe
C:\Users\dhs\AppData\Local\stm32cube\bundles\gnu-tools-for-stm32\14.3.1+st.2\bin\arm-none-eabi-gcc.exe
C:\Users\dhs\AppData\Local\stm32cube\bundles
cube_bundle_path=C:\Users\dhs\AppData\Local\stm32cube\bundles
```

What matters about that result:
- the effective `PATH` no longer depends on guessing paths manually
- `cube` remains the source of truth for bundle and environment resolution
- the exact paths may change with another extension or bundle version
- the correct mechanism is the repo bootstrap, not hardcoded versions

## 6. What the canonical build entrypoint is in this repo

This project did not leave the decision abstract. These two entrypoints were compared:

```powershell
cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

against:

```powershell
cube cmake --preset Debug
cube cmake --build --preset Debug
```

The validated canonical entrypoint for this repo is:

```powershell
cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

Concrete reasons:
- `.vscode/settings.json` uses `cmake.cmakePath = cube-cmake`
- `CMakePresets.json` defines `Debug` and `Release` with generator `Ninja`
- both entrypoints resolved the same effective `CMakeCache.txt` for:
  - `CMAKE_GENERATOR = Ninja`
  - `CMAKE_TOOLCHAIN_FILE = cmake/gcc-arm-none-eabi.cmake`
  - `CMAKE_COMMAND` pointing to the STM32 bundle `cmake.exe`
  - `CMAKE_MAKE_PROGRAM` pointing to the STM32 bundle `ninja.exe`
- both builds finished on this machine with the same `build/Debug/fmc-320u-v2.elf`
- the observed artifact size was `1807308` bytes in both cases

Practical conclusion:
- `cube-cmake` is the main entrypoint because it matches the current workspace contract more directly
- `cube cmake` remains documented as a valid comparison path
- `cmake --preset Debug` remains an equivalent path only after the STM32 environment has been loaded

## 7. How a human builds this project

Recommended PowerShell flow:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

If you want to inspect the environment before configuring:

```powershell
where.exe cube
where.exe cube-cmake
where.exe cmake
where.exe ninja
where.exe arm-none-eabi-gcc
cube --get-current-value cube_bundle_path
cube --resolve cmake
```

If the bootstrap is already loaded, you can also use:

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

But in this repo that is treated as a post-bootstrap equivalent path, not as the canonical entrypoint.

## 8. How an agent should build this project

An agent should not assume that because the project builds in VS Code, the shell is ready.

Recommended sequence:

### Step 1. Detect whether the shell is clean or already bootstrapped

```powershell
where.exe cube
where.exe cube-cmake
where.exe cmake
where.exe arm-none-eabi-gcc
echo $env:CUBE_BUNDLE_PATH
```

### Step 2. If the environment is missing, load the repo bootstrap

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
```

### Step 3. Verify the STM32 resolver

```powershell
cube --get-current-value cube_bundle_path
cube --get-current-value cmsis_pack_root
cube --resolve cmake
```

### Step 4. Build using the canonical repo entrypoint

```powershell
cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

### Step 5. Use direct `cmake` only as a later equivalent path

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

Practical rule for agents:
- do not assume that an arbitrary `cmake.exe` represents the correct STM32 tooling
- do not assume that `CUBE_BUNDLE_PATH` already exists in the shell
- do not assume that the terminal and the IDE share the same environment
- prefer the repo bootstrap first and the documented canonical entrypoint second

## 9. How to replicate this pattern in a new STM32 project

If you want to apply this same idea to another STM32 project, the minimum pattern is:

### Define the build contract

The repo should explicitly define:
- presets in `CMakePresets.json`
- the expected generator
- the expected toolchain file
- the canonical build entrypoint

### Avoid hardcoded paths

It is not a good idea to hardcode in docs or scripts:
- exact extension versions
- exact bundle versions
- absolute paths that change on each update

Prefer:
- discovering `cube.exe` and `cube-cmake.exe` from installed extensions
- using `cube --get-current-value ...`
- using `cube --resolve ...` as the environment source of truth

### Include repo bootstrap

The repo should include bootstrap scripts so that:
- a person can open a shell and build
- an agent can load the same environment before acting
- the flow is reproducible outside the IDE

### Compare entrypoints before declaring one canonical

Do not assume the right entrypoint will always be the same in every project.

At minimum, compare:
- the entrypoint used by the workspace or extension
- an equivalent alternative resolved by `cube`

Then decide the canonical one using evidence:
- same preset
- same generator
- same toolchain file
- same `CMAKE_COMMAND`
- same `CMAKE_MAKE_PROGRAM`
- comparable final artifact

## 10. Limits and warnings

This document uses examples verified on this machine, but two levels must be separated.

What is stable:
- the underlying problem
- the need for bootstrap outside the IDE
- the use of `cube` as the source of truth for the STM32 environment
- the criterion for declaring a canonical build entrypoint

What may change between installations:
- the STM32 extension version
- the STM32 bundle version
- absolute paths under `.vscode\extensions`
- absolute paths under `%LOCALAPPDATA%\stm32cube\bundles`
- the exact artifact size if sources, flags, or tooling change

That is why the correct rule is not to copy exact versions, but to preserve the mechanism:
- repo bootstrap
- STM32 resolver
- documented canonical entrypoint
- explicit equivalence criteria

---

## Version en espanol

# Guia reusable para dejar un build STM32 canonico en proyectos nuevos

Fuente operativa de verdad para este repo:
- `docs/stm32cube-cli-workflow.md`

Este documento explica qué problema resolvió el plan para hacer reproducible el entorno STM32 fuera del IDE, cómo reconocer ese problema en una shell común, y cómo repetir el mismo esquema en proyectos STM32 nuevos.

La idea central es esta:
- el problema no era solamente "falta `cmake`"
- el build real del proyecto depende del tooling que resuelve STM32 desde VS Code
- una shell o un agente pueden fallar aunque el build funcione desde el IDE

## 1. Qué problema se resolvió

En este repo, VS Code puede compilar usando el tooling STM32 aunque una terminal común no tenga visible ese mismo entorno.

Eso pasa porque el flujo real no depende solo de encontrar un `cmake.exe` cualquiera. También depende de:
- `cube.exe`
- `cube-cmake.exe`
- bundles STM32 instalados localmente
- variables y paths resueltos por `cube`
- toolchain ARM accesible desde la shell

En otras palabras:
- el IDE puede tener suficiente contexto para compilar
- una PowerShell limpia puede no heredar ese contexto
- un agente no debería asumir que el sistema ya está listo solo porque el proyecto compila desde VS Code

Lo que resolvimos fue dejar explícito:
- cómo detectar ese desfase
- cómo cargar el mismo entorno fuera del IDE
- cuál es la entrada canónica de build en este repo
- cómo debe operar una persona y cómo debe operar un agente

## 2. Cómo reproducir el problema original

Abre una PowerShell común, sin haber cargado el bootstrap STM32 del repo, y ejecuta:

```powershell
where.exe cube
where.exe cube-cmake
where.exe cmake
where.exe ninja
where.exe arm-none-eabi-gcc
echo $env:CUBE_BUNDLE_PATH
cmake --preset Debug
```

En esta máquina, el resultado observado fue:

```text
INFO: Could not find files for the given pattern(s).
INFO: Could not find files for the given pattern(s).
INFO: Could not find files for the given pattern(s).
INFO: Could not find files for the given pattern(s).
INFO: Could not find files for the given pattern(s).
cmake : The term 'cmake' is not recognized as the name of a cmdlet, function, script file, or operable program.
```

Eso reproduce el problema real:
- la shell no ve `cube`
- la shell no ve `cube-cmake`
- la shell no ve `cmake`, `ninja` ni `arm-none-eabi-gcc`
- `CUBE_BUNDLE_PATH` está vacío

Conclusión:
- el problema no es necesariamente que STM32 esté mal instalado
- el problema es que la shell no comparte el entorno que el tooling STM32 usa dentro de VS Code

## 3. Qué revisar primero

Antes de asumir que todo se arregla agregando un `cmake.exe` al `PATH`, conviene separar causas posibles.

### `cmake` no visible

La shell actual no encuentra `cmake`, aunque exista dentro de los bundles STM32.

### `cube-cmake` no visible

La extensión STM32 puede estar instalada en VS Code, pero la terminal no tiene en `PATH` la carpeta versionada donde vive `cube-cmake.exe`.

### `CUBE_BUNDLE_PATH` no exportada

El tooling STM32 puede conocer el bundle, pero la variable puede no estar exportada en la shell actual.

### El IDE resuelve entorno internamente

El complemento STM32 de VS Code puede resolver `cube`, `cube-cmake`, bundles y toolchain sin exponer todo eso a una terminal externa.

## 4. Cómo inspeccionar el tooling STM32 real

Cuando `cube` ya es accesible, la fuente de verdad del entorno no es `where.exe`, sino el resolvedor STM32.

Comandos útiles:

```powershell
cube --get-current-value cube_bundle_path
cube --get-current-value cmsis_pack_root
cube --resolve cmake
```

Qué te dice cada comando:

- `cube --get-current-value cube_bundle_path`
  Devuelve la raíz del bundle STM32 que el tooling está usando.
- `cube --get-current-value cmsis_pack_root`
  Devuelve la raíz de packs CMSIS configurada para ese entorno.
- `cube --resolve cmake`
  Muestra el `cmake.exe` efectivo y las variables exportadas que STM32 usa para invocarlo.

Nota importante:
- en esta iteración no tomamos `cube --resolve ninja` como requisito documental, porque no fue necesario para demostrar el problema ni para justificar la entrada canónica
- cuando un comando no fue validado, no conviene promoverlo como parte central del contrato

## 5. Antes y después del bootstrap

El repo ya tiene un bootstrap explícito para PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
```

Después de cargarlo, puedes verificar el entorno con:

```powershell
where.exe cube
where.exe cube-cmake
where.exe cmake
where.exe ninja
where.exe arm-none-eabi-gcc
echo $env:CUBE_BUNDLE_PATH
cube --get-current-value cube_bundle_path
cube --resolve cmake
```

En esta máquina, después del bootstrap, se observó:

```text
C:\Users\dhs\.vscode\extensions\stmicroelectronics.stm32cube-ide-core-1.2.1-win32-x64\resources\binaries\win32\x86_64\cube.exe
C:\Users\dhs\.vscode\extensions\stmicroelectronics.stm32cube-ide-build-cmake-1.44.0-win32-x64\resources\cube-cmake\win32\x86_64\cube-cmake.exe
C:\Users\dhs\AppData\Local\stm32cube\bundles\cmake\4.2.3+st.1\bin\cmake.exe
C:\Users\dhs\AppData\Local\stm32cube\bundles\ninja\1.13.2+st.1\bin\ninja.exe
C:\Users\dhs\AppData\Local\stm32cube\bundles\gnu-tools-for-stm32\14.3.1+st.2\bin\arm-none-eabi-gcc.exe
C:\Users\dhs\AppData\Local\stm32cube\bundles
cube_bundle_path=C:\Users\dhs\AppData\Local\stm32cube\bundles
```

Qué importa de ese resultado:
- el `PATH` efectivo ya no depende de adivinar rutas manuales
- `cube` sigue siendo la fuente de verdad para bundle y entorno
- las rutas exactas pueden cambiar con otra versión de la extensión o del bundle
- el mecanismo correcto es el bootstrap del repo, no hardcodear versiones

## 6. Cuál es la entrada canónica del build en este repo

En este proyecto no se dejó la decisión en abstracto. Se compararon estas dos entradas:

```powershell
cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

contra:

```powershell
cube cmake --preset Debug
cube cmake --build --preset Debug
```

La entrada canónica validada para este repo es:

```powershell
cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

Razones concretas:
- `.vscode/settings.json` usa `cmake.cmakePath = cube-cmake`
- `CMakePresets.json` fija `Debug` y `Release` con generator `Ninja`
- ambos entrypoints resolvieron el mismo `CMakeCache.txt` efectivo para:
  - `CMAKE_GENERATOR = Ninja`
  - `CMAKE_TOOLCHAIN_FILE = cmake/gcc-arm-none-eabi.cmake`
  - `CMAKE_COMMAND` apuntando al `cmake.exe` del bundle STM32
  - `CMAKE_MAKE_PROGRAM` apuntando al `ninja.exe` del bundle STM32
- ambos builds terminaron en esta máquina con el mismo `build/Debug/fmc-320u-v2.elf`
- el tamaño observado del artefacto fue `1807308` bytes en ambos casos

Conclusión práctica:
- `cube-cmake` es la entrada principal porque coincide más directamente con el contrato actual del workspace
- `cube cmake` queda documentado como comparación válida
- `cmake --preset Debug` queda como vía equivalente solo después de haber cargado el entorno STM32

## 7. Cómo builda un humano este proyecto

Flujo recomendado en PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

Si quieres comprobar el entorno antes de configurar:

```powershell
where.exe cube
where.exe cube-cmake
where.exe cmake
where.exe ninja
where.exe arm-none-eabi-gcc
cube --get-current-value cube_bundle_path
cube --resolve cmake
```

Si el bootstrap ya quedó cargado, también puedes usar:

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

Pero en este repo eso se considera una vía equivalente posterior al bootstrap, no la entrada canónica.

## 8. Cómo debe buildar un agente este proyecto

Un agente no debería asumir que porque el proyecto builda en VS Code, la shell ya está preparada.

Secuencia recomendada:

### Paso 1. Detectar si la shell está limpia o ya está bootstrappeada

```powershell
where.exe cube
where.exe cube-cmake
where.exe cmake
where.exe arm-none-eabi-gcc
echo $env:CUBE_BUNDLE_PATH
```

### Paso 2. Si falta el entorno, cargar el bootstrap del repo

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
```

### Paso 3. Verificar el resolvedor STM32

```powershell
cube --get-current-value cube_bundle_path
cube --get-current-value cmsis_pack_root
cube --resolve cmake
```

### Paso 4. Compilar usando la entrada canónica del repo

```powershell
cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

### Paso 5. Usar `cmake` directo solo como equivalente posterior

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

Regla práctica para agentes:
- no asumir que un `cmake.exe` cualquiera representa el tooling STM32 correcto
- no asumir que `CUBE_BUNDLE_PATH` ya existe en la shell
- no asumir que la terminal y el IDE comparten el mismo entorno
- preferir primero el bootstrap del repo y luego la entrada canónica documentada

## 9. Cómo replicar este esquema en un proyecto STM32 nuevo

Si quieres aplicar esta misma idea a otro proyecto STM32, el patrón mínimo es:

### Fijar el contrato del build

El repo debería dejar explícito:
- presets en `CMakePresets.json`
- generator esperado
- toolchain file esperado
- entrypoint canónico del build

### No depender de rutas hardcodeadas

No conviene fijar manualmente en documentación o scripts:
- versiones exactas de extensiones
- versiones exactas de bundles
- paths absolutos que cambian con cada actualización

Conviene:
- descubrir `cube.exe` y `cube-cmake.exe` desde las extensiones instaladas
- usar `cube --get-current-value ...`
- usar `cube --resolve ...` como fuente de verdad del entorno

### Tener bootstrap del repo

El repo debería incluir scripts de bootstrap para que:
- una persona pueda abrir una shell y compilar
- un agente pueda cargar el mismo entorno antes de actuar
- el flujo sea reproducible fuera del IDE

### Comparar entrypoints antes de fijar uno como canónico

No asumir que el entrypoint correcto siempre será el mismo en todos los proyectos.

Conviene contrastar al menos:
- el entrypoint que usa el workspace o la extensión
- una alternativa equivalente resuelta por `cube`

Y decidir el canónico usando evidencia:
- mismo preset
- mismo generator
- mismo toolchain file
- mismo `CMAKE_COMMAND`
- mismo `CMAKE_MAKE_PROGRAM`
- artefacto final comparable

## 10. Límites y advertencias

Este documento usa ejemplos verificados en esta máquina, pero hay que distinguir dos niveles.

Lo que sí es estable:
- el problema de fondo
- la necesidad de bootstrap fuera del IDE
- el uso de `cube` como fuente de verdad del entorno STM32
- el criterio para fijar una entrada canónica del build

Lo que puede cambiar entre instalaciones:
- la versión de la extensión STM32
- la versión del bundle STM32
- los paths absolutos bajo `.vscode\extensions`
- los paths absolutos bajo `%LOCALAPPDATA%\stm32cube\bundles`
- el tamaño exacto del artefacto si cambian fuentes, flags o tooling

Por eso la regla correcta no es copiar versiones exactas, sino preservar el mecanismo:
- bootstrap del repo
- resolvedor STM32
- entrypoint canónico documentado
- criterio explícito de equivalencia
