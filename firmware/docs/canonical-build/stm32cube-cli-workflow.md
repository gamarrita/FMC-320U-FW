# Flujo completo STM32 por CLI para este proyecto

Este documento describe el flujo operativo completo para este repo cuando se
quiere trabajar fuera de la extensión STM32 en VS Code / STM32CubeIDE:

1. build canónico
2. flashing
3. debug
4. recepción UART

También deja documentado cómo se llegó a este setup y cómo repetir el mismo
proceso en proyectos nuevos.

Este es el documento operativo canónico para este repo.

Documento relacionado:
- guía reusable para proyectos nuevos: `docs/build_canonical_new_projects.md`

---

## 0. Estado actual del proyecto

Hoy este repo ya tiene resuelto:
- build canónico reproducible desde CLI
- flashing desde CLI con STM32CubeProgrammer
- debug funcional con ST-LINK GDB Server + `arm-none-eabi-gdb`
- salida UART por el Virtual COM Port del ST-LINK

El app canónico actual para validación rápida del firmware es:
- `FM_ACTIVE_APP=lcd_bringup`

El artefacto principal generado en `Debug` es:
- `build/Debug/fmc-320u-v2.elf`

El entorno STM32 efectivo de esta máquina, según `.settings/bundles.store.json`,
es:
- `cmake` = `4.2.3+st.1`
- `ninja` = `1.13.2+st.1`
- `gnu-tools-for-stm32` = `14.3.1+st.2`
- `programmer` = `2.22.0+st.1`
- `stlink-gdbserver` = `7.13.0+st.3`

---

## 1. Compilación: build canónico

### 1.1 Qué significa "canónico" en este proyecto

En este repo, "build canónico" no significa solamente "compila".
Significa:

- usar el mismo flujo de entrada que usa el workspace STM32
- usar el bootstrap del repo para cargar el entorno STM32 real
- usar `cube-cmake` como entrypoint principal
- usar los bundles STM32 correctos para `ninja`, toolchain GNU y utilidades
- respetar los presets de `CMakePresets.json`

La configuración que fija esto hoy está en:
- `.vscode/settings.json`
- `CMakePresets.json`
- `tools/stm32cube-env.ps1`
- `tools/stm32cube-env.sh`

Puntos importantes del workspace:
- `.vscode/settings.json` fija `cmake.cmakePath = cube-cmake`
- `CMakePresets.json` fija generator `Ninja`
- el toolchain file es `cmake/gcc-arm-none-eabi.cmake`

### 1.2 Flujo canónico exacto

En PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
cube-cmake --preset Debug -DFM_ACTIVE_APP=lcd_bringup
cube-cmake --build --preset Debug
```

Notas:
- `Set-ExecutionPolicy -Scope Process Bypass` solo afecta la shell actual.
- `-DFM_ACTIVE_APP=lcd_bringup` deja el flujo explícito y reproducible aunque el
  cache de CMake venga de otro app anterior.
- si querés usar el app default actual del repo, podés omitir
  `-DFM_ACTIVE_APP=lcd_bringup`, pero para reproducibilidad conviene dejarlo.

Resultado esperado:
- `build/Debug/fmc-320u-v2.elf`
- `build/Debug/fmc-320u-v2.map`

### 1.3 Herramientas involucradas

El bootstrap `tools/stm32cube-env.ps1` hace lo siguiente:
- encuentra `cube.exe` dentro de la extensión STM32 instalada en VS Code
- encuentra `cube-cmake.exe` dentro de la extensión STM32 de build
- consulta a `cube` para resolver el entorno real de CMake
- exporta a la shell:
  - `CUBE_BUNDLE_PATH`
  - `CMSIS_PACK_ROOT`
  - `CMAKE_ROOT`
  - `NINJA_ROOT`
  - `GCC_TOOLCHAIN_ROOT`
  - `CUBE_PROGRAMMER_PATH`
  - `PATH`

En esta máquina, después del bootstrap, los binarios relevantes quedan en:

```text
cube.exe
C:\Users\dhs\.vscode\extensions\stmicroelectronics.stm32cube-ide-core-1.2.1-win32-x64\resources\binaries\win32\x86_64\cube.exe

cube-cmake.exe
C:\Users\dhs\.vscode\extensions\stmicroelectronics.stm32cube-ide-build-cmake-1.44.0-win32-x64\resources\cube-cmake\win32\x86_64\cube-cmake.exe

bundle root
C:\Users\dhs\AppData\Local\stm32cube\bundles

GNU toolchain
C:\Users\dhs\AppData\Local\stm32cube\bundles\gnu-tools-for-stm32\14.3.1+st.2\bin

STM32CubeProgrammer CLI
C:\Users\dhs\AppData\Local\stm32cube\bundles\programmer\2.22.0+st.1\bin\STM32_Programmer_CLI.exe

ST-LINK GDB Server
C:\Users\dhs\AppData\Local\stm32cube\bundles\stlink-gdbserver\7.13.0+st.3\bin\ST-LINK_gdbserver.exe
```

### 1.4 Variables y archivos relevantes

Archivos clave:
- `.vscode/settings.json`
- `CMakePresets.json`
- `CMakeLists.txt`
- `cmake/gcc-arm-none-eabi.cmake`
- `tools/stm32cube-env.ps1`

Variables relevantes después del bootstrap:
- `CUBE_BUNDLE_PATH`
- `CMSIS_PACK_ROOT`
- `CMAKE_ROOT`
- `NINJA_ROOT`
- `GCC_TOOLCHAIN_ROOT`
- `CUBE_PROGRAMMER_PATH`
- `PATH`

### 1.5 Build no canónico vs build canónico

#### Build no canónico

Ejemplos:

```powershell
cmake --preset Debug
cmake --build build/Debug
ninja -C build/Debug
```

o incluso:

```powershell
where.exe cmake
where.exe ninja
where.exe arm-none-eabi-gcc
```

si esas herramientas salen del `PATH` del sistema en vez del entorno STM32.

Por qué no es canónico:
- depende de herramientas encontradas implícitamente en `PATH`
- puede usar `cmake`, `ninja` o toolchains no alineados con la extensión STM32
- puede compilar hoy y fallar mañana en otra máquina o shell
- no reproduce el mismo contrato que usa el workspace

#### Build canónico

Ejemplo:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
cube-cmake --preset Debug -DFM_ACTIVE_APP=lcd_bringup
cube-cmake --build --preset Debug
```

Por qué sí es canónico:
- entra por `cube-cmake`, igual que el workspace
- usa el bootstrap del repo
- usa bundles STM32 resueltos por `cube`
- usa el preset `Debug`
- usa generator `Ninja`
- usa el toolchain file del repo

### 1.6 Cómo verificar que el build quedó alineado

Comandos útiles:

```powershell
where.exe cube
where.exe cube-cmake
where.exe cmake
where.exe ninja
where.exe arm-none-eabi-gcc
cube --get-current-value cube_bundle_path
cube --get-current-value cmsis_pack_root
cube --resolve cmake
Get-Content build/Debug/CMakeCache.txt | Select-String "CMAKE_GENERATOR:INTERNAL|CMAKE_MAKE_PROGRAM:FILEPATH|CMAKE_C_COMPILER:FILEPATH|CMAKE_CXX_COMPILER:FILEPATH|CMAKE_TOOLCHAIN_FILE:FILEPATH"
```

En este repo, las señales más importantes de alineación son:
- generator `Ninja`
- `CMAKE_MAKE_PROGRAM` apuntando al `ninja.exe` del bundle STM32
- compilador `arm-none-eabi-gcc` del bundle STM32
- toolchain file `cmake/gcc-arm-none-eabi.cmake`
- entorno cargado desde `cube --resolve cmake`

Nota importante:
- hoy `cube --resolve cmake` resuelve el `cmake.exe` del bundle STM32
- pero `CMAKE_COMMAND` en `CMakeCache.txt` puede no ser un indicador suficiente
  por sí solo
- en este proyecto la canonicidad se decide por:
  - entrypoint `cube-cmake`
  - bootstrap del repo
  - bundles STM32 correctos
  - toolchain y `ninja` efectivos

---

## 2. Programación: flashing desde CLI

### 2.1 Herramienta usada

La herramienta usada es:
- `STM32_Programmer_CLI.exe`

En este proyecto, el bootstrap ya exporta:
- `CUBE_PROGRAMMER_PATH`

### 2.2 Listar probes ST-LINK

Antes de programar, podés listar probes:

```powershell
& "$env:CUBE_PROGRAMMER_PATH\STM32_Programmer_CLI.exe" -l st-link
```

Si hay más de un probe conectado, podés después seleccionar uno con:
- `sn=<serial>`
- `index=<n>`

### 2.3 Comando de flashing recomendado

Con el firmware ya compilado:

```powershell
& "$env:CUBE_PROGRAMMER_PATH\STM32_Programmer_CLI.exe" `
  -c port=SWD mode=NORMAL reset=HWrst `
  -w build/Debug/fmc-320u-v2.elf `
  -v `
  -rst
```

Qué hace:
- conecta por `SWD`
- programa el `.elf`
- verifica la escritura (`-v`)
- resetea al final (`-rst`)

Si querés acelerar iteraciones y tenés un criterio claro de riesgo, podés
ajustar el comando después, pero este es el flujo más claro y seguro para uso
general.

### 2.4 Variante con selección explícita de probe

```powershell
& "$env:CUBE_PROGRAMMER_PATH\STM32_Programmer_CLI.exe" `
  -c port=SWD sn=<SERIAL_DEL_STLINK> mode=NORMAL reset=HWrst `
  -w build/Debug/fmc-320u-v2.elf `
  -v `
  -rst
```

---

## 3. Debug

### 3.1 Estado actual

En este proyecto el debug está considerado funcional.

Importante:
- el repo no versiona `launch.json` ni `.launch` propios
- la sesión normal de debug desde la extensión STM32 funciona con tooling
  resuelto por los bundles STM32
- la reproducción por CLI debe hacerse manualmente con:
  - `ST-LINK_gdbserver.exe`
  - `arm-none-eabi-gdb.exe`

### 3.2 Herramientas necesarias

Servidor:
- `ST-LINK_gdbserver.exe`

Cliente:
- `arm-none-eabi-gdb.exe`

Ambas salen del entorno STM32 del repo:
- `ST-LINK_gdbserver.exe` desde `CUBE_BUNDLE_PATH`
- `arm-none-eabi-gdb.exe` desde `GCC_TOOLCHAIN_ROOT`

### 3.3 Iniciar servidor GDB

Con el bootstrap ya cargado:

```powershell
$GdbServer = Join-Path $env:CUBE_BUNDLE_PATH "stlink-gdbserver\7.13.0+st.3\bin\ST-LINK_gdbserver.exe"
& $GdbServer -p 61234 -d -v -cp $env:CUBE_PROGRAMMER_PATH
```

Significado práctico:
- `-p 61234`: puerto TCP del server GDB
- `-d`: debug por SWD
- `-v`: verbose
- `-cp`: path de STM32CubeProgrammer, requerido por el server

Si necesitás fijar un probe específico:
- agregá `-i <serial>`

### 3.4 Iniciar GDB

En otra terminal con el bootstrap cargado:

```powershell
& "$env:GCC_TOOLCHAIN_ROOT\arm-none-eabi-gdb.exe" build/Debug/fmc-320u-v2.elf
```

Dentro de GDB:

```gdb
target extended-remote localhost:61234
monitor reset halt
load
monitor reset halt
continue
```

Si ya flasheaste antes con `STM32_Programmer_CLI.exe` y no querés reprogramar
desde GDB, podés omitir `load`.

### 3.5 Flujo mínimo de debug reproducible

1. build canónico
2. arrancar `ST-LINK_gdbserver`
3. abrir `arm-none-eabi-gdb` con el `.elf`
4. conectar con `target extended-remote`
5. `monitor reset halt`
6. `load` si querés programar desde GDB
7. `continue`

---

## 4. Comunicación UART

### 4.1 Canal UART de debug

En este proyecto:
- el canal de debug usa `USART1`
- está configurado a `115200 8N1`
- `USART1` está conectado al Virtual COM Port del ST-LINK

Configuración observada en `Core/Src/usart.c`:
- baudrate: `115200`
- data bits: `8`
- parity: `none`
- stop bits: `1`

En `port/fm_port_usart1.c` también quedó explícito:
- el backend de debug es `USART1`
- el host recibe eso por ST-LINK VCP

### 4.2 Cómo recibir datos

Podés usar cualquier terminal serie.

Opción CLI con Python + pyserial:

```powershell
python -m serial.tools.list_ports
python -m serial.tools.miniterm COMx 115200
```

Reemplazá `COMx` por el puerto real del ST-LINK VCP.

Alternativas válidas:
- PuTTY
- Tera Term
- cualquier terminal serie equivalente

### 4.3 Interacción con jumpers de debug

Este proyecto tiene dos señales de habilitación muestreadas por GPIO:
- mensajes UART de debug
- LEDs de debug

El código actual:
- samplea esos jumpers en `FM_DEBUG_Init()`
- emite un banner inicial por UART con el estado observado

Ejemplo:

```text
DEBUG_INIT:MSG=ENABLED LED=DISABLED
```

Eso es intencional:
- evita esperar mensajes UART que nunca van a aparecer
- deja evidencia inmediata de si el jumper de mensajes está habilitado

### 4.4 Qué esperar con el bringup actual

Con `FM_ACTIVE_APP=lcd_bringup`, la UART debería mostrar:

```text
LCD_BRINGUP:START
LCD_BRINGUP:LCD_INIT_OK
LCD_BRINGUP:ROW_TOP_OK
LCD_BRINGUP:INDICATOR_OK
LCD_BRINGUP:FLUSH_OK
LCD_BRINGUP:IDLE
```

---

## 5. Contexto y decisiones

Este estado no fue el inicial del proyecto.
Se llegó a este setup por iteraciones.

Problemas iniciales:
- builds no reproducibles desde terminal
- shells que no encontraban `cube`, `cube-cmake`, `cmake`, `ninja` ni el
  toolchain ARM
- agentes que conseguían compilar algunas veces, pero no de forma alineada con
  el tooling STM32 del workspace
- uso implícito de herramientas encontradas en `PATH`, no del entorno STM32 real

Decisiones que estabilizaron el flujo:
- agregar bootstrap explícito del repo:
  - `tools/stm32cube-env.ps1`
  - `tools/stm32cube-env.sh`
- usar `cube` como fuente de verdad del entorno STM32
- documentar `cube-cmake` como entrada canónica del build
- fijar el contrato del workspace en:
  - `.vscode/settings.json`
  - `CMakePresets.json`
  - `cmake/gcc-arm-none-eabi.cmake`
- dejar comandos reproducibles para flashing, debug y UART

---

## 6. Ajustes iniciales para proyectos nuevos

Esta sección es la guía reusable.

### 6.1 Cómo detectar que el build NO es canónico

Señales típicas:
- `where.exe cube` no devuelve nada
- `where.exe cube-cmake` no devuelve nada
- `where.exe arm-none-eabi-gcc` devuelve un toolchain no STM32, o nada
- `CUBE_BUNDLE_PATH` está vacía
- `cmake --preset Debug` depende de un `cmake.exe` cualquiera del sistema
- `CMAKE_MAKE_PROGRAM` no apunta al `ninja.exe` del bundle STM32
- el compilador efectivo no es el GNU bundle STM32
- el comportamiento cambia entre IDE y terminal

Chequeo mínimo recomendado:

```powershell
where.exe cube
where.exe cube-cmake
where.exe cmake
where.exe ninja
where.exe arm-none-eabi-gcc
echo $env:CUBE_BUNDLE_PATH
```

### 6.2 Cómo alinear el entorno

Paso 1: identificar qué usa realmente la extensión STM32

Revisar:
- `.vscode/settings.json`
- `.settings/bundles.store.json`
- `.settings/ide.store.json`

Paso 2: resolver el entorno real con `cube`

```powershell
cube --get-current-value cube_bundle_path
cube --get-current-value cmsis_pack_root
cube --resolve cmake
```

Paso 3: no usar herramientas "equivalentes" encontradas en `PATH`

No asumir que estos son válidos por sí mismos:
- `cmake.exe` del sistema
- `ninja.exe` del sistema
- toolchains ARM instalados por fuera de STM32

Paso 4: forzar las rutas correctas

Lo más sano es:
- bootstrap del repo que descubra extensiones STM32 instaladas
- exporte bundles correctos
- y actualice `PATH` para esa sesión

### 6.3 Qué archivos revisar o ajustar

Mínimo:
- `CMakeLists.txt`
- `CMakePresets.json`
- `.vscode/settings.json`
- `cmake/gcc-arm-none-eabi.cmake`
- scripts de bootstrap del repo

Si existen en el proyecto:
- `tasks.json`
- `launch.json`
- Makefiles auxiliares

Regla práctica:
- si existen tasks o launch configs, deben apuntar al mismo wrapper, bundles y
  toolchain que usa STM32
- en este repo no hay `launch.json` versionado

### 6.4 Estrategia paso a paso para pasar de "funciona" a "canónico"

1. confirmar que el build desde terminal no hereda automáticamente el entorno
2. descubrir qué resuelve STM32 con `cube --resolve cmake`
3. crear bootstrap del repo
4. fijar presets y generator
5. fijar toolchain file
6. decidir un entrypoint canónico:
   - en este repo: `cube-cmake`
7. validar con build real
8. comparar:
   - generator
   - `ninja`
   - compilador
   - toolchain file
   - artefacto final
9. recién después documentar flashing y debug

---

## 7. Proceso guiado con Codex

Este setup no apareció en un solo paso.
Se logró con instrucciones narrativas iterativas.

Situación inicial con Codex:
- lograba compilar algunas veces
- pero no de forma canónica
- tendía a usar herramientas encontradas en `PATH`
- no siempre coincidía con el entorno de la extensión STM32

Cómo se lo fue guiando:
- distinguir "compila" de "compila canónicamente"
- hacer que inspeccione el entorno real del workspace
- corregir toolchains
- alinear `cube`, `cube-cmake`, bundles y variables STM32
- dejar bootstrap reutilizable en el repo
- dejar un criterio verificable de equivalencia

Reglas reutilizables extraídas del proceso:
- no asumir que IDE y terminal comparten entorno
- no asumir que el `cmake` del sistema es el correcto
- usar `cube` como fuente de verdad
- documentar un entrypoint canónico explícito
- dejar bootstrap del repo antes de pedir automatización a un agente

---

## 8. Guía de reutilización mínima

Si arrancás un proyecto STM32 nuevo y querés dejarlo listo para humanos y
agentes, el mínimo recomendable es:

1. `CMakePresets.json` con generator y build dirs claros
2. toolchain file explícito
3. `.vscode/settings.json` alineado con `cube-cmake`
4. bootstrap del repo para cargar el entorno STM32 real
5. comando canónico de build documentado
6. comando de flashing documentado
7. flujo manual de debug documentado
8. canal UART documentado

Plantilla mínima de operación en PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
cube-cmake --preset Debug -DFM_ACTIVE_APP=lcd_bringup
cube-cmake --build --preset Debug
& "$env:CUBE_PROGRAMMER_PATH\STM32_Programmer_CLI.exe" -c port=SWD mode=NORMAL reset=HWrst -w build/Debug/fmc-320u-v2.elf -v -rst
python -m serial.tools.miniterm COMx 115200
```

Y para debug:

```powershell
$GdbServer = Join-Path $env:CUBE_BUNDLE_PATH "stlink-gdbserver\7.13.0+st.3\bin\ST-LINK_gdbserver.exe"
& $GdbServer -p 61234 -d -v -cp $env:CUBE_PROGRAMMER_PATH
& "$env:GCC_TOOLCHAIN_ROOT\arm-none-eabi-gdb.exe" build/Debug/fmc-320u-v2.elf
```

---

## 9. Resumen ejecutivo

Si hoy querés repetir por CLI lo que hace el entorno STM32 de este proyecto:

### Build canónico

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
cube-cmake --preset Debug -DFM_ACTIVE_APP=lcd_bringup
cube-cmake --build --preset Debug
```

### Flashing

```powershell
& "$env:CUBE_PROGRAMMER_PATH\STM32_Programmer_CLI.exe" -c port=SWD mode=NORMAL reset=HWrst -w build/Debug/fmc-320u-v2.elf -v -rst
```

### Debug

```powershell
$GdbServer = Join-Path $env:CUBE_BUNDLE_PATH "stlink-gdbserver\7.13.0+st.3\bin\ST-LINK_gdbserver.exe"
& $GdbServer -p 61234 -d -v -cp $env:CUBE_PROGRAMMER_PATH
& "$env:GCC_TOOLCHAIN_ROOT\arm-none-eabi-gdb.exe" build/Debug/fmc-320u-v2.elf
```

### UART

```powershell
python -m serial.tools.list_ports
python -m serial.tools.miniterm COMx 115200
```

Eso es hoy el flujo reproducible, canónico y ejecutable del proyecto.
