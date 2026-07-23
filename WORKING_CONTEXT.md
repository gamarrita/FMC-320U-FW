# Working Context: Product Documentation Reframe

## Active Objective

Reframe the repository documentation so that the repository, rather than a
conversation, is the persistent project memory and can identify the next
reviewable work slice.

This is the only active workstream. It is documentation-only and prepares later
product documentation and Phase 6A; it does not activate either implementation
work or Phase 6A.

## Motivation

Current documents overstate the authority of one evolving use-case file, start
the presentation slice before the broader product documentation is organized,
and do not define a complete human-agent-repository continuity workflow. A new
conversation therefore cannot reliably reconstruct current scope, authorities,
and the next cut from the repository alone.

## Scope

- define the human-agent-repository workflow;
- distinguish stable policy, active context, durable strategy, current
  authorities, working inputs, and historical evidence;
- align the FMC roadmap with the documentation-first program sequence;
- establish the planned product-documentation backbone and the strategy of
  broad superficial coverage followed by incremental depth;
- prepare, but do not implement, the later Phase 6A presentation slice;
- keep repository entrypoints and references consistent.

## Out Of Scope

- creating `docs/product/fmc/` or documents for individual screens;
- changing firmware, build files, generated code, CubeMX configuration, or
  protected files;
- implementing or activating Phase 6A;
- migrating Word content or transcribing `docs/specs/fmc/use_cases.yaml`;
- reconciling legacy key, transition, or behavior contradictions;
- defining screen schemas or reproducing the legacy firmware exactly;
- redesigning the interface or adding parallel status/handoff documents;
- deleting useful legacy evidence.

## Sources To Consult

Repository policy and workflow:
- `AGENTS.md`;
- `docs/project/WORKFLOW.md`;
- `docs/workflow/README.md`;
- `docs/workflow/doc_closure.md`.

Durable strategy and product inputs:
- `docs/roadmaps/fmc_refactoring.md`;
- `docs/specs/README.md`;
- `docs/specs/fmc/use_cases.yaml`;
- `docs/specs/fmc/screen_spec_style.md`;
- `docs/specs/lcd/lcd_true_source.yaml`;
- `legacy/README.md`.

`docs/specs/lcd/lcd_true_source.yaml` remains the technical authority for the
LCD glass and mapping. `docs/specs/fmc/use_cases.yaml` is a useful evolving
inventory and design input; it is not an automatically accepted, complete, or
universal current-product contract. Legacy material is evidence, not authority
by default.

## Decisions In Force

- Conversations are disposable working sessions; durable decisions, accepted
  requirements, active scope, and next work belong in the repository.
- Documentation will first cover the product broadly and superficially, then
  deepen incrementally according to the next programming slice.
- Markdown is the default for requirements, behavior, and interface
  documentation. Structured formats are used only when they provide a concrete
  technical advantage.
- Product documentation is expected eventually under `docs/product/fmc/`, with
  a small backbone for overview, requirements, behavior, and user interface,
  but that structure is not created in this cut.
- Product requirements become authoritative only through explicit review and
  placement in the applicable current product documentation or contract.
- Legacy firmware and documents may inform design but are not permanent
  authorities and are not reproduced automatically.
- Legacy contradictions remain visible and unresolved until a later slice
  requires a decision.
- Phase 6A is later work limited to the three initial visible states and their
  presentation path. This context only prepares the documentation needed to
  select and specify that work.
- Do not commit or push without explicit human authorization.

## Deliverables

- `docs/project/WORKFLOW.md` defines continuity across conversations and
  auditable work on an identifiable branch or commit;
- this file names one active documentation cut, its boundaries, sources, and
  closure criteria;
- `docs/roadmaps/fmc_refactoring.md` records the full documentation-first
  sequence, dependencies, and bounded Phase 6A/6B work;
- `AGENTS.md` links the workflow and no longer promotes
  `docs/specs/fmc/use_cases.yaml` as a universal authority;
- the root `README.md` exposes the project workflow entrypoint.

## Verifiable Closure Criteria

This context closes when:
- a new conversation can reconstruct project authority, active state, and the
  next cut using only repository files;
- `docs/project/WORKFLOW.md` includes the complete new-conversation cycle, a
  copyable base prompt, and branch/commit audit guidance;
- this is the only active workstream and remains documentation-only;
- the roadmap states the approved sequence and its general dependencies;
- Phase 6A is explicitly future work with the approved bounded scope;
- all repository references that could present `use_cases.yaml` as a universal
  contract or authority are found and correctly limited;
- `docs/specs/lcd/lcd_true_source.yaml` remains the technical LCD authority;
- references and paths touched by this cut are valid;
- the diff contains no firmware changes, premature product-documentation split,
  or Phase 6A implementation;
- `docs/workflow/doc_closure.md` has been applied and final Git status is
  reported.
