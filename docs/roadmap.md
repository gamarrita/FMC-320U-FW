# Roadmap

Purpose: use the repository as the shared source of truth for progress, constraints, and next steps.

Status values:
- done
- current
- next
- later

---

## M0 - Repository foundation
Status: done

Goal:
- Create repository and base folder structure.

Completed:
- Repo initialized
- `firmware/` layout created

Exit criteria:
- Repo usable as base for development

---

## M1 - Workflow definition
Status: done

Goal:
- Define the repository workflow for human discussion, milestone consolidation, and Codex execution.

Scope:
- Define milestone source-of-truth document
- Define design workspace document
- Define agent execution prompt document
- Define persistent agent instructions
- Align roadmap and current milestone docs

Out of scope:
- C coding style
- Formatting rules
- Static analysis
- Firmware architecture decisions
- CubeMX
- Firmware implementation

Exit criteria:
- Workflow documents created
- Roles of each document are explicit
- Human -> ChatGPT -> repo -> Codex flow is clear

---

## M2 - Coding rules and tooling baseline
Status: current

---

## M3 - Minimal platform integration
Status: later

Goal:
- Introduce project layers with one real vertical slice.

Scope:
- Add minimal `port/` startup hooks
- Add minimal `bsp/` for LED and UART
- Add minimal `app/` entry point
- Add one reusable `core/` service

Exit criteria:
- Application logic outside CubeMX generated files
- No HAL calls from `firmware/app/`
- HAL callbacks owned by `firmware/port/` or `firmware/bsp/`

---

## M4 - RTOS-ready structure
Status: later

Goal:
- Prepare the codebase for future FreeRTOS integration without refactoring architecture.

Scope:
- Define OS abstraction boundary if needed
- Ensure app/core do not depend on RTOS headers
- Add task-oriented integration plan

Exit criteria:
- Architecture can absorb FreeRTOS with local changes in `port/`
- No RTOS types leaked into high-level modules

---

## M5 - First production-oriented service
Status: later

Goal:
- Implement one real service with clean API, bounded behavior, and clear ownership.

Scope:
- Define service API
- Implement service core
- Add STM32 backend if required
- Add basic validation hooks

Exit criteria:
- Service works on target
- Separation of concerns remains clean
- Module is ready to serve as pattern for the rest of the firmware