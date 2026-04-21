# Methodological Passes

## Purpose

This directory contains methodological passes used after the main workflow stage
has already been identified.

These passes are not the human-facing wizard entrypoint.
Use:
- `WORKING_CONTEXT.md` first for active work
- `docs/workflow/wizard/README.md` when starting or reframing a new workstream

---

## Pass Types

Bootstrap passes:
- `refactor/`
- `new_feature/`

Bootstrap passes may:
- create `WORKING_CONTEXT.md`
- reframe `WORKING_CONTEXT.md`

Non-bootstrap passes:
- `comments/`
- `naming_style/`
- `contracts/`
- `safety/`

Non-bootstrap passes:
- require an existing `WORKING_CONTEXT.md`
- operate inside the current context
- do not create or replace `WORKING_CONTEXT.md`

---

## Prompt Placement

Prompts belong to the pass that owns them.

Examples:
- bootstrap prompts for `refactor/` live under `refactor/`
- bootstrap prompts for `new_feature/` live under `new_feature/`
- support prompts for `comments/` live under `comments/`

Do not rely on a global prompts folder for pass-owned behavior.

The wizard may call into bootstrap passes internally, but the wizard itself is
the human-facing entry layer.
