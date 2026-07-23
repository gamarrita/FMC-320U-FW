# Specs

This directory contains structured technical specifications, normalized
inventories, and engineering inputs.

Current reviewed FMC product documentation belongs under
`docs/product/fmc/`. A file in `docs/specs/` is authoritative only for the
technical scope it explicitly owns.

Rules:
- keep structured, implementation-facing specs here
- keep normalized working inventories here without treating their entries as
  automatically accepted product decisions
- keep direct source artifacts here when they are the canonical source for a
  technical specification
- use `legacy/source/` as frozen legacy-code evidence
- keep normalized specs separate from the historical source snapshot

FMC entrypoints:
- current product documentation: `docs/product/fmc/README.md`
- product behavior working inventory: `docs/specs/fmc/use_cases.yaml`
- technical LCD authority: `docs/specs/lcd/lcd_true_source.yaml`
