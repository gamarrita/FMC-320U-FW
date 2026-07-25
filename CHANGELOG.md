# CHANGELOG.md

## Purpose

This file records meaningful firmware-level changes, milestones, and notable decisions.

It is a historical record.
It is not a source of truth for current style, naming, workflow rules, or
active work.

For current repository guidance, see:

- `AGENTS.md`
- `WORKING_CONTEXT.md`
- `STYLE.md`
- `docs/workflow/README.md`

---

## Format

Use concise entries that capture:

- what changed
- why it mattered
- any important boundary or decision

Recommended format:

```text
## YYYY-MM-DD
- <change summary>
- <impact or rationale>
```

---

## Entries

## 2026-07-25
- Normalized the active workflow around one phase-aligned workstream and one
  active slice, with the same `WORKING_CONTEXT.md` persisting between slices.
- Reserved documentation closure for formally closing, deferring, or replacing
  a workstream instead of routine tasks, commits, handoffs, or slice changes.

## 2026-07-24
- Established the human-agent-repository continuity workflow and separated
  current FMC product documentation, frozen legacy evidence, and mutable legacy
  review coverage.
- Completed and hardware-accepted the bounded Phase 6A startup presentation
  path through steady TTL/RATE with controlled values.

## 2026-05-10
- Added the first pure FMC model/unit-policy validation app.
- Kept FMC runtime, presentation, LCD adapter, and pulse acquisition out of the
  model/units baseline while making the slice build-verifiable.

## 2026-04-17
- Introduced staged workflow model under `docs/workflow/`.
- Added `comment_pass.md` as canonical method for comment consistency.
- Aligned repository entrypoints to the staged workflow.
- Began consolidation of legacy workflow documents.

## 2026-04-05
- Formalized a reproducible STM32 environment outside the IDE, usable from terminal by both humans and agents.
- Added STM32 bootstrap scripts for PowerShell and Git Bash to discover the tooling and prepare the effective build environment.
- Documented the canonical build workflow of the repo.

## 2026-04-04
- Reorganized firmware documentation structure around `WORKING_CONTEXT.md` as the single operational entrypoint.
- Separated quickstart, full workflow, prompt libraries, style guide, and working context.
