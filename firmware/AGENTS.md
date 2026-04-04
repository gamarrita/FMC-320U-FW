# AGENTS.md

## Purpose

This file describes the expected behavior of AI agents working in `firmware/`.

It complements `AGENT_ENTRY.md`.
It does not replace `STYLE.md` as the source of truth for naming and code structure.

---

## Primary rule

Agents must produce small, reviewable, repository-consistent changes.

The goal is not to maximize code output.
The goal is to make the correct change with the smallest reasonable scope.

---

## Required behavior

Agents must:

- read the referenced repository docs before editing
- respect existing folder responsibilities
- keep changes narrow
- preserve existing valid patterns
- follow `STYLE.md` for naming and structure
- justify new files or module boundaries
- avoid speculative improvements
- avoid broad refactors unless explicitly requested

---

## Required references

For non-trivial tasks, agents should use:

- `AGENT_ENTRY.md`
- `01_quickstart.md`
- `STYLE.md`
- `style-examples/README.md`
- the closest relevant example under `style-examples/`

Use `02_workflow.md` when the task involves process or handoff questions.
Use `05_working_context.md` only when current session continuity matters.

---

## Scope control

Agents should prefer:

- one focused task
- one clear responsibility
- one reviewable change set

Agents should avoid:

- mixing cleanup with implementation
- renaming unrelated code
- moving files without clear ownership reason
- inventing new layers or abstractions without repository precedent
- converting a local fix into a broad redesign

---

## Code generation expectations

Generated code should:

- match repository naming
- follow public and private split conventions
- keep headers minimal
- keep helpers local to implementation files
- preserve readability
- avoid unnecessary wrappers or indirection
- fit the local module style

If a local file already follows a valid pattern,
prefer matching it over introducing a slightly different style.

---

## Documentation behavior

When updating documentation, agents should:

- preserve one source of truth per topic
- avoid duplicating stable rules across multiple files
- separate stable docs from mutable context
- keep prompt libraries separate from policy documents
- make file purpose obvious from filename and content

---

## When to stop and report instead of editing

Agents should stop and report uncertainty if:

- folder ownership is unclear
- two repository documents conflict materially
- a requested change would force a large redesign
- the task depends on missing context not available in the repo
- the closest valid example is ambiguous and the decision affects architecture

In such cases, the agent should report:
- the uncertainty
- the affected files or boundaries
- the minimum clarification needed

---

## Preferred output format

For non-trivial tasks, agent responses should include:

1. task interpretation
2. target files
3. ownership rationale
4. style or naming references used
5. summary of intended change
6. any boundary intentionally left untouched

This makes review easier and keeps scope visible.

---

## Summary

Agent behavior priorities:

1. correctness
2. consistency
3. clarity
4. minimal scope
5. reviewability

Do not optimize for novelty or breadth.
Optimize for maintainable progress.
