# Next FMC Slice Selection

## Purpose

This context closes the validated `display_format` slice and records the
evidence used to choose the next refactor slice.

The next step must not be chosen by inertia. It should be selected by comparing:
- current authored code under `src/`
- legacy source under `legacy/source/`
- the current CubeMX project `fmc-320u-v2.ioc`
- the legacy CubeMX project `legacy/100_main.ioc`
- product specs under `docs/specs/fmc/`

## Display Format Closure

The `display_format` slice is treated as closed for now.

Implemented and validated:
- `src/services/display_format.*`
- regression coverage in `src/apps/tests/regression/`
- LCD visual integration through `src/apps/bringups/display_format_lcd/`
- top row and bottom row formatting on the physical LCD
- UART-guided human inspection for padding, decimal placement, rounding, low
  values, and overflow

Do not keep expanding this slice unless a concrete defect appears.

## Current Refactor State

Implemented product-domain foundation:
- `fmc_model.*`
- `fmc_units.*`
- `fmc_rate.*`
- `fmc_volume.*`
- `display_format.*`

Implemented hardware/display foundation:
- LCD BSP and PCF8553 backend
- LCD layout/map
- LCD bring-ups
- debug UART and debug LEDs
- minimal RTC/debug support

Still not a product runtime:
- `src/apps/product/main` remains a placeholder/smoke app
- there is no current `fmc_service.*`
- there is no input/event service
- there is no persistence service
- there is no command/MXC/printer communication service

## CubeMX Comparison Summary

Current `fmc-320u-v2.ioc`:
- toolchain: CMake
- IP count: 13
- pin count: 32
- active hardware: DEBUG, ICACHE, MEMORYMAP, PWR, RCC, RTC, SPI1, USART1
- configured pins cover:
  - PCF8553 SPI/control
  - debug UART USART1
  - debug LEDs
  - debug enable jumpers
  - keyboard GPIO EXTI on PE10..PE13
  - LSE
  - SWD/SWO

Legacy `legacy/100_main.ioc`:
- toolchain: STM32CubeIDE
- IP count: 20
- pin count: 99
- additional configured IPs:
  - FLASH
  - GPDMA1
  - LPTIM1
  - LPTIM3
  - LPTIM4
  - THREADX
  - USART3
- additional hardware/function areas:
  - keyboard GPIO EXTI on PE10..PE13
  - external buttons on PD3/PD4
  - pulse/rate timing through LPTIM3/LPTIM4
  - USART3 with GPDMA for the MXC/communication path
  - LCD backlight on PE0
  - MXC enable/control pins on PA8/PC8
  - explicit flash/log/device memory regions
  - backup RAM memory region
  - Cortex-M33 MPU regions
  - ThreadX and TIM6 time base

Important conclusion:
- the current repo intentionally has enough hardware for LCD/display-format
  validation and the first four-key short-press bring-up, but not enough
  CubeMX configuration for external keys, backlight, pulse capture, ThreadX
  runtime, USART3/MXC, or flash persistence migration.

## Hardware Configuration Rule

CubeMX remains the source of truth for hardware configuration.

For any slice that needs new pins, clocks, peripherals, middleware, interrupts,
DMA, memory-map regions, or generated init:
1. the human updates CubeMX first
2. generated changes are reviewed deliberately
3. the agent then adds `port/`, `bsp/`, app, test, and product wrappers around
   the generated configuration

The agent must not manually edit `fmc-320u-v2.ioc` or generated init code to
enable hardware.

## Candidate Next Slices

### Option A: Product Presentation Semantics

Scope:
- implement a pure `fmc_presentation.*` slice for `TTL_RATE` and `ACM_RATE`
- consume model/rate/volume values and decimal policy
- output semantic rows, legends, unit labels, slash, and time-base cues
- stay testable without LCD hardware

Why it fits now:
- does not require CubeMX changes
- builds directly on validated `fmc_model`, `fmc_units`, `fmc_rate`,
  `fmc_volume`, and `display_format`
- product specs already describe TTL/RATE and ACM/RATE
- keeps navigation, keys, RTOS, persistence, and pulse capture out of scope

Risks:
- can become too broad if it starts owning navigation or runtime state
- decimal policy ownership still needs a small explicit decision

Best first validation:
- regression tests for semantic presentation output
- optional LCD adapter bring-up after the pure contract is stable

### Option B: Keyboard/Input Bring-Up

Scope:
- configure key and external-button GPIO/EXTI through CubeMX
- add `port`/`bsp` input wrappers
- add a bring-up app that reports short key observations over UART

Why it may be next:
- unlocks real user-flow work
- legacy and specs both emphasize key semantics
- validates a major missing hardware path before product runtime work

CubeMX prerequisite:
- key pins PE10..PE13 and external inputs PD3/PD4
- EXTI/NVIC settings
- optional LED backlight pin PE0 if testing user interaction wake/backlight

Risks:
- long-press/debounce behavior can pull in runtime/timer decisions too early
- should start as hardware/event observation, not full menu navigation

Selection update:
- chosen as the next slice
- initial scope is short press only and is now implemented/validated
- no firmware debounce, because the hardware debounce circuit is expected to be
  sufficient
- no ThreadX timers or queues in the first pass
- keep the input event boundary RTOS-neutral for later ThreadX integration

### Option C: Pulse Acquisition And Rate Runtime Bring-Up

Scope:
- configure the pulse input timing/counter path through CubeMX
- add port-level LPTIM wrappers and a bring-up for pulse windows
- connect observed pulse/time windows to existing `fmc_rate.*`

Why it may be next:
- core flow-computer behavior depends on it
- legacy `fmx.*` uses LPTIM3/LPTIM4 for rate/volume evidence
- current pure rate math is validated but not fed by real capture hardware

CubeMX prerequisite:
- LPTIM3/LPTIM4 configuration, pins, interrupts, and clocks
- confirm the intended sensor/front-end signal mapping before editing code

Risks:
- highest hardware uncertainty
- can force runtime/event architecture questions early

### Option D: Persistence, Backup, And Flash Foundation

Scope:
- define flash and backup-domain foundation before restoring real product state
- compare legacy flash/backup behavior with product requirements
- add wrappers only after CubeMX memory/backup configuration is settled

Why it may be next:
- TTL/config survival is product-critical
- specs explicitly call out flash-backed configuration and backup RAM
- legacy `.ioc` has flash/log/device regions and backup RAM mapping

CubeMX prerequisite:
- FLASH/memory-map/MPU/backup-related configuration reviewed and applied by
  the human if needed

Risks:
- persistence policy is broad and easy to overdesign
- should not precede a clear minimum state schema

### Option E: Communication/MXC/USART3 Foundation

Scope:
- configure USART3/DMA and MXC control pins through CubeMX
- add transport wrappers and a narrow bring-up

Why it may be next:
- legacy includes MXC/Bluetooth/printer workflows
- `legacy/100_main.ioc` includes USART3 and GPDMA for this path

CubeMX prerequisite:
- USART3 pins PD8/PD9
- GPDMA1 channels for USART3 RX/TX
- MXC enable/control pins PA8/PC8

Risks:
- lower immediate value for core flow/display refactor
- can entangle command protocol, printer, Bluetooth, and runtime scheduling

## Recommended Ordering Bias

If the goal is to keep momentum without requiring a CubeMX hardware step:
1. choose Option A, product presentation semantics

If the goal is to unlock interactive hardware behavior:
1. choose Option B, keyboard/input bring-up

If the goal is to validate core measurement behavior next:
1. choose Option C, pulse acquisition/rate runtime bring-up

Persistence and communication are important, but they look less ideal as the
very next slice unless the human wants to prioritize those product risks now.

## Open Questions

- Should the CMake default app remain `bringups/display_format_lcd` after the
  demo checkpoint, or return to `tests/regression` for normal development?
- For presentation work, who owns decimal-resolution policy at runtime until
  configuration persistence exists?
