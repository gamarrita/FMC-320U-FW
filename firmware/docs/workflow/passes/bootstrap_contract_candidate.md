# Bootstrap Contract Candidate

## Purpose

This file is a candidate contract for bootstrapping a new active context pair:
- `WORKING_CONTEXT.md`
- `docs/contexts/<slug>.md`

It is not yet stable policy.
It is derived from:
- `AGENTS.md`
- the archived LCD case under `docs/contexts/archive/lcd_stack_closed/`
- the checklist application already recorded in the bootstrap
  industrialization track

Use it as a candidate contract while industrializing bootstrap for:
- `new_feature`
- `refactor`

---

## Contract Scope

This contract is for a fresh active bootstrap.

It does not require an archived reference pair to look identical to a fresh
bootstrap, because archived closure contexts can have weaker immediate
`Next Step` semantics than a first-turn context.

This contract should therefore be read as:
- what a good fresh bootstrap must provide
- not what every archived successful context must look like in every detail

---

## Required Inputs

Before bootstrapping, enough information must exist to answer at least these:

- what workstream is being started:
  - `new_feature`
  - `refactor`
- what real area, module, folder, or flow is in scope
- what behavior must be introduced or preserved
- what boundaries are explicitly out of scope
- what next executable step should happen immediately after bootstrap

If one or more of those are missing, the bootstrap should not be finalized yet.

---

## Output Pair

A valid bootstrap must produce both:
- `WORKING_CONTEXT.md`
- `docs/contexts/<slug>.md`

The short file is the execution state.
The extended file is the technical basis and backlog for that workstream.

---

## `WORKING_CONTEXT.md` Contract

`WORKING_CONTEXT.md` must:

- behave as short execution state, not long documentation
- identify the current workstream clearly enough to route the next turn
- contain a real current focus
- contain constraints that actively reduce scope creep
- contain a concrete next executable step
- identify the relevant area with enough precision
- point to the active extended context file
- include explicit rules for:
  - invocation
  - maintenance
  - alignment with the extended context
- avoid duplicating stable policy that belongs in `AGENTS.md`

The effective operational core must be:
- `Current focus`
- `Constraints`
- `Next Step`

The short file should remain short even when the extended context grows.

### Required Shape

At minimum, `WORKING_CONTEXT.md` should contain:

- `Current Work`
- `Stage`
- `Active pass`
- `Operational context`
  - `Area`
  - `Core structure`
  - `Current focus`
  - `Constraints`
- `Next Step`
- `References`
- `Invocation Rule`
- `Maintenance Rule`
- `Strong Rule`

---

## `docs/contexts/<slug>.md` Contract

The extended context must:

- preserve the technical state of the active workstream
- make the current situation understandable without relying on thread memory
- keep rationale, risks, and remaining work out of the short file
- keep the split of responsibility with `WORKING_CONTEXT.md` explicit
- preserve enough structure to support reframing later

### Required Content Coverage

The extended context must cover, at minimum:

- purpose of the workstream
- current state
- real scope
- validated or trusted reference evidence
- known gaps or risks
- remaining work
- out-of-scope boundaries

### Recommended Shape

The exact section names may vary, but a strong candidate shape is:

- `Purpose`
- `Current State`
- `Scope`
- `Observed Validated Case` or equivalent evidence section
- `Known Gaps`
- `Risks`
- `Minimum Plan`

---

## Alignment Contract

The pair must make these rules explicit:

- immediate execution authority lives in `WORKING_CONTEXT.md`
- extended rationale and backlog live in `docs/contexts/<slug>.md`
- both files must be updated together when they would otherwise diverge
- the short file must not absorb extended design detail

---

## Fresh Bootstrap Expectations

For a fresh bootstrap, these are required:

- the next step must be immediately actionable
- the context must be usable by human or agent on the next turn
- the output must not depend on implicit conversation memory
- the workstream boundaries must already be narrow enough to avoid accidental
  spread

These expectations are stronger than what may be visible in an archived closure
reference.

---

## Explicit Non-Requirements

This contract does not yet require:

- `Active pass` to map directly to a pass folder name
- `new_feature` and `refactor` to have separate prompt systems forever
- one bootstrap to remain sufficient without later reframing

Those remain open questions until further validation.

---

## Acceptance Rule

A bootstrap candidate is acceptable only if:

- it satisfies the `Must-Have` items from
  `docs/workflow/passes/bootstrap_quality_checklist_candidate.md`
- it satisfies this contract's required inputs and output pair rules
- it does not violate the documentation split defined by `AGENTS.md`
