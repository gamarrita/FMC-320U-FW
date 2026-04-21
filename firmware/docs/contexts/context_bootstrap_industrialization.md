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
- a candidate bootstrap quality checklist now exists to evaluate future
  bootstrap outputs without yet promoting them to stable truth
- a candidate bootstrap contract now exists to capture what survived the
  archived LCD evaluation and what should apply only to fresh bootstraps
- a candidate minimum input and stop rule now exists for a first `refactor`
  wizard MVP
- the theoretical `refactor` bootstrap prompt has now been reduced to follow
  that wizard MVP candidate
- sandbox trials have now exercised multiple plausible `refactor` requests
- those trials justified explicit question mode and explicit
  create-versus-reframe branching in the prompt
- a stable wizard gate now exists in `AGENTS.md`
- a human-facing wizard entrypoint now exists under `docs/workflow/wizard/`
- the current narrowing is to reach a refactor wizard MVP first, then decide if
  the same approach should later expand to `new_feature`

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
- `docs/workflow/wizard/`
- `docs/workflow/passes/README.md`
- `docs/workflow/passes/refactor/`
- the structure and maintenance rules of `working_context.md`
- the structure and maintenance rules of active and archived files under
  `docs/contexts/`

Out of scope:
- changes to non-bootstrap passes
- reopening the LCD stack implementation
- broad repo documentation cleanup unrelated to bootstrap quality
- industrialization claims beyond a first `refactor` wizard MVP

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

- the new wizard entrypoint has not yet been validated on one live refactor
  case
- archive behavior for replaced contexts is not yet explicit
- `new_feature` still has no equivalent wizard entry layer
- some older workflow and pass docs still reflect the pre-wizard mental model

---

## Checklist Application To Archived LCD Case

Reference evaluated:
- `docs/contexts/archive/lcd_stack_closed/working_context.md`
- `docs/contexts/archive/lcd_stack_closed/lcd_redesign.md`

Observed result:
- the archived LCD pair remains a strong validated reference case
- most `Must-Have` and `Should-Have` items from the candidate checklist are
  satisfied by the archived pair
- one important tension appeared:
  - the archived pair is a closure-stage context, not a fresh bootstrap
  - because of that, "next executable step" is weaker than what a new bootstrap
    should ideally provide

Validated takeaways from the checklist application:
- the split between short execution state and extended context is strong
- explicit alignment and maintenance rules are essential
- `Current focus`, `Constraints`, and `Next Step` are the operational core
- the short context can remain short while the extended context grows
- the extended context benefits from clearly separated sections for:
  - current state
  - completed work
  - remaining work
  - out-of-scope boundaries

Open tension revealed by the checklist application:
- the candidate checklist should not assume that every archived reference pair
  behaves like a first-turn bootstrap
- "next step must be immediately executable" looks correct for a bootstrap, but
  it is too strict when evaluating a closure-stage archived reference case

Experimental items not promoted by this application:
- `Active pass` does not need to map directly to a pass folder name
- the archived case does not support the assumption that one strong bootstrap is
  enough without later reframing
- the archived case does not prove yet whether `new_feature` and `refactor`
  require separate prompts

Implication:
- derive the bootstrap contract from the items that clearly survived this
  application
- keep the closure-stage mismatch as a reminder that bootstrap acceptance and
  archived-reference evaluation are related but not identical

---

## Bootstrap Contract Candidate

Candidate file:
- `docs/workflow/passes/bootstrap_contract_candidate.md`

What this candidate now captures:
- the minimum required inputs before a bootstrap can be finalized
- the required output pair:
  - `working_context.md`
  - `docs/contexts/<slug>.md`
- the operational obligations of the short context
- the minimum content coverage of the extended context
- the alignment contract between the two files
- the stronger expectations that apply only to fresh bootstraps

What it intentionally does not freeze yet:
- direct mapping between `Active pass` and pass folder naming
- whether `new_feature` and `refactor` must keep separate prompt systems
- whether a strong bootstrap should normally avoid later reframing

Implication:
- the next useful reduction in ambiguity is no longer another abstract
  checklist step
- the next useful step is to turn this contract into a minimum input and stop
  rule for a wizard MVP, starting with `refactor`

---

## Refactor Wizard MVP Candidate

Candidate file:
- `docs/workflow/passes/refactor/refactor_wizard_mvp_candidate.md`

What this candidate now captures:
- the minimum information needed before a `refactor` wizard should stop asking
  questions
- the difference between blocking and non-blocking inputs
- the stop rule that makes the wizard safe to bootstrap from
- the keep-asking conditions that prevent premature output
- the MVP question style needed to avoid a heavy interview

What it intentionally does not freeze yet:
- final prompt wording
- create-versus-reframe behavior
- archive behavior for replaced active contexts
- any `new_feature` wizard behavior

Implication:
- the next useful step is no longer to define another abstract candidate layer
- the next useful step is to compare the current theoretical `refactor`
  bootstrap prompt against the wizard MVP candidate and reduce it to what the
  wizard actually needs

---

## Refactor Prompt Reduction

Updated file:
- `docs/workflow/passes/refactor/prompts/bootstrap_context.md`

What changed in this step:
- the prompt now behaves as a wizard MVP instead of a generic bootstrap request
- it now requires reading `AGENTS.md` before bootstrapping
- it now asks only for the minimum missing information:
  - refactor target
  - preserved behavior
  - structural driver
  - scope boundary
  - immediate next analysis step
- it now contains an explicit stop condition
- it now supports a question-only mode before bootstrap mode
- it now treats create-versus-reframe as an explicit branch
- it now starts fresh bootstraps in `Stage = analysis` and
  `Active pass = refactor`
- it now requires `Invocation Rule` in the short context output

What remains unresolved:
- whether one real wizard trial will expose any missing bootstrap input
- whether archive behavior for replaced contexts needs to be explicit this early

Implication:
- a sandbox trial set is now useful before a live trial
- the prompt should only be changed again if a real trial exposes another gap

---

## Sandbox Trials

Sandbox files:
- `docs/workflow/passes/refactor/sandbox/README.md`
- `docs/workflow/passes/refactor/sandbox/trial_matrix.md`
- `docs/workflow/passes/refactor/sandbox/case_01_port_time_clear.md`
- `docs/workflow/passes/refactor/sandbox/case_02_app_entry_partial.md`
- `docs/workflow/passes/refactor/sandbox/case_03_debug_panic_reframe.md`

What the sandbox showed:
- a clear fresh refactor case should bootstrap immediately
- a partial fresh refactor case should stay in question mode until blockers are
  resolved
- an explicit reframe request should not create a fresh context pair blindly

Adjustments justified by the sandbox:
- the prompt now supports question mode before bootstrap mode
- create-versus-reframe is now treated as an explicit wizard branch

What the sandbox did not justify:
- broader changes outside `refactor`
- freezing archive behavior for replaced contexts
- changing `new_feature` yet

Implication:
- the next useful step is a real refactor bootstrap trial using the reduced
  prompt
- that live trial should now judge the adjusted prompt, not the pre-sandbox one

---

## Wizard Entry Layer

Updated files:
- `AGENTS.md`
- `docs/workflow/README.md`
- `docs/workflow/passes/README.md`
- `docs/workflow/wizard/README.md`

What changed in this step:
- `AGENTS.md` now has a stable wizard gate
- the keyword `wizard` is now an explicit trigger for leaving the active track
  and starting bootstrap flow
- `docs/workflow/wizard/README.md` now acts as the human-facing entrypoint
- `docs/workflow/README.md` now points to the wizard when a new workstream is
  being started or reframed
- `docs/workflow/passes/README.md` now makes it explicit that `passes/` is
  backend methodology, not the human-facing wizard entry

What this entry layer explains:
- when to stay in `WORKING_CONTEXT.md`
- when to use the wizard
- how the current interactive process works now
- how templates or manual fallback could improve that later

Implication:
- the next live wizard trial should now be invoked through
  `docs/workflow/wizard/README.md`
- that trial should validate both routing and bootstrap behavior together

---

## Candidate Files

- `AGENTS.md`
- `docs/workflow/README.md`
- `docs/workflow/wizard/README.md`
- `docs/workflow/passes/README.md`
- `docs/workflow/passes/bootstrap_quality_checklist_candidate.md`
- `docs/workflow/passes/bootstrap_contract_candidate.md`
- `docs/workflow/passes/refactor/README.md`
- `docs/workflow/passes/refactor/refactor_wizard_mvp_candidate.md`
- `docs/workflow/passes/refactor/sandbox/trial_matrix.md`
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

1. run one live `refactor` wizard trial through the new wizard entrypoint
2. judge whether routing, question mode, and create-versus-reframe branching
   are sufficient in practice
3. adjust the wizard MVP only if the live trial exposes a real missing input,
   stop rule, or routing gap
4. only after that, decide whether `new_feature` should follow the same path
