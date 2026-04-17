# CONTRIBUTING.md

## Purpose

This file explains how to contribute changes under `firmware/`.

It focuses on change quality, scope control, and alignment with the staged workflow.

For workflow stages, see `workflows/README.md`.
For style and naming, see `STYLE.md`.
For agent entry guidance, see `AGENT_ENTRY.md`.

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

## Workflow usage

Contributions may be split into stages:

- analysis
- plan
- implementation
- comment pass
- validation

Not all tasks require all stages.

Prefer explicit stage-based changes over mixed-purpose patches.

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
- do not duplicate workflow definitions
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
- Workflow model: `workflows/README.md`
- Comment pass: `workflows/comment_pass.md`
- Style and naming: `STYLE.md`
- Agent behavior: `AGENTS.md`
