# FMC-320U-FW

New firmware for FMC-320U.

## Objective

Incrementally rebuild the FMC-320U firmware with clear ownership, small
reviewable changes, and a workflow that works for both humans and agents.

## Repository Boundary

CubeMX-generated and vendor-managed folders remain at repository root in the
layout expected by CubeMX. This keeps CubeMX usable for RTOS changes,
middleware, peripherals, and regeneration.

Authored firmware code belongs under `src/`.

## Authored Source Layout

- `src/apps/`: selectable firmware application profiles.
- `src/bsp/`: board and device-facing hardware details.
- `src/port/`: MCU, toolchain, and RTOS/base-runtime adaptation.
- `src/product/`: FMC-320U product semantics and behavior.
- `src/services/`: reusable technical services.
- `src/libs/`: small support libraries that are not product semantics.

## Working With The Repo

For agents:
- start with `AGENTS.md`
- use `docs/project/WORKFLOW.md` for continuity across conversations
- use `WORKING_CONTEXT.md` when an active workstream exists
- use the referenced product documentation, roadmaps, and specifications for
  accepted behavior, durable strategy, and technical authority
- use `STYLE.md` for naming and code structure

For humans:
- use local `README.md` files to understand folder ownership
- use `docs/product/fmc/README.md` for current FMC product documentation
- use `CONTRIBUTING.md` for commit and changelog hygiene
- use `docs/workflow/README.md` for the stage model

STM32 build and environment:
- `docs/canonical-build/stm32cube-cli-workflow.md`
- `docs/canonical-build/build_canonical_new_projects.md`
