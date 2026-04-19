# Methodological Passes

## Purpose

This directory contains methodological passes used after the main workflow stage
has already been identified.

These passes are not entrypoints.
Use `WORKING_CONTEXT.md` first.

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
