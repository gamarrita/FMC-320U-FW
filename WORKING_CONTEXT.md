# Working Context: FMC Legacy Source Classification

## Objective

Separate the primary historical FMC source, its frozen structured extraction,
the mutable coverage register, and current product documentation.

This context preserves provenance and prepares controlled future review. It
does not make functional product decisions or deepen Phase 6A.

## Motivation

The derived FMC use-case extraction was stored under `docs/specs/`, where its
location and working-inventory wording could make it appear current or mutable.
The repository needs an explicit chain from historical source to frozen
extraction, human review record, and reviewed product documentation.

## Status

Implemented; awaiting human audit and acceptance.

## Scope

- locate the historical Word source without opening or reprocessing it;
- move the YAML extraction byte-for-byte into frozen legacy evidence;
- create a mutable coverage and disposition register;
- update repository references and authority descriptions affected by the
  move;
- preserve current FMC product documentation as the only current product
  authority.

## Out Of Scope

- reading or modifying the historical Word;
- editing, cleaning, splitting, or summarizing the frozen YAML extraction;
- accepting, rejecting, replacing, or incorporating legacy content;
- changing current product requirements, behavior, or UI decisions;
- implementation, firmware, build, CubeMX, or LCD technical-authority changes;
- Phase 6A definition or implementation.

## Sources

- `legacy/specs/fmc/use_cases.docx`: primary historical source;
- `legacy/derived/fmc/use_cases.extraction-v1.yaml`: frozen derived extraction;
- `docs/workflow/fmc_legacy_coverage.md`: mutable coverage register;
- `docs/product/fmc/`: current reviewed product documentation;
- `docs/specs/lcd/lcd_true_source.yaml`: technical LCD authority;
- `legacy/README.md`: legacy ownership and preservation rules;
- `AGENTS.md`: stable repository policy;
- `docs/workflow/doc_closure.md`: documentation closure rules.

## Authorities And Roles

- `legacy/specs/fmc/use_cases.docx` is the primary historical source and is
  consulted only to audit extraction ambiguity, omission, or suspected error.
- `legacy/derived/fmc/use_cases.extraction-v1.yaml` is frozen derived legacy
  evidence and the preferred legacy source for routine future consultation.
- `docs/workflow/fmc_legacy_coverage.md` is the mutable record of review
  coverage and human-decided disposition; it is not a functional authority.
- `docs/product/fmc/requirements.md` owns current product obligations.
- `docs/product/fmc/behavior.md` owns current cross-cutting observable behavior.
- `docs/product/fmc/user_interface.md` owns the current visible and operable
  experience.
- `docs/specs/lcd/lcd_true_source.yaml` remains the technical authority for LCD
  glass and physical mapping.

## Deliverables

- frozen extraction at
  `legacy/derived/fmc/use_cases.extraction-v1.yaml`, with content preserved;
- mutable coverage register at
  `docs/workflow/fmc_legacy_coverage.md`;
- updated references with no second extraction copy under `docs/specs/fmc/`.

## Next Context Candidate

`FMC Phase 6A Presentation Definition`

This is a candidate only. It is not active or started by this context.

## Verifiable Closure Criteria

- the repository path of the historical Word is recorded without opening it;
- the extraction SHA-256 matches its pre-move value;
- no extraction copy remains under `docs/specs/fmc/`;
- every top-level YAML locator starts as `Not evaluated`;
- no functional disposition is inferred;
- all references use the new path and frozen-evidence role;
- current product documentation retains its existing ownership and content;
- the LCD technical authority is unchanged;
- no firmware, build, CubeMX, Word, or YAML content changed;
- documentation closure and Git verification pass.
