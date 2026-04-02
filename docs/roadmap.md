# Roadmap

Purpose: use the repository as the shared source of truth for progress and next steps.

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

## M1 - Project rules and working agreement
Status: current

Goal:
- Define minimal rules before generating code.

Scope:
- Add `.gitignore`
- Add `docs/c_style.md`
- Establish milestone tracking in repo

Out of scope:
- CubeMX project
- VSCode build/debug
- FreeRTOS
- Service refactors

Exit criteria:
- `.gitignore` committed
- `docs/c_style.md` committed
- `docs/current_milestone.md` aligned

---

## M2 - STM32U575 hello world (CubeMX + VSCode)
Status: next

Goal:
- First working embedded project.

Scope:
- Create CubeMX project in `firmware/cube/hello_u575`
- Configure 1 LED
- Configure 1 UART
- Build + flash + debug from VSCode

Exit criteria:
- Build works
- Flash works
- LED blinks
- UART prints hello

---

## M3 - Minimal layer integration
Status: later

Goal:
- Introduce layers without breaking flow.

Scope:
- Add `app_main`
- Add minimal BSP (LED + UART)
- Add minimal port (delay)

Exit criteria:
- App logic outside Cube code
- No HAL calls from `app/`

---

## M4 - fm_debug baseline refactor
Status: later

Goal:
- First reusable service with clean boundaries.

Scope:
- Define API
- Implement core
- Add STM32 backend

Exit criteria:
- Service works
- Clean separation of layers