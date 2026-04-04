# 01_quickstart.md

## Purpose

This is the quick operational guide for working in `firmware/`.

Read this file first when starting a task.
It is intended for both humans and agents.

This file explains how to start correctly, how to keep tasks small,
what to inspect before changing code, and how to decide whether a result is acceptable.

For detailed workflow, see `02_workflow.md`.
For naming and style, see `STYLE.md`.

---

## Core principle

Every interaction with an AI assistant or code agent should be framed as a small,
concrete, verifiable task.

Good tasks are narrow enough to review easily and specific enough to validate quickly.

Examples of good task scope:

- add one missing init function
- split one mixed-responsibility module
- rename one public API to match naming rules
- create one service module following an existing example
- review whether one file belongs in `app/` or `services/`

Examples of bad task scope:

- rewrite the whole architecture
- clean up the entire codebase
- redesign all naming
- refactor everything related to initialization
- reorganize all folders in one pass

---

## Start sequence

Before asking an agent to generate or modify code:

1. define the exact task
2. identify the expected owning folder
3. identify the affected module or file
4. read `STYLE.md`
5. inspect the closest matching example in `style-examples/`
6. confirm whether the task is additive, corrective, or refactor-only
7. define what "done" means

If any of these are unclear, reduce the scope before proceeding.

---

## How to define a good task

A good task should answer:

- What needs to change?
- Where should it change?
- Why does that file or module own the change?
- What existing pattern should be followed?
- What should remain untouched?

Recommended task statement format:

> Update `<target file or module>` to `<specific change>`, following `STYLE.md`
> and the closest relevant file under `style-examples/`.
> Keep the change minimal and do not expand scope beyond `<boundary>`.

---

## How to choose the owning folder

Use existing repository responsibility boundaries.

Typical rule of thumb:

- choose `app/` for orchestration and product behavior
- choose `bsp/` for board-level integration
- choose `cube/` only when aligned with vendor-generated or low-level MCU startup or config
- choose `port/` for adaptation boundaries
- choose `services/` for reusable internal modules not owned by `app/` or hardware specifics

Do not place code based only on convenience.
Place code where future maintenance will be most obvious.

---

## What to inspect before editing

Before generating changes, inspect:

- the target file itself
- neighboring files in the same module
- the naming rules in `STYLE.md`
- an existing example with similar public and private split
- whether the change introduces a new responsibility into a file that already has another purpose

If a file already mixes concerns, do not expand that mistake automatically.
Call it out and keep the change as contained as possible.

---

## What to use from `style-examples/`

Use examples to learn:

- file layout
- function naming
- public versus private symbol split
- include ordering
- internal helper placement
- callback naming
- wrapper naming
- minimal module structure

Do not use examples as final architecture templates.
They are style and module-form references, not product design templates.

---

## What to avoid

Avoid these common failures:

- creating new naming styles
- mixing public API and internal orchestration without structure
- exposing helpers that should stay private
- creating folders or layers without repository precedent
- adding abstractions "for later"
- broad cleanup inside a task that was supposed to be narrow
- using examples mechanically without checking current repository structure

---

## Definition of done

A task is in good shape when:

- the target folder is justified
- the module responsibility is clear
- naming matches `STYLE.md`
- public and private split is consistent
- no extra architectural change was introduced unintentionally
- the patch is easy to review
- the result matches an existing repository pattern where possible

---

## Minimal completion checklist

Before finalizing:

- [ ] task is concrete and bounded
- [ ] target folder is correct
- [ ] naming was checked
- [ ] file split is coherent
- [ ] closest example was reviewed
- [ ] no unnecessary scope expansion happened
- [ ] result is reviewable in isolation

---

## Related documents

- Full process: `02_workflow.md`
- Style and naming: `STYLE.md`
- Agent behavior: `AGENTS.md`
- Examples index: `style-examples/README.md`
- Live continuity: `05_working_context.md`
