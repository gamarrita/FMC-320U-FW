# CHANGELOG.md

## Purpose

This file records meaningful firmware-level changes, milestones, and notable decisions.

It is a historical record.
It is not a source of truth for current style, naming, or workflow rules.

For stable rules, see:

- `WORKING_CONTEXT.md`
- `STYLE.md`
- `AGENTS.md`
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
