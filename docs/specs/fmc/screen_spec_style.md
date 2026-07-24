# FMC Screen Specification Style

## Purpose

This file preserves the proposed style for a future approved FMC screen
documentation slice. It is not an active plan and does not authorize creating
screen files or starting Phase 6A.

It is documentation style, not firmware coding style. Use `STYLE.md` for C
code, public contracts, naming, comments, and module boundaries.

## Scope

When an approved future context calls for separate product-screen behavior
documents, apply this style in the location approved by the owning product
documentation. Phase 6A remains owned by
`docs/product/fmc/user_interface.md` and does not require one file per screen.

The previously proposed initial slice was limited to:
- startup all segments;
- firmware version;
- TTL/RATE steady user screen.

Do not use this style to specify firmware implementation details, test plans,
LCD RAM mapping, HAL behavior, ThreadX behavior, or generated CubeMX
configuration.

## Format

Prefer Markdown files.

Use YAML only when a small structured block is clearly more readable than prose.
Do not convert full screen behavior into YAML by default.

Reasons:
- Markdown is easy for humans to read and edit;
- Markdown diffs are reviewable;
- agents can parse consistent headings reliably;
- behavior prose stays distinct from machine-oriented configuration.

Optional future tools:
- `markdownlint` for Markdown structure and formatting checks;
- `Vale` for prose terminology and editorial consistency;
- schema validation only if a later slice introduces stable structured data.

Do not add new tooling only to complete a screen documentation slice.

## File Naming

Use short, stable, human-readable lowercase filenames:

```text
startup_all_segments.md
firmware_version.md
ttl_rate.md
```

Do not force legacy screen IDs as the primary contract. A screen document may
include a stable key or legacy name only when it improves traceability or future
implementation, and the reason should be visible in the document.

## Screen Document Shape

Use only the sections that help a human review the behavior. Avoid empty
template sections.

Recommended headings:

```markdown
# <Human Screen Name>

## Status

## Purpose

## Visible Behavior

## Inputs And Transitions

## Data Dependencies

## Unresolved Notes

## Legacy Evidence
```

Keep each screen document focused on the behavior visible from that screen.
Move shared behavior into another document only after duplication becomes a
real review problem.

## Content Rules

Write confirmed behavior as direct requirements:

```markdown
- The screen remains visible for 3 seconds unless skipped by ESC.
```

Write uncertainty as unresolved, not as implementation behavior:

```markdown
- Unresolved: the current source does not define the exact firmware-version
  string source.
```

Write inferred content explicitly:

```markdown
- Inferred: the current LCD BSP may need an all-segments helper before this
  screen can be implemented cleanly.
```

Do not mix these categories in one bullet.

## Validation And Tests

Do not inherit legacy validation methods as current test policy.

Legacy test IDs, smoke procedures, acceptance notes, or measurement scripts are
not part of the screen behavior contract. If they are useful, keep them only as
legacy evidence or traceability notes inside the relevant screen document.

Phase 6A validation is owned by its implemented contracts and regression
coverage. Validation for future screen slices is designed only after the human
accepts their behavior documentation.

## Legacy Evidence

Legacy sources are evidence, not automatic authority.

When preserving legacy-derived behavior, name the source briefly without copying
large source passages. If legacy evidence conflicts with reviewed current
product documentation or known hardware behavior, record the conflict and stop
for human review.

## Human Review Gate

New screen documents are draft behavior until the human accepts them.

Do not implement UI state, LCD rendering, navigation, timers, or product
behavior from a new screen document until that review has happened.

## Agent Workflow

Only when an approved active context selects screens:
1. read relevant entries from the frozen
   `legacy/derived/fmc/use_cases.extraction-v1.yaml` as legacy evidence, not
   automatically accepted requirements;
2. consult `legacy/specs/fmc/use_cases.docx` only to audit a concrete
   discrepancy in the frozen extraction;
3. extract only behavior needed to understand the selected screen;
4. classify each extracted fact as confirmed, inferred, unresolved, or legacy
   evidence;
5. write a concise Markdown screen document;
6. report what was intentionally not migrated.

Do not split the entire FMC behavior catalog unless the human explicitly asks
for that broader migration.
