# FMC-320U-FW

New firmware for FMC-320U.

Initial objective:
- incremental rewrite of the firmware
- authored firmware source under `src/`
- layered separation: `src/port`, `src/bsp`, `src/product`, `src/services`,
  `src/apps`
- progressive migration from the previous project
- maintainable workflow for humans and agents

Repository boundary:
- CubeMX-generated and vendor-managed folders remain at repository root in the
  layout expected by CubeMX
- authored firmware code belongs under `src/`

Current canonical capabilities:
- layered architecture with explicit responsibilities
- work oriented to small, reviewable changes consistent with repository documentation
- reproducible STM32 build outside of VS Code
- explicit environment bootstrap for terminal usage
- staged workflow usable by both humans and agents
- documented and verifiable canonical build entry
- explicit comment-pass workflow for incremental comment cleanup

Operational entrypoint:
- `WORKING_CONTEXT.md`

Stable supporting references:
- `STYLE.md`
- `docs/workflow/README.md`

Specialized pass reference:
- `docs/workflow/passes/README.md` when the task enters a methodological pass

STM32 build and environment:
- `docs/canonical-build/stm32cube-cli-workflow.md`
- `docs/canonical-build/build_canonical_new_projects.md`
