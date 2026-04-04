# CONTRIBUTING.md

## Purpose

This file explains how to contribute changes under `firmware/`.

It focuses on change quality, scope control, and documentation alignment.

For workflow details, see `02_workflow.md`.
For style and naming, see `STYLE.md`.
For agent-oriented entry guidance, see `AGENT_ENTRY.md`.

---

## Contribution principles

Contributions should be:

- small when possible
- clearly scoped
- consistent with repository conventions
- easy to review
- aligned with existing folder responsibilities

Avoid mixing unrelated improvements into one change.

---

## Before modifying code

Before making a code change:

1. define the exact task
2. identify the target folder and module
3. review `STYLE.md`
4. inspect the closest relevant example in `style-examples/`
5. confirm what is explicitly out of scope

If the task is broad, split it before implementation.

---

## Preferred change types

Preferred contribution size:

- one bug fix
- one local refactor
- one new small module
- one API naming correction
- one documentation cleanup
- one responsibility split with clear ownership

Less preferred:

- large mixed refactors
- broad folder reorganization without prior design agreement
- naming sweeps mixed with behavior changes
- speculative cleanup outside the task boundary

---

## Documentation expectations

When changing documentation:

- keep one topic per file
- keep one source of truth per rule
- do not store stable rules in working-context files
- do not embed one document inside another when it should be a real file
- keep prompt libraries separate from normative documentation

---

## Code expectations

Code contributions should:

- follow `STYLE.md`
- keep public APIs minimal
- keep private helpers local
- respect module ownership
- avoid unnecessary abstraction
- match repository patterns where valid

---

## Review checklist

Before considering a contribution ready:

- [ ] task scope is clear
- [ ] target ownership is justified
- [ ] naming matches `STYLE.md`
- [ ] no unrelated edits were introduced
- [ ] documentation was updated if a stable rule changed
- [ ] examples were consulted if relevant
- [ ] patch is reasonably easy to review

---

## Related documents

- Agent entry: `AGENT_ENTRY.md`
- Quick start: `01_quickstart.md`
- Workflow: `02_workflow.md`
- Style and naming: `STYLE.md`
- Agent behavior: `AGENTS.md`
- Examples index: `style-examples/README.md`
