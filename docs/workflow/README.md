# Workflow Model

## Purpose

This file keeps the repo's default stage vocabulary small and stable.

Use `WORKING_CONTEXT.md` first when an active workstream exists. Follow the
applicable roadmap and specifications referenced there. Use this file only to
interpret stage names consistently.

## Default Stages

Routine work normally moves through:

1. analysis
2. plan
3. implementation
4. validation
5. closure

Small tasks may combine stages.

## Stage Meanings

- `analysis`: understand the current code, ownership, and risks.
- `plan`: define the smallest correct change.
- `implementation`: make the requested change.
- `validation`: verify the result and report what was or was not checked.
- `closure`: finish the bounded work, review its evidence, and hand it off.
  Apply `docs/workflow/doc_closure.md` only when the active workstream is
  formally closed, deferred, or replaced.

## Context Rule

`docs/project/WORKFLOW.md` owns working-context terminology and lifecycle.
`WORKING_CONTEXT.md` records one active workstream and exactly one active
slice. Update that same context as the workstream advances between slices;
reframe it only when the workstream changes.

## Related Docs

- `AGENTS.md`: stable agent policy
- `WORKING_CONTEXT.md`: active workstream and active slice, when one exists
- `docs/project/WORKFLOW.md`: working-context terminology and lifecycle
- `docs/roadmaps/`: durable refactor strategy
- `docs/product/`: current reviewed product documentation
- `docs/specs/`: structured technical specifications, inventories, and
  engineering inputs
- `docs/workflow/fmc_legacy_coverage.md`: mutable FMC legacy review coverage
- `STYLE.md`: naming and code structure
- `CONTRIBUTING.md`: repo hygiene
- `docs/workflow/doc_closure.md`: documentation closure checklist
