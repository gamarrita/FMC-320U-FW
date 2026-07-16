# Legacy migration ledger

## Purpose

This ledger tracks how observed legacy responsibilities relate to the new
architecture.

Statuses are intentionally conservative. A module is not marked as migrated
just because a new file has a similar name.

## Status vocabulary

- `no analizado`: present in the snapshot but not reviewed enough.
- `inventariado`: reviewed at module level only.
- `comportamiento parcialmente documentado`: behavior appears in specs or notes.
- `parcialmente migrado`: some behavior has an equivalent under `src/`.
- `migrado`: intended behavior has a new implementation.
- `validado`: intended behavior has implementation and verification evidence.
- `descartado deliberadamente`: behavior was intentionally not preserved.
- `fuera de alcance actual`: not part of the active workstream.

## Ledger

| Modulo legado | Responsabilidades observadas | Destino probable en arquitectura nueva | Estado actual | Evidencia disponible | Pruebas disponibles | Comportamiento pendiente de confirmar | Prioridad sugerida | Notas |
|---|---|---|---|---|---|---|---|---|
| `FLOWMEET/fm_init.*` | Startup, restore/init sequence, service initialization. | Future app/runtime startup service. | inventariado | `legacy/source/FLOWMEET/fm_init.*` | None specific. | Exact restore order and failure policy. | Medium | Needs backup/flash/RTC context. |
| `FLOWMEET/fm_log_policy.*` | Log policy decisions and documentation. | Future logging/reporting policy module. | inventariado | Code plus two Markdown policy files. | None. | Which policy rules remain current product decisions. | Low | Preserve as evidence until log slice. |
| `FLOWMEET/fm_setup.*` | Password, setup navigation, calibration factor, volume unit, time unit, date/time editing. | Future config, menu flow, presentation, persistence slices. | comportamiento parcialmente documentado | `docs/specs/fmc/use_cases.yaml`; `legacy/source/FLOWMEET/fm_setup.*` | None in current harness. | Password flow, edit cursor/blink behavior, persistence timing. | Medium | Do not start before operation presentation/volume are clearer. |
| `FLOWMEET/fm_usart.*` | USART helper/bridge behavior. | Port/service communication layer. | inventariado | Source includes generated UART context. | Current debug UART has separate tests only indirectly. | Relationship between debug, command, printer, and MXC transport. | Low | Needs original UART/DMA settings. |
| `FLOWMEET/fm_user.*` | User screens, TTL/RATE, ACM/RATE, print, Bluetooth, clock, navigation and actions. | Split into presentation, user-flow service, reporting, communication workflows. | comportamiento parcialmente documentado | `docs/specs/fmc/use_cases.yaml`; `docs/specs/fmc/presentation_screens.md`; source. | Current LCD bringups and FMC model/rate harness cover only adjacent pieces. | Exact contract between values, screen modes, key events, and RTOS refresh. | High | Next likely evidence source for `fmc_presentation.*`, after visible-volume calculation. |
| `FLOWMEET/fmx.*` | Event loop, keyboard events, refresh scheduling, ThreadX integration. | Future RTOS/UI service boundary. | inventariado | Source includes ThreadX and LPTIM context. | None in new architecture. | Event timing, queue/thread ownership, key semantics. | Medium | Keep out of pure presentation/model modules. |
| `FLOWMEET/fmx_lp.*` | Low-power/LPTIM support. | Future port/runtime power service. | inventariado | Source. | None. | Hardware timing and low-power requirements. | Low | Needs original LPTIM/CubeMX config. |
| `libs/fm_backup.*` | Backup-domain state handling. | Future persistence/backup service. | inventariado | Source. | None. | What state must survive each reset class. | Medium | Relevant to TTL/config preservation. |
| `libs/fm_cmd.*` | Command parser and communication commands. | Future command protocol plus transport adapter. | inventariado | Source. | None. | Product command set and side effects. | Low | Do not mix with UART driver migration. |
| `libs/fm_debug.*` | Debug output. | Existing `src/libs/fm_debug.*` plus port UART. | parcialmente migrado | Current debug lib and legacy source. | Bringups emit debug messages through current path. | Whether legacy formatting/status behavior matters. | Low | Current replacement is practical, not a full behavior migration. |
| `libs/fm_factory.*` | Factory/default FMC state. | Future config/default provider. | inventariado | Source and current `fmc_model` defaults. | Current model defaults tested. | Which factory defaults are product-required. | Medium | Do not assume current defaults cover all factory behavior. |
| `libs/fm_flash.*` | Flash access. | Future flash/persistence service. | inventariado | Source. | None. | Flash layout, erase/write policy, failure behavior. | Medium | Needs original linker/flash map context. |
| `libs/fm_fmc.*` | Totals, calibration, unit conversion, rate, decimal selection, formatting helpers. | `src/product/fmc/fmc_model.*`, `fmc_units.*`, `fmc_rate.*`, future volume and presentation helpers. | parcialmente migrado | Current FMC modules, specs, source. | `tests/regression` validates model/units/rate/display-format slices. | Visible ACM/TTL volume calculation, decimal policy, formatting, persistence interactions. | High | Core evidence for next `fmc_volume.*` and presentation slices. |
| `libs/fm_lcd.*` | High-level LCD string/decimal helper. | Current `src/bsp/devices/lcd/` plus future LCD adapter. | parcialmente migrado | LCD source and current LCD BSP. | `bringups/lcd`, `bringups/lcd_blink`. | Exact legacy text/decimal behavior versus new API. | Medium | Avoid forcing old API shape onto new LCD stack. |
| `libs/fm_lcd_ll.*` | LCD layout, symbol, blink and low-level mapping. | Current LCD BSP mapping/layout modules. | parcialmente migrado | Current LCD BSP and legacy source. | `bringups/lcd`, `bringups/lcd_blink`. | Full symbol parity and any untested visual cases. | Medium | Historical source should be consulted for product indicators. |
| `libs/fm_log.*` | Log storage, timestamps, policy integration. | Future log service plus persistence. | inventariado | Source and policy docs. | None. | Record schema, retention, trigger behavior. | Low | Depends on flash/RTC/FMC decisions. |
| `libs/fm_mxc.*` | MXC/Bluetooth communication workflow. | Future communication service. | inventariado | Source. | None. | Hardware protocol and product priority. | Low | Used by print/Bluetooth user screens. |
| `libs/fm_pcf8553.*` | LCD controller backend. | Current `src/bsp/devices/lcd/pcf8553/`. | parcialmente migrado | Current PCF8553 BSP and legacy source. | LCD bringups. | Hardware edge cases and controller init parity. | Medium | Do not include legacy backend in active build. |
| `libs/fm_ppt.*` | Ticket/printer formatting. | Future reporting/ticket module plus transport adapter. | inventariado | Source. | None. | Ticket format and required data fields. | Low | Relevant after FMC/log/persistence decisions. |
| `libs/fm_rtc.*` | RTC date/time get/set/format. | Future RTC service or port adapter. | inventariado | Source. | None. | Exact date/time format and RTC HAL behavior. | Low | Needed for clock, logs, tickets, setup date/time. |

## Current migrated/validated areas

- `fm_fmc` model state concepts are partially represented by
  `src/product/fmc/fmc_model.*`.
- `fm_fmc` unit policy is partially represented by
  `src/product/fmc/fmc_units.*`.
- `fm_fmc` rate calculation is partially represented by
  `src/product/fmc/fmc_rate.*`.
- LCD controller/layout behavior is partially represented by
  `src/bsp/devices/lcd/` and validated through current LCD bring-up apps.
- These areas are not fully migrated until the missing behavior listed in the
  ledger is either implemented, validated, or deliberately discarded.
