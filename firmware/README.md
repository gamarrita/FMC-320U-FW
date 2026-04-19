# FMC-320U-FW

New firmware for FMC-320U.

Initial objective:
- incremental rewrite of the firmware
- layered separation: port, libs, apps
- progressive migration from the previous project
- maintainable workflow for humans and agents

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
- `docs/workflow/comment_pass.md` only when the task is a dedicated comment pass

STM32 build and environment:
- `docs/stm32cube-cli-workflow.md`
- `docs/build_canonical_new_projects.md`
