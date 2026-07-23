# Working Context: FMC Product Documentation Backbone

## Objective

Create the minimal product-documentation backbone for the complete FMC-320U and
give its principal functional domains broad, superficial coverage.

The repository must identify where reviewed product knowledge belongs, separate
decision state from evidence, and name the next slice that requires depth.

## Motivation

The repository has technical specifications, working inventories, implemented
contracts, and legacy evidence, but it does not yet have a clear owner for
reviewed FMC product requirements, observable behavior, and user interface
knowledge. Without that backbone, detailed evidence can be mistaken for
accepted product direction and important domains can remain invisible.

## Status

Implemented; awaiting human audit and acceptance.

## Scope

- create the FMC product-documentation entrypoint;
- establish separate owners for requirements, cross-cutting behavior, and user
  interface knowledge;
- map the whole product at superficial depth;
- apply `Accepted`, `Candidate`, `Unresolved`, and `Deferred` to substantive
  statements;
- record evidence separately as provenance, not as a decision state;
- integrate the new ownership model into repository entrypoints and closure
  rules;
- identify Phase 6A as the next planned documentation depth.

## Out Of Scope

- firmware, build, CubeMX, roadmap, or LCD technical-specification changes;
- implementation of Phase 6A;
- detailed Word or legacy migration;
- mass transcription of `docs/specs/fmc/use_cases.yaml`;
- complete requirements, state machine, navigation, or screen specifications;
- screen schemas or files per screen;
- detailed resolution of legacy contradictions;
- removal of legacy evidence or deferred functions.

## Sources

Policy and continuity:
- `AGENTS.md`;
- `docs/project/WORKFLOW.md`;
- `docs/workflow/README.md`;
- `docs/workflow/doc_closure.md`.

Product strategy, contracts, inputs, and evidence:
- `docs/roadmaps/fmc_refactoring.md`;
- `src/product/fmc/` public headers and local README;
- `src/apps/product/main/README.md`;
- `docs/specs/README.md`;
- `docs/specs/fmc/use_cases.yaml`;
- `docs/specs/lcd/lcd_true_source.yaml`;
- `legacy/README.md`;
- `legacy/analysis/module_inventory.md`;
- `legacy/analysis/migration_ledger.md`.

## Authorities

- `docs/product/fmc/` owns current reviewed FMC product documentation.
- `docs/product/fmc/requirements.md` owns product obligations.
- `docs/product/fmc/behavior.md` owns cross-cutting observable behavior.
- `docs/product/fmc/user_interface.md` owns the visible and operable
  experience.
- `docs/specs/` owns structured technical specifications, normalized
  inventories, and engineering inputs.
- `docs/specs/fmc/use_cases.yaml` remains an evolving working inventory; its
  `confirmed` status does not mean `Accepted`.
- `docs/specs/lcd/lcd_true_source.yaml` remains the technical authority for LCD
  glass and physical mapping.
- public headers own implemented module contracts.
- legacy sources remain evidence, not authority.

## Deliverables

- `docs/product/fmc/README.md`;
- `docs/product/fmc/requirements.md`;
- `docs/product/fmc/behavior.md`;
- `docs/product/fmc/user_interface.md`;
- updated repository, specification, workflow, and closure entrypoints.

## Verifiable Closure Criteria

- all four FMC product documents exist and link to each other correctly;
- principal product domains have superficial coverage;
- decision states are visible on substantive content and evidence is separate;
- document ownership is unambiguous and normative content is not duplicated;
- no working inventory or legacy evidence is promoted automatically;
- LCD physical mapping is referenced without duplication;
- Phase 6A is identified as the next planned depth, not active implementation;
- authority statements for `docs/product/`, `docs/specs/`, working inventories,
  LCD facts, contracts, and legacy evidence are consistent;
- no files outside the approved documentation scope changed;
- documentation closure and Git verification pass;
- the human audits and accepts this implemented backbone or requests a focused
  correction.
