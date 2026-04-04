# AGENT_ENTRY.md

## Purpose

This is the primary entry file for AI agents working in `firmware/`.

Use this file as the default reference when assigning implementation, refactor,
review, or documentation tasks to an agent.

This file defines the mandatory reading order, the source of truth for each topic,
and the minimum behavioral expectations for generated work.

---

## Mandatory reading order

For any non-trivial task, read in this order:

1. `01_quickstart.md`
2. `STYLE.md`
3. `AGENTS.md`
4. `02_workflow.md`
5. `style-examples/README.md`
6. The closest relevant example under `style-examples/`
7. `05_working_context.md` only if current task continuity matters

---

## Source of truth by topic

| Topic | Source of truth |
|---|---|
| Quick task startup | `01_quickstart.md` |
| Full workflow | `02_workflow.md` |
| Style and naming | `STYLE.md` |
| Agent behavior and constraints | `AGENTS.md` |
| Examples and reference patterns | `style-examples/README.md` |
| Live task continuity | `05_working_context.md` |

---

## Non-negotiable rules

- Work in small, concrete, reviewable tasks.
- Do not assume naming by intuition.
- Check `STYLE.md` before creating or renaming APIs, modules, files, or symbols.
- Use `style-examples/` only as reference for code form, naming, file split, and visibility patterns.
- Do not treat `style-examples/` as target architecture.
- Keep responsibilities separated by folder and module.
- Keep public APIs minimal.
- Keep private helpers local to the `.c` file unless a stronger reason exists.
- Avoid broad refactors unless the task explicitly asks for them.
- Prefer consistency with the repository over personal preference.

---

## Folder responsibility reminder

Before creating or moving code, confirm the owning area.

Typical responsibility split:

- `app/`  
  Product or feature-level behavior, orchestration, and application logic.

- `bsp/`  
  Board-specific control, pin-related setup, and hardware-level board integration.

- `cube/`  
  Vendor-generated or vendor-aligned initialization and low-level MCU support.

- `port/`  
  Adaptation layer between platform or framework specifics and project code.

- `services/`  
  Reusable internal services that are not board-specific and are not direct app orchestration.

If unsure, do not invent a new folder role.
Match the closest existing pattern in the repository.

---

## Expected output from an agent

Every non-trivial agent output should make these points clear:

- what exact task is being solved
- what files are affected
- why the selected folder or module is the right owner
- which naming or style rules were applied
- whether an existing example was followed
- whether the change is intentionally minimal

---

## Required pre-output checklist

Before finalizing code or a patch, verify:

- task scope is small and concrete
- folder ownership is correct
- naming was checked against `STYLE.md`
- public and private split is consistent
- responsibilities are not mixed
- no unnecessary new abstractions were introduced
- the closest relevant example was reviewed if applicable
- the result is easy to review

---

## Prompt reference

When assigning work to an agent, reference:

- `AGENT_ENTRY.md`
- `STYLE.md`
- the closest file under `style-examples/`

Minimal prompt pattern:

> Follow `AGENT_ENTRY.md` and `STYLE.md`.  
> Use the closest relevant example under `style-examples/`.  
> Keep the change minimal and consistent with the existing folder responsibilities.

---

## What this file is not

This file is not:

- the full workflow description
- the full style guide
- the current project status log
- a prompt template library

Those live in the dedicated files referenced above.
