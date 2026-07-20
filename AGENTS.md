# AGENTS.md

## Purpose

This file defines stable agent policy for `firmware/`.

It is policy, not the universal task entrypoint.
It is not extended technical documentation.

Use:
- `STYLE.md` for naming, code structure and new contracts locations. 

---

## Operational Reading Order

For active work, read in this order:
1. the user request
2. `AGENTS.md`
3. `WORKING_CONTEXT.md`, when present and relevant
4. roadmap and specification files referenced by `WORKING_CONTEXT.md`, when
   applicable
5. local `README.md` files for the touched folders
6. `STYLE.md` for naming and code structure questions

`WORKING_CONTEXT.md` should stay short and operational.
Roadmaps hold durable phase strategy, dependencies, decision gates, risks, and
exit criteria. They are not active task logs.

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

For behavior-preserving FMC work, confirmed requirements in
`docs/specs/fmc/use_cases.yaml` are the current product behavior authority.
Legacy documents and firmware are evidence of provenance, not authority over
that specification.

Before changing observable product behavior:
- identify the relevant specification sections
- distinguish confirmed, inferred, and unresolved requirements
- report conflicts between specification, legacy, tests, and current code
- do not resolve conflicts silently

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

## Context Reframe

Refresh the active working context when:
- the user asks to start a new `refactor` or `new_feature` track
- the user asks to create or reframe a `WORKING_CONTEXT.md`
- the request falls outside the active `WORKING_CONTEXT.md` and should become a
  separate workstream

If a context reframe is triggered:
- do not force the request into the current workstream
- create or refresh one short `WORKING_CONTEXT.md`
- reference or update the applicable roadmap/specification only when its
  durable content changes
- ask only the minimum blocking questions before generating or updating it

---

## Protected Code

Without explicit human approval, do not edit:
- `drivers/CMSIS/`
- `drivers/STM32U5xx_HAL_Driver/`
- `Middlewares/`
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

## Hardware Configuration Source Of Truth

CubeMX remains the source of truth for MCU hardware configuration:
- pins
- clocks
- peripheral instances
- middleware enablement
- generated initialization shape

When new work requires hardware configuration:
- the human updates CubeMX first, or explicitly approves the intended CubeMX
  change before it is made
- agents do not hand-edit `.ioc` files or generated init code to enable
  hardware
- after CubeMX/regeneration, agents may add the repository-facing wrappers,
  ports, BSP modules, apps, tests, and documentation needed to use the
  generated configuration

Legacy CubeMX files are evidence for comparison, not authority to copy
silently.

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
- active execution state in `WORKING_CONTEXT.md` when required, not lateral prompts
- current product behavior in `docs/specs/`
- durable refactor strategy in `docs/roadmaps/`
- onboarding and repository maps in `README.md` files
- naming and code structure in `STYLE.md`
- public contracts for implemented modules in their headers

Do not duplicate stable policy, product requirements, roadmap content, or module
contracts across multiple files.

`WORKING_CONTEXT.md` controls current scope, sequencing, and temporary
boundaries. It does not override confirmed product requirements.

When closing a slice or changing active context, apply
`docs/workflow/doc_closure.md` before the final response.

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
