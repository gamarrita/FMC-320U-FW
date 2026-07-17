# FMC Service/Runtime Boundary

## Purpose

This context tracks the next selected refactor slice: defining the smallest
useful `fmc_service` / `fmc_runtime` boundary.

The product already works on finished hardware with legacy firmware. The
primary project risk is not hardware novelty; it is losing the ability to
continue the refactor clearly with agent assistance once ThreadX, low-power
modes, timers, input, acquisition, persistence, and presentation begin to
interact.

## Selected Direction

Build a narrow runtime boundary before adding more behavior around it.

The boundary should make these responsibilities explicit:
- pure FMC model/math remains under `src/product/fmc/`
- service ownership of live FMC state is separated from presentation and HAL
- runtime adaptation owns scheduling, event delivery, RTOS, and low-power
  concerns
- hardware-facing acquisition/input/persistence/communication modules connect
  through small contracts instead of calling product internals directly

## Current Evidence

Implemented foundations:
- pure FMC model, unit, rate, and volume slices
- display formatting and LCD validation slices
- debug UART/LED support
- keyboard short-press bring-up for `ESC`, `ENTER`, `UP`, and `DOWN`

Nearby evidence:
- `legacy/source/FLOWMEET/fmx.c`: legacy event loop, keyboard, ThreadX, and
  low-power interactions
- `legacy/source/FLOWMEET/fm_user.*`: visible user flow and presentation calls
- `legacy/100_main.ioc`: legacy CubeMX hardware/middleware comparison

Still missing:
- `fmc_service.*`
- `fmc_runtime.*`
- product event vocabulary beyond the keyboard bring-up
- runtime snapshot/update contract
- ThreadX ownership decision
- low-power ownership decision
- acquisition-to-rate runtime contract
- persistence restore/save policy

## First Sub-Slice Options

### Option 1: Service State/Snapshot Contract

Scope:
- add `fmc_service.*` as the owner of live FMC state
- expose init, snapshot, and simple update/query functions
- stay independent of ThreadX, HAL, LCD, keyboard, persistence, and low power

Why it fits:
- creates the central product boundary with low risk
- keeps regression testing simple
- gives future runtime/input/acquisition/persistence code one place to connect

### Option 2: Runtime Event Loop Skeleton

Scope:
- add a minimal runtime owner that receives events and calls service functions
- keep it buildable in the current bare-metal app shape
- define what will later become ThreadX thread/queue ownership

Why it fits:
- directly addresses the future RTOS boundary
- can be written as a stepping stone before enabling ThreadX

### Option 3: ThreadX/Low-Power Risk Study

Scope:
- analyze legacy `fmx.*`, `fmx_lp.*`, LPTIM use, wake sources, and ThreadX
  interaction
- produce an implementation plan before generated-code or low-power changes

Why it fits:
- addresses the riskiest combined area before committing architecture

Risk:
- produces less product code immediately

## Recommended First Cut

Start with Option 1 or Option 2.

Option 1 is the safer first implementation because it creates `fmc_service.*`
without RTOS or hardware churn.

Option 2 is better if the next lesson should be runtime ownership and event
flow, accepting that the first loop may be bare-metal before ThreadX.

Option 3 is better only if the human wants risk analysis before more code.

Avoid starting with low power plus ThreadX together. Treat their interaction as
an explicit later decision, not an accidental side effect of a broader runtime
port.

## Boundaries

Do not implement in the first sub-slice unless explicitly selected:
- full menu navigation
- long press
- ThreadX enablement
- low-power entry/exit
- pulse acquisition
- persistence restore/save
- MXC/communication
- flash or backup-domain policy

## Validation Bias

Prefer one of:
- regression tests for pure service behavior
- a narrow product app smoke path that initializes the service and emits a
  clear UART trace
- both, if the implementation touches runtime wiring
