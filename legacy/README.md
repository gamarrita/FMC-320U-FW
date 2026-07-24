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
- `specs/fmc/use_cases.docx`: primary historical FMC use-case source, reserved
  for exceptional extraction audits.
- `derived/fmc/use_cases.extraction-v1.yaml`: frozen structured extraction used
  as the preferred FMC legacy evidence for later review.
- `derived/fmc/README.md`: classification and preservation contract for that
  extraction.
- `analysis/`: inventories, dependency maps, and migration tracking. Analysis
  may classify responsibilities using the current architecture, but it must not
  rewrite the historical source layout.

## Rules

- Treat `legacy/source/` as evidence, not authority.
- Preserve `legacy/specs/` primary historical documents without modification.
- Preserve versioned files under `legacy/derived/` without silent correction;
  record suspected conversion errors in the applicable coverage register.
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

The source snapshot has been imported from the supplied historical ZIP.

Verified imported content:
- 42 authored files
- 20 `.c` files
- 20 `.h` files
- 2 `.md` files

`legacy/source/` is now the canonical historical source reference for authored
legacy firmware. The original `FLOWMEET/` and `libs/` organization is preserved
intentionally because it records how the firmware was actually organized.

The removed `legacy_backup/` tree was not canonical. It mixed partial legacy
files, post-hoc BSP classification, and obsolete refactor bring-up
applications. Git history is sufficient to recover those experiments if needed.

Future migration work should extract behavior and contracts from this snapshot.
It should not copy complete modules or reproduce the old architecture inside
`src/`.

FMC legacy review disposition is tracked in
`docs/workflow/fmc_legacy_coverage.md`. Current reviewed FMC product
documentation remains under `docs/product/fmc/`.
