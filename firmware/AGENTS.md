# AGENTS.md

## Purpose

This file defines stable agent policy for `firmware/`.

It is policy, not the universal task entrypoint.
It is not extended technical documentation.

Use:
- `STYLE.md` for naming and code structure

---

## Operating Rule

Make small, correct, reviewable, repository-consistent changes.

Do not:
- broaden scope without need
- refactor broadly without request
- introduce speculative improvements
- optimize unrelated code
- create new module boundaries without justification

---

## Learning And Product Quality

This firmware is also a learning-oriented product engineering project.

Treat legacy code as evidence, not authority. Preserve inherited behavior only
when it is a deliberate current product decision, not because it already exists.

Prefer maintainable, understandable product design over copying legacy shapes.
When a decision is architectural or educationally important, make the tradeoff
visible instead of silently encoding it.

---

## Scope

The user request defines the task scope.

If `WORKING_CONTEXT.md` is in play, treat it as the dominant context for that
workstream.
Do not force lateral tasks into `WORKING_CONTEXT.md`.

If scope, ownership, or interaction with active work is unclear:
- stop
- report the ambiguity
- do not guess

---

## Wizard Gate

Use the workflow wizard when:
- the user explicitly says `wizard`
- the user asks to start a new `refactor` or `new_feature` track
- the user asks to create or reframe a `WORKING_CONTEXT.md`
- the request falls outside the active `WORKING_CONTEXT.md` and should become a
  separate workstream

If the wizard gate is triggered:
- do not force the request into the current workstream
- use `docs/workflow/wizard/README.md` as the entrypoint
- ask only the minimum blocking questions before generating a new context pair

---

## Protected Code

Without explicit human approval, do not edit:
- `drivers/CMSIS/`
- `drivers/STM32U5xx_HAL_Driver/`
- `fmc-320u-v2.ioc`
- `cmake/stm32cubemx/CMakeLists.txt`
- `startup_stm32u575xx.s`
- `Core/Src/system_stm32u5xx.c`

Generated `Core/` sources and headers are CubeMX-managed:
- prefer edits only inside explicit `USER CODE` regions when they exist
- do not restructure generated regions outside `USER CODE`
- keep manual edits minimal and easy to preserve across regeneration

If protected code or generated code outside `USER CODE` appears to require
changes:
- stop
- report the exact path, reason, and intended scope
- wait for explicit human approval before editing

---

## Build And Verification

This repository has a canonical STM32 build flow.

When verification matters:
- use the canonical flow documented in
  `docs/canonical-build/stm32cube-cli-workflow.md`
- do not invent alternate build entrypoints
- do not assume system `cmake`, `ninja`, or toolchain binaries are acceptable
  substitutes

After code changes that affect buildability, linkage, or runtime behavior:
- run canonical verification when feasible
- report the result clearly
- avoid broad speculative fixes if verification fails

For documentation-only or policy-only edits, build verification is not required
unless the task explicitly asks for it.

---

## Documentation

Keep one source of truth per topic.

Use this split:
- stable policy in `AGENTS.md`
- active execution state in `WORKING_CONTEXT.md` when is requiered, not lateral prompts
- extended rationale and detailed context in `docs/contexts/` when is requiered, not lateral prompts

Do not duplicate stable policy across multiple files.

---

## Stop And Report Instead Of Editing

Stop and report when:
- scope is unclear
- folder ownership is unclear
- repo documents conflict materially
- the requested change would force a large redesign
- the requested change appears to require editing protected code
- the requested change appears to require editing generated code outside
  `USER CODE`
- required repository context is missing
- a meaningful architecture decision is required and the nearest valid pattern
  is ambiguous
- verification is required but cannot be completed

---

## Preferred Response Shape

For non-trivial tasks, include:
1. task interpretation
2. target files
3. scope or ownership rationale
4. style or policy references used
5. summary of intended change
6. verification performed or planned
7. any boundary intentionally left untouched
