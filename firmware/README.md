# FMC-320U-FW

Nuevo firmware para FMC-320U.

Objetivo inicial:
- reescritura incremental del firmware
- separación por capas: port, APIs, app
- migración progresiva desde el proyecto anterior
- primer milestone: hello world en VSCode para STM32U575

Capacidades canónicas actuales:
- arquitectura separada por capas con responsabilidades explícitas
- trabajo orientado a cambios pequeños, reviewables y consistentes con la documentación del repo
- build STM32 reproducible fuera de VS Code
- bootstrap explícito del entorno para terminal
- flujo de build usable por humanos y agentes
- entrada canónica del build documentada y verificable

Guías operativas:
- `AGENT_ENTRY.md`
- `01_quickstart.md`
- `02_workflow.md`
- `04_codex_prompts.md`

Build y entorno STM32:
- `docs/stm32cube-build-env.md`
- `docs/build_canonical_new_projects.md`
- `docs/prompt_bootstrap_canonical_stm32_build.md`
