# Context Bootstrap Industrialization

## Purpose

This is the active working document for industrializing context bootstrap in
`firmware/`.

It is intentionally dynamic.
It should preserve:
- current state
- validated reference evidence
- remaining work
- the contract pieces still needed to continue safely

For immediate execution, `working_context.md` is authoritative.
This file is the extended technical basis, backlog, and rationale for the
bootstrap industrialization track.

---

## Current State

Current stage:
- analysis

Current situation:
- the repo already has a stable split between:
  - `AGENTS.md`
  - `working_context.md`
  - `docs/contexts/`
- the repo already has workflow stages under `docs/workflow/README.md`
- the repo already has bootstrap pass areas for:
  - `docs/workflow/passes/new_feature/`
  - `docs/workflow/passes/refactor/`
- the current bootstrap prompts are theoretical; they were written during the
  LCD process but were not the source of that successful result
- the archived LCD track is the validated reference case for what a high-quality
  active context actually looked like in practice
- the goal now is to make that quality reproducible from the beginning of a new
  `new_feature` or `refactor` track

Reference archived case:
- `docs/contexts/archive/lcd_stack_closed/working_context.md`
- `docs/contexts/archive/lcd_stack_closed/lcd_redesign.md`

Target outcome:
- a repo state that can bootstrap a new `new_feature` or `refactor` line of
  work with lower ambiguity for human + agent
- enough explicit structure that a future tag can honestly claim the process is
  industrialized

---

## Scope

In scope:
- `AGENTS.md` as stable policy reference
- `docs/workflow/README.md`
- `docs/workflow/passes/README.md`
- `docs/workflow/passes/new_feature/`
- `docs/workflow/passes/refactor/`
- the structure and maintenance rules of `working_context.md`
- the structure and maintenance rules of active and archived files under
  `docs/contexts/`

Out of scope:
- changes to non-bootstrap passes
- reopening the LCD stack implementation
- broad repo documentation cleanup unrelated to bootstrap quality
- industrialization claims for passes other than `new_feature` and `refactor`

---

## Observed Validated Case

The archived LCD work proved these patterns useful:
- `working_context.md` worked best as short execution state, not as technical
  documentation
- the extended context worked best as structured technical memory
- keeping both files explicitly aligned reduced drift and confusion
- reframe capability mattered; bootstrap once was not enough
- `Current focus`, `Constraints`, and `Next Step` were the operational core
- strong rules were needed to prevent the short context from absorbing too much
  detail

The archived LCD work also showed what was not yet systematized:
- the bootstrap prompts were not the source of the successful context
- quality improved through iteration during the work
- several useful rules appeared only later in the process

---

## Known Gaps

- `AGENTS.md` is not yet a mandatory reference in the bootstrap prompts
- stage naming in bootstrap prompts is not aligned with the repository stage
  model
- create versus reframe behavior is not codified clearly enough
- the current bootstrap prompts do not fully capture the rules that made the
  archived LCD context effective
- no explicit acceptance checklist exists for a "good bootstrap"
- no canonical archived example is named as reference evidence

---

## Candidate Files

- `AGENTS.md`
- `docs/workflow/README.md`
- `docs/workflow/passes/README.md`
- `docs/workflow/passes/new_feature/README.md`
- `docs/workflow/passes/refactor/README.md`
- `docs/workflow/passes/new_feature/prompts/bootstrap_context.md`
- `docs/workflow/passes/refactor/prompts/bootstrap_context.md`
- `working_context.md`
- `docs/contexts/`

---

## Risks

- overfitting the LCD case and mistaking local success for universal shape
- turning bootstrap into a planning framework instead of an execution enabler
- duplicating stable policy from `AGENTS.md` into bootstrap prompts
- codifying too little and preserving the same ambiguity under new wording
- claiming industrialization before validating one `new_feature` and one
  `refactor` from the new bootstrap

---

## Minimum Plan

1. extract the bootstrap contract from the archived LCD case
2. define acceptance criteria for a high-quality initial `working_context.md`
   and extended context
3. align stage/pass semantics with `docs/workflow/README.md`
4. decide create-versus-reframe behavior and archive rules
5. update prompts and helper docs only after the contract is explicit
6. validate the result on one `new_feature` and one `refactor` bootstrap before
   any "industrialized" tag claim
