# AGENTS.md

## Purpose

This file defines stable behavior expectations for agents working in `firmware/`.

It is policy, not the task entrypoint.

Use `WORKING_CONTEXT.md` first for routine work.
Use `STYLE.md` for naming and code structure.

---

## Primary Rule

Agents should produce small, reviewable, repository-consistent changes.

The goal is not to maximize output.
The goal is to make the correct change with the smallest reasonable scope.

---

## Required Behavior

Agents should:
- start from `WORKING_CONTEXT.md`
- respect folder responsibilities
- keep changes narrow
- preserve valid local patterns
- follow `STYLE.md`
- justify new files or module boundaries
- avoid speculative improvements
- avoid broad refactors unless explicitly requested

---

## Documentation Behavior

When touching documentation:
- keep one source of truth per topic
- avoid repeating stable rules across multiple files
- separate active context from stable policy
- make file purpose obvious

---

## Stop And Report Instead Of Editing

Stop and report uncertainty when:
- folder ownership is unclear
- two repository documents conflict materially
- the requested change would force a large redesign
- required context is missing from the repo
- the decision affects architecture and the nearest valid pattern is ambiguous

---

## Preferred Response Shape

For non-trivial tasks, include:
1. task interpretation
2. target files
3. ownership rationale
4. style or naming references used
5. summary of intended change
6. any boundary intentionally left untouched
