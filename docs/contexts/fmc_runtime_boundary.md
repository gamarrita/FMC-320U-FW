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
- `fmc_service.*` live-state owner with pulse-delta update, total reset, and
  snapshot contract
- `fmc_runtime.*` minimal RTOS-neutral event dispatch over `fmc_service`,
  including pulse-delta, ACM/TTL reset, and presentation-invalidation events
- display formatting and LCD validation slices
- debug UART/LED support
- keyboard short-press bring-up for `ESC`, `ENTER`, `UP`, and `DOWN`

Nearby evidence:
- `legacy/source/FLOWMEET/fmx.c`: legacy event loop, keyboard, ThreadX, and
  low-power interactions
- `legacy/source/FLOWMEET/fm_user.*`: visible user flow and presentation calls
- `legacy/100_main.ioc`: legacy CubeMX hardware/middleware comparison

Still missing:
- product event vocabulary beyond the keyboard bring-up
- ThreadX ownership decision
- low-power ownership decision
- acquisition-to-rate runtime contract
- persistence restore/save policy

## Incremental Working Rule

Move one or two decisions at a time.

For each new module, the API contract belongs in its `.h`. Markdown should
capture the workstream rationale, ordering, and open decisions without
duplicating public API documentation.

For later `fmc_runtime` growth, review the header contract before extending
behavior.

## Next Micro-Slice

### Closed: Service State/Snapshot Contract

Implemented scope:
- add `fmc_service.*` as the owner of live FMC state
- expose init, pulse-delta update, total reset, and snapshot functions
- stay independent of ThreadX, HAL, LCD, keyboard, persistence, and low power

### Implemented: Minimal Runtime Contract

Implemented scope:
- add `fmc_runtime.*` as the RTOS-neutral owner of one `fmc_service_t`
- accept minimal product events for pulse deltas, ACM reset, TTL reset, and
  presentation invalidation
- expose snapshot and presentation-update acknowledgement without owning keyboard,
  LCD rendering, HAL, queues, timers, ThreadX, persistence, or low power

Why it fits:
- directly addresses the future RTOS boundary
- preserves human/agent iteration on an architectural boundary
- keeps API-specific documentation in `fmc_runtime.h` instead of this context

### Next Selection Pending

Choose the next micro-slice after reviewing the implemented runtime contract.

Candidates:
- map keyboard-originated product events into the runtime boundary
- introduce acquisition-originated pulse/rate window dispatch
- wire a narrow product-app smoke path through `fmc_runtime`

### Later Option: ThreadX/Low-Power Risk Study

Scope:
- analyze legacy `fmx.*`, `fmx_lp.*`, LPTIM use, wake sources, and ThreadX
  interaction
- produce an implementation plan before generated-code or low-power changes

Why it fits:
- addresses the riskiest combined area before committing architecture

Risk:
- produces less product code immediately

## Roadmap

1. Inspect legacy runtime/event responsibilities narrowly.
2. Propose and implement the minimal `fmc_runtime.h` contract.
3. Review the runtime contract with the human before selecting the next
   product-facing behavior.
4. Defer ThreadX, low power, menu navigation, long press, acquisition,
   persistence, and communication until selected deliberately.

Avoid starting with low power plus ThreadX together. Treat their interaction as
an explicit later decision, not an accidental side effect of a broader runtime
port.

## Boundaries

Do not implement in the next micro-slice unless explicitly selected:
- full menu navigation
- long press
- ThreadX enablement
- low-power entry/exit
- pulse acquisition
- persistence restore/save
- MXC/communication
- flash or backup-domain policy
- broad `fmc_runtime.c` behavior beyond the reviewed minimal contract

## Validation Bias

Prefer one of:
- no build for analysis-only updates
- regression tests when runtime behavior is introduced
- a narrow product app smoke path only after runtime wiring exists
