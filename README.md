# FMC-320U-FW

New firmware for FMC-320U.

Initial objective:
- incremental rewrite of the firmware
- layered separation: port, APIs, app
- progressive migration from the previous project
- first milestone: hello world in VSCode for STM32U575

Current canonical capabilities:
- layered architecture with explicit responsibilities
- work oriented to small, reviewable changes consistent with the repository documentation
- reproducible STM32 build outside of VS Code
- explicit environment bootstrap for terminal usage
- build workflow usable by both humans and agents
- documented and verifiable canonical build entry

Operational guides:
- `AGENT_ENTRY.md`
- `01_quickstart.md`
- `02_workflow.md`
- `04_codex_prompts.md`

STM32 build and environment:
- `docs/stm32cube-build-env.md`
- `docs/build_canonical_new_projects.md`
- `docs/prompt_bootstrap_canonical_stm32_build.md`