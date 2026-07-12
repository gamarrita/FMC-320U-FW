# Legacy module inventory

## Purpose

This is an initial reading of the authored legacy snapshot under
`legacy/source/`.

It is not a proposed architecture. It records apparent responsibilities,
dependencies, and migration signals so each future refactor slice can choose
deliberately what behavior to preserve.

## Snapshot summary

- Source root: `legacy/source/`
- Original folders preserved: `FLOWMEET/`, `libs/`
- Authored files: 42
- C files: 20
- Headers: 20
- Markdown files: 2

## Module inventory

| Module | Files | Original folder | Apparent responsibility | Main dependencies | HAL/CubeMX/ThreadX usage | Hardware interaction | Legacy interactions | Mixed responsibilities | Product relevance | Active workstream relevance | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|
| `fm_init` | `fm_init.c/.h` | `FLOWMEET` | Product startup and initialization sequencing. | `fm_backup`, `fm_debug`, `fm_flash`, `fm_fmc`, `fm_log`, `fm_mxc`, `fm_rtc`, `main` | Uses CubeMX `main.h`; no direct ThreadX include seen. | Indirect through backup, flash, RTC, MXC, FMC. | Initializes or coordinates major legacy services. | Startup policy, persistence restore, peripheral/service readiness. | High: boot behavior and restored state can affect totals and configuration. | Medium: useful before runtime/service refactor, not needed for pure presentation. | Needs separate analysis before any startup migration. |
| `fm_log_policy` | `fm_log_policy.c/.h`, `fm_log_policy.md`, `fm_log_policy new.md` | `FLOWMEET` | Logging policy and retention/decision rules. | `fm_fmc`, `fmx`, standard bool/stdint/string. | Depends on `fmx`; no direct HAL seen. | Indirect through log/storage modules. | Feeds or constrains `fm_log`. | Policy documentation and code coexist; may include product rules. | Medium: ticket/log behavior may preserve regulatory or operational expectations. | Low now; later relevant for reporting/persistence. | Two Markdown files exist; preserve both as historical evidence. |
| `fm_setup` | `fm_setup.c/.h` | `FLOWMEET` | Configuration menu flow, password entry, factor/unit/time/date editing. | `fm_debug`, `fm_factory`, `fm_fmc`, `fm_lcd`, `fm_log`, `fm_rtc`, `fmx`, string. | Uses `fmx` event vocabulary; LCD calls; no direct ThreadX include in `.c`. | LCD, RTC, configuration side effects. | Calls FMC setters, RTC setters, factory/log helpers. | Navigation, authorization, editing, formatting, LCD writes, persistence triggers. | High: contains setup behavior and operator-facing configuration rules. | Medium: useful evidence after pure presentation and totals slices. | Do not port as one module; split into config, menu flow, presentation, persistence. |
| `fm_usart` | `fm_usart.c/.h` | `FLOWMEET` | USART-facing helper around debug/communication path. | `fm_debug`, `fmx`, `main`, `usart`, stdio. | Uses CubeMX `main.h`/`usart.h`; `fmx` dependency. | UART. | Used by command/MXC/printing paths. | Communication helper may be coupled to product events. | Medium: relevant for commands, Bluetooth/printer bridge, diagnostics. | Low now. | Needs distinction between debug UART and product communications. |
| `fm_user` | `fm_user.c/.h` | `FLOWMEET` | User screen flow: power-on, version, TTL/RATE, ACM/RATE, print, Bluetooth, clock. | `fm_factory`, `fm_fmc`, `fm_lcd`, `fm_log`, `fm_mxc`, `fm_ppt`, `fm_rtc`, `fmx`, `tx_api`, string. | Header includes ThreadX; uses `fmx` events and semaphore-related flow. | LCD, Bluetooth/MXC, RTC, printer path indirectly. | Calls FMC getters/setters, LCD APIs, MXC, PPT, RTC, FMX refresh. | Navigation, presentation, LCD writes, actions, communication workflows, reset policy. | High: primary user-visible behavior. | High: strongest source for future `fmc_presentation.*` and user-flow slices. | Start with TTL/RATE and ACM/RATE evidence only; defer full menu/RTOS flow. |
| `fmx` | `fmx.c/.h` | `FLOWMEET` | Main event framework, keyboard/menu event dispatch, refresh coordination. | `app_threadx`, `tx_api`, `tx_port`, `lptim`, `fm_cmd`, `fm_debug`, `fm_fmc`, `fm_lcd`, `fm_log`, `fm_mxc`, `fm_setup`, `fm_usart`, `fm_user`, `fmx_lp`, `main`. | Heavy ThreadX and CubeMX dependency. | Keyboard/event timing through LPTIM and platform glue. | Calls user/setup menus and service modules. | Scheduler/event loop, UI dispatch, product service orchestration. | High for runtime behavior. | Low for pure presentation; high for later UI service. | Should become evidence for future RTOS/event architecture, not a direct port. |
| `fmx_lp` | `fmx_lp.c/.h` | `FLOWMEET` | Low-power or LPTIM-related support. | `main`, `lptim`, `tx_api`, `fm_debug`, `fmx_lp`. | Uses CubeMX `main.h`/`lptim.h` and ThreadX header. | LPTIM/low-power timing. | Supports `fmx`. | Timing policy and hardware coupling. | Medium: affects event timing and power behavior. | Low now. | Needs hardware configuration context before migration. |
| `fm_backup` | `fm_backup.c/.h` | `libs` | Backup-domain state handling. | `main`. | Uses CubeMX `main.h`. | Backup registers/domain. | Used by startup/init and persistence restore. | Hardware access and persistence policy may be intertwined. | High: totals/config survival may depend on it. | Low now; later runtime/persistence. | Needs original backup/RTC config to interpret correctly. |
| `fm_cmd` | `fm_cmd.c/.h` | `libs` | Command parsing/handling over communication channel. | `fm_debug`, `fmx`, `main`, `tx_api`, `usart`, stdio/string. | ThreadX and CubeMX UART dependencies. | UART/communication path. | Interacts with FMX and debug. | Command protocol, event effects, UART coupling. | Medium: external service/debug behavior. | Low now. | Separate command protocol from transport in any future migration. |
| `fm_debug` | `fm_debug.c/.h` | `libs` | Debug output utilities. | `main`, `usart`, stdio/string. | CubeMX `main.h`/`usart.h`. | UART. | Used throughout legacy modules. | Debug and transport details. | Medium for diagnostics, low for product behavior. | Low now; current `src/libs/fm_debug.*` exists separately. | Compare behavior before assuming current debug fully replaces it. |
| `fm_factory` | `fm_factory.c/.h` | `libs` | Factory/default data application. | `fm_fmc`. | No direct HAL seen in includes. | Indirect through FMC/config consumers. | Used by init/setup/FMC. | Factory policy tied to FMC state. | High: default calibration/config values can affect product behavior. | Medium: relevant when validating model defaults and config slices. | Not migrated just because `fmc_model` has defaults; behavior needs review. |
| `fm_flash` | `fm_flash.c/.h` | `libs` | Flash read/write helper. | `main`, `fm_debug`, stdint. | CubeMX/HAL likely through `main.h`. | Internal flash. | Used by log/init/persistence flows. | Storage driver and error policy may be mixed. | High for persistence. | Low now. | Needs flash layout before migration. |
| `fm_fmc` | `fm_fmc.c/.h` | `libs` | Flow-computer totals, units, calibration, rate, display formatting helpers. | `fm_factory`, `fm_lcd`, `fm_debug`, `fmx`, `fm_lcd_ll`, stdio/stdint. | Depends on `fmx`; includes LCD headers. | Indirect LCD coupling for formatting/unit labels. | Used by user/setup/log/ppt/policy. | Domain math, canonical state, formatting, unit labels, runtime cache. | Very high: core product behavior. | Very high: partially migrated into `fmc_model.*`, `fmc_units.*`, `fmc_rate.*`; totals visible calculation still pending. | Do not mark fully migrated until totals, decimal policy, formatting boundaries, and persistence interactions are covered. |
| `fm_lcd` | `fm_lcd.c/.h` | `libs` | High-level LCD string/decimal helper over low-level LCD. | `fm_lcd_ll`, stdio. | No direct HAL; hardware through LCD LL. | LCD glass via `fm_lcd_ll`. | Used by user/setup/FMC. | Text formatting and display writes. | High for display behavior. | Medium: current `src/bsp/devices/lcd/` replaces some LCD stack behavior. | Compare behavior against current LCD stack before declaring validated coverage. |
| `fm_lcd_ll` | `fm_lcd_ll.c/.h` | `libs` | Low-level LCD layout, symbols, rows, blink, PCF8553 mapping. | `fm_pcf8553`, `main`, `fm_debug`. | Uses CubeMX `main.h`; hardware backend through PCF8553. | LCD segment RAM and symbols. | Used by LCD, FMC, user/setup. | Hardware mapping, product symbols, blink behavior, text cells. | High for display fidelity. | Medium: current LCD BSP was redesigned and tested, but behavior mapping remains evidence. | Historical source is now under `legacy/source/libs/`; old reorganized copies were removed. |
| `fm_log` | `fm_log.c/.h` | `libs` | Event/log storage and retrieval. | `fm_flash`, `fm_debug`, `fm_fmc`, `fm_log_policy`, `fm_rtc`, `fmx`, `main`. | CubeMX `main.h`; `fmx` dependency. | Flash and RTC. | Interacts with FMC and policy. | Storage, event policy, timestamps, product data. | High for audit/reporting behavior. | Low now; later persistence/reporting. | Requires flash layout and policy review. |
| `fm_mxc` | `fm_mxc.c/.h` | `libs` | MXC/Bluetooth communication control. | `fm_cmd`, `fm_debug`, `fm_usart`, `fmx`, `main`, `tx_api`, string/stdbool. | ThreadX and CubeMX dependencies. | UART/GPIO/control path likely through main/usart. | Used by user/init/printing. | Connection workflow, transport, command exchange. | Medium: optional communication feature. | Low now. | Needs original hardware config and product priority decision. |
| `fm_pcf8553` | `fm_pcf8553.c/.h` | `libs` | PCF8553 LCD controller backend. | `main`. | Uses CubeMX `main.h`; likely HAL SPI/GPIO via generated symbols. | PCF8553 LCD controller. | Used by `fm_lcd_ll`. | Hardware driver and board assumptions. | High for LCD hardware. | Medium: current `src/bsp/devices/lcd/pcf8553/` is the new path. | Compare only for hardware behavior evidence, not API preservation. |
| `fm_ppt` | `fm_ppt.c/.h` | `libs` | Printer/ticket formatting and output workflow support. | `fm_fmc`, `fm_mxc`, `fm_rtc`, `fm_usart`, `main`, stdio/string. | CubeMX `main.h`; transport dependencies. | Printer/communication path via MXC/USART. | Used by `fm_user` print screen. | Formatting, data selection, transport coupling. | Medium: reporting/ticket behavior. | Low now. | Later reporting slice should extract ticket contract before transport. |
| `fm_rtc` | `fm_rtc.c/.h` | `libs` | RTC get/set and date/time formatting. | `fm_debug`, stdint/time. | Likely HAL RTC through implementation and generated context. | RTC. | Used by user/setup/log/ppt/init. | Timekeeping, formatting, setup edits. | High for logs, clock screen, tickets. | Low now. | Needs original RTC CubeMX settings for full interpretation. |

## Initial observations

- `FLOWMEET/` is runtime and UI heavy: `fmx`, `fm_user`, and `fm_setup`
  combine events, timing, LCD calls, and product actions.
- `libs/` is not purely reusable. Several modules depend on `fmx`, generated
  CubeMX headers, LCD details, or product-level FMC state.
- `fm_fmc` is the main source for current FMC model/unit/rate migration
  evidence, but it also contains formatting and display coupling.
- `fm_user` is the clearest evidence for operation-screen presentation, but it
  also owns navigation and action handling. Future work should extract only one
  behavior slice at a time.
