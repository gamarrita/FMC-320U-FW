# port

## Intent
Contain platform-dependent adaptation (MCU, toolchain, or runtime environment).

## Should contain
- code tied to the porting layer,
- adaptations required to run on this platform,
- details that would change if the target or base environment changes.

## Should NOT contain
- application logic,
- use cases,
- board-specific logic that clearly belongs in `../bsp/`,
- higher-level reusable services.

## Relationship with other folders
`src/port/` resolves platform dependencies.
It should not become a layer for product functional logic.

## Practical guideline
If the code exists because the platform or base environment changes, it likely belongs here.

## Time

`FM_PORT_TIME_SleepMs()` is a ThreadX task-context sleep. It suspends the
calling task through ThreadX and is not a pre-kernel, ISR, HAL busy-wait, or
runtime-detected fallback delay API.

## ThreadX Idle Hooks

`FM_PORT_THREADX_IDLE_Enter()` and `FM_PORT_THREADX_IDLE_Exit()` are called by
ThreadX low-power support while the scheduler enters or leaves idle. The current
port drives the debug-gated run LED and uses conservative MCU Sleep/WFI. It
does not enter STOP mode, define product low-power policy, program alternate
wake timers, or adjust ThreadX ticks.
