# AGENT_ENTRY.md

## Purpose

This is the primary operational entry file for AI agents working in `firmware/`.

Use this file as the default entrypoint when assigning analysis, planning,
implementation, comment-pass, or validation work to an agent.

This file defines:
- the stable source of truth for repository rules
- the workflow entrypoints to use by stage
- the minimum expectations for generated work

---

## Read this first

For any non-trivial task, use this reading order:

1. `STYLE.md`
2. `AGENTS.md`
3. `workflows/README.md`
4. the stage document relevant to the current pass, if it exists
5. `style-examples/README.md`
6. the closest relevant example under `style-examples/`
7. `WORKING_CONTEXT.md` only if current session continuity matters

Do not read more documentation than the task needs.
Use the workflow stage to narrow the required context.

---

## Source of truth by topic

| Topic | Source of truth |
|---|---|
| Workflow stage model | `workflows/README.md` |
| Comment pass rules | `workflows/comment_pass.md` |
| Style and naming | `STYLE.md` |
| Agent behavior and constraints | `AGENTS.md` |
| Examples and reference patterns | `style-examples/README.md` |
| Live task continuity | `WORKING_CONTEXT.md` |

If a workflow document conflicts with `STYLE.md` on naming or code style,
`STYLE.md` wins.

---

## Workflow usage rule

Do not assume every request should be handled in a single pass.

Prefer working by stage when needed:
- analysis
- plan
- implementation
- comment pass
- validation

Examples:
- analyze this module first
- analyze and propose a plan
- implement only
- run a comment pass on these files
- run validation on the current change

Use the stage model in `workflows/README.md`.
If a stage-specific document exists, follow it.

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
- Do not keep competing local methods alive inside the same touched file when a workflow stage defines a repository method.

---

## Folder responsibility reminder

Before creating or moving code, confirm the owning area.

Typical responsibility split:

- `apps/`  
  Product or feature-level behavior, orchestration, and application logic.

- `bsp/`  
  Board-specific control, pin-related setup, and hardware-level board integration.

- `cube/`  
  Vendor-generated or vendor-aligned initialization and low-level MCU support.

- `port/`  
  Adaptation layer between platform or framework specifics and project code.

- `services/`  
  Reusable internal services that are not board-specific and are not direct app orchestration.

- `libs/`  
  Reusable project modules that expose local firmware capabilities without being board bring-up code.

If unsure, do not invent a new folder role.
Match the closest existing pattern in the repository.

---

## Expected output from an agent

Every non-trivial agent output should make these points clear:

- what exact task or stage is being solved
- what files are affected
- why the selected folder or module is the right owner
- which naming or style rules were applied
- whether an existing example was followed
- whether the change is intentionally minimal
- what remains for a later pass, if anything

---

## Required pre-output checklist

Before finalizing code or a patch, verify:

- task or stage scope is small and concrete
- folder ownership is correct
- naming was checked against `STYLE.md`
- public and private split is consistent
- responsibilities are not mixed
- no unnecessary new abstractions were introduced
- the closest relevant example was reviewed if applicable
- the result is easy to review
- the claimed stage was actually completed

---

## Prompt reference

When assigning work to an agent, reference:

- `AGENT_ENTRY.md`
- `STYLE.md`
- `workflows/README.md`
- the stage document if one exists
- the closest file under `style-examples/` when relevant

Minimal prompt pattern:

> Follow `AGENT_ENTRY.md` and `STYLE.md`.  
> Use `workflows/README.md` and the relevant stage document for this pass.  
> Use the closest relevant example under `style-examples/` when needed.  
> Keep the change minimal and consistent with the existing folder responsibilities.

---

## What this file is not

This file is not:

- the full style guide
- the full prompt library
- the mutable project status log
- the detailed instructions for every workflow stage

Those live in the dedicated files referenced above.
