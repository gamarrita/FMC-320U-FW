# Legacy firmware reference

## Purpose

This directory holds the historical authored firmware used as evidence during
the FMC-320U refactor.

The canonical historical source is the exact content originally stored under:

- `FLOWMEET/`
- `libs/`

These names and boundaries are historically accurate even though they do not
represent the architecture targeted by the current refactor.

## Intended layout

- `source/FLOWMEET/`: original product and runtime-oriented authored files.
- `source/libs/`: original authored modules grouped as libraries in the legacy
  project.
- `analysis/`: inventories, dependency maps, and migration tracking. Analysis
  may classify responsibilities using the current architecture, but it must not
  rewrite the historical source layout.

## Rules

- Treat `legacy/source/` as evidence, not authority.
- Preserve files in `legacy/source/` without renaming, reformatting, fixing, or
  reorganizing them.
- Do not include `legacy/source/` in the active build.
- Do not include headers from `legacy/source/` in active code.
- Do not copy complete legacy modules into `src/`.
- Extract required behavior and validate it as a deliberate current product
  decision before implementing it in the new architecture.
- Git history is sufficient for obsolete refactor experiments; do not use this
  directory as an archive for inactive bring-up applications.

## Source status

The source snapshot must be populated from the supplied legacy ZIP. The current
`legacy_backup/` directory is not canonical: it mixes partial legacy files,
post-hoc BSP classification, and obsolete refactor bring-up applications.

Do not remove `legacy_backup/` until the extracted snapshot has been committed
and compared for unique content.
