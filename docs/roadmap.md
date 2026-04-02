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

## M1 - Project rules, architecture and tooling baseline
Status: current

Goal:
- Define the minimum project rules required to start firmware work without structural rework.

Scope:
- Add `.gitignore`
- Add `.editorconfig`
- Add `.clang-format`
- Add `docs/c_style.md`
- Add `docs/firmware_architecture.md`
- Add `AGENTS.md`
- Align milestone tracking documents
- Define initial static analysis strategy

Out of scope:
- CubeMX project generation
- VSCode build/debug
- FreeRTOS integration
- Service implementation
- Hardware bring-up

Exit criteria:
- Repository ignores generated/build/temp files correctly
- Formatting rules are committed and unambiguous
- Layering and dependency rules are documented
- Human and AI guidance are aligned
- `docs/current_milestone.md` reflects the actual deliverables of M1

---

## M2 - STM32U575 hello world baseline
Status: next

Goal:
- Create the first working firmware baseline on target.

Scope:
- Create CubeMX project for STM32U575
- Confirm generated project builds cleanly
- Configure one LED
- Configure one UART
- Place generated code under `firmware/generated/`
- Keep user code outside generated files except required hooks

Exit criteria:
- Build works
- Flash works
- LED blinks
- UART prints hello
- Project structure respects documented architecture

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