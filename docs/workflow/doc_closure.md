# Doc Closure

Use before commit when documentation or active context changed.

Prompt:

```text
Aplicar doc closure pre-commit.
```

Meaning: apply this file, clean documentation drift, then report added,
deleted, compressed, and still-questionable docs.

## Rules

- Run before commit. Post-commit use is audit only.
- Do not keep documentation as backup history; Git does that.
- Documentation must become shorter, clearer, or more directly useful.
- Delete or compress old plans, option comparisons, slice narration,
  duplicated validation history, and rationale already visible in code, tests,
  specs, or Git history.
- Keep one source of truth per topic: current product documentation in
  `docs/product/`, structured technical specifications and inventories in
  `docs/specs/`, active scope in `WORKING_CONTEXT.md`, durable strategy in
  roadmaps, and implemented module contracts in headers.
- Do not duplicate product requirements in contexts, roadmaps, READMEs, or
  module documentation.

## Active Milestone

Required:
- `WORKING_CONTEXT.md` names exactly one active workstream.
- `WORKING_CONTEXT.md` names the product behavior authority when product
  behavior is in scope.
- No other `.md` claims to define the current milestone or active workstream.
- A new agent can find the applicable roadmap and specification from
  `WORKING_CONTEXT.md` when they are relevant.
- `WORKING_CONTEXT.md` and the roadmap do not describe the same state.

## Ownership

- Stable agent policy: `AGENTS.md`
- Active milestone, scope, sequencing, and temporary boundaries:
  `WORKING_CONTEXT.md`
- Durable refactor strategy, phases, dependencies, decision gates, and exit
  criteria: `docs/roadmaps/`
- Folder ownership: local `README.md` files
- Current product documentation: `docs/product/`
- Structured technical specifications and normalized inventories: `docs/specs/`
- Implemented module contracts: public headers
- Historical legacy evidence: `legacy/`

Roadmaps should update only when a phase, dependency, decision gate, exit
criterion, or broad architectural decision changes.

Closed contexts and abandoned design notes are deleted by default. Keep one only
if it contains concise evidence needed by the current product specification or
roadmap.

## Final Scrape

Run:

```powershell
rg --files -g "*.md"
rg -n "<old_context>|<deleted_file>|obsolete_route|TODO" -g "*.md"
```

Adapt the second command to the files and terms touched by the slice.

Report:
- docs added, deleted, or compressed
- whether stale references remain
- whether build verification was skipped because the change was documentation-only
