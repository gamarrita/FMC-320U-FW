# CHANGELOG.md

## Purpose

This file records meaningful firmware-level changes, milestones, and notable decisions.

It is a historical record.
It is not a source of truth for current style, naming, or workflow rules.

For stable rules, see:

- `AGENT_ENTRY.md`
- `01_quickstart.md`
- `02_workflow.md`
- `STYLE.md`
- `AGENTS.md`

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

## 2026-04-04
- Reorganized firmware documentation structure around a single agent entrypoint.
- Separated quickstart, full workflow, prompt libraries, style guide, and working context.
- Clarified that style and naming rules live in `STYLE.md`.
- Clarified that live session continuity belongs in `05_working_context.md`.
- Added `style-examples/README.md` as the index for example usage.
- Renamed `port/fm_hw_*` modules and public APIs to `fm_port_*` / `FM_PORT_*` for clearer platform-layer ownership.
