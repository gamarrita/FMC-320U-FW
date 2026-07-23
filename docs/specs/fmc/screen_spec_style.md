# FMC Screen Specification Style

## Purpose

This file defines how to write FMC screen behavior documents for Phase 6.

It is documentation style, not firmware coding style. Use `STYLE.md` for C
code, public contracts, naming, comments, and module boundaries.

## Scope

Apply this style to product-screen behavior documents under:

```text
docs/specs/fmc/screens/
```

The first documentation slice covers only:
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

Do not add new tooling only to complete the first documentation slice.

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

Current validation for Phase 6 will be designed by the implementation slice
after the human accepts the screen behavior documents.

## Legacy Evidence

Legacy sources are evidence, not automatic authority.

When preserving legacy-derived behavior, name the source briefly without copying
large source passages. If legacy evidence conflicts with the current
specification or with known hardware behavior, record the conflict and stop for
human review.

## Human Review Gate

New screen documents are draft behavior until the human accepts them.

Do not implement Phase 6 UI state, LCD rendering, navigation, timers, or product
behavior from a new screen document until that review has happened.

## Agent Workflow

For each selected screen:
1. read the current `use_cases.yaml` screen entry and related shared sections;
2. read the available legacy Word evidence when available;
3. extract only behavior needed to understand the selected screen;
4. classify each extracted fact as confirmed, inferred, unresolved, or legacy
   evidence;
5. write a concise Markdown screen document;
6. report what was intentionally not migrated.

Do not split the entire FMC behavior catalog unless the human explicitly asks
for that broader migration.
