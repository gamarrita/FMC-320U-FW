# Workflow Model

## Purpose

This file keeps the repo's default stage vocabulary small and stable.

Use `WORKING_CONTEXT.md` first when an active workstream exists.
Use this file only to interpret stage names consistently.

## Default Stages

Routine work normally moves through:

1. analysis
2. plan
3. implementation
4. validation
5. closure

Small tasks may combine stages.

## Stage Meanings

- `analysis`: understand the current code, ownership, and risks.
- `plan`: define the smallest correct change.
- `implementation`: make the requested change.
- `validation`: verify the result and report what was or was not checked.
- `closure`: converge documentation, naming, or small polish after behavior is stable.
  Apply `docs/workflow/doc_closure.md` when documentation or active
  context changed.

## Context Rule

When a request starts a new track or no longer fits the current one:
- refresh `WORKING_CONTEXT.md`
- refresh one matching file under `docs/contexts/`
- keep the short file operational and the long file explanatory

## Related Docs

- `AGENTS.md`: stable agent policy
- `WORKING_CONTEXT.md`: active workstream
- `STYLE.md`: naming and code structure
- `CONTRIBUTING.md`: repo hygiene
- `docs/workflow/doc_closure.md`: documentation closure checklist
