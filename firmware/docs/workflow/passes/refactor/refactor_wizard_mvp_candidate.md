# Refactor Wizard MVP Candidate

## Purpose

This file defines a candidate minimum input and stop rule for a first
interactive bootstrap wizard limited to `refactor`.

It is not yet stable policy.
It is derived from:
- `AGENTS.md`
- `docs/workflow/passes/bootstrap_quality_checklist_candidate.md`
- `docs/workflow/passes/bootstrap_contract_candidate.md`
- the archived LCD reference case under
  `docs/contexts/archive/lcd_stack_closed/`

Use it to reduce ambiguity before changing any `refactor` bootstrap prompt.

---

## Scope

This candidate is intentionally narrow.

It covers:
- a wizard MVP for `refactor`
- the minimum information needed to stop asking questions
- the minimum information needed to generate a usable active context pair
- the minimum branching needed between fresh bootstrap and reframe

It does not yet freeze:
- the final prompt wording
- archive behavior for replaced contexts
- any `new_feature` behavior

---

## Wizard Goal

The wizard should:
- identify that the requested work is a real `refactor` track
- ask only for information that is still missing and not safely inferable from
  the user request or local repo context
- stop as soon as the bootstrap contract can be satisfied for a fresh
  refactor context
- produce a usable output pair:
  - `working_context.md`
  - `docs/contexts/<slug>.md`

The wizard should not:
- force a large interview up front
- require a full architecture description before bootstrapping
- ask for details that can be discovered safely during the next analysis step

---

## Wizard Modes

The wizard MVP needs two response modes:

1. `Question mode`
   - used when the stop rule is not yet satisfied
   - the wizard should return only:
     - what is already known
     - what blocking input is still missing
     - the smallest useful next questions

2. `Bootstrap mode`
   - used only after the stop rule is satisfied
   - the wizard should return the active context pair

The wizard should not force bootstrap-mode output when blocking input is still
missing.

---

## Create Versus Reframe

The wizard MVP must detect whether the task is:
- `create`
  - start a fresh refactor track
- `reframe`
  - update an existing active context pair without losing track continuity

If the user request explicitly says the work should be reframed, or if an
existing active refactor context is already in play, the wizard should treat
that as a blocking branch decision before bootstrapping.

For a `reframe`, the wizard must know:
- which active context pair is being reframed
- what the new narrowed focus is
- what remains out of scope after the reframe
- what next analysis step replaces the old one

---

## Minimum Input Set

The wizard has enough information to bootstrap a refactor context only when
all of these are known with practical clarity:

1. `Refactor target`
   - the real module, folder, flow, or workstream being refactored

2. `Preserved behavior`
   - what current behavior, path, or external expectation must remain correct

3. `Structural driver`
   - the real structural problem that justifies a refactor track
   - examples:
     - mixed responsibilities
     - ownership drift
     - duplicated logic
     - poor local readability
     - difficult safe extension

4. `Scope boundary`
   - what is definitely in scope
   - what is definitely out of scope

5. `Immediate next step anchor`
   - the first concrete analysis action after bootstrap
   - examples:
     - inspect a specific folder
     - trace a request path
     - compare two modules
     - map ownership across named files

6. `Mode clarity`
   - whether the wizard is creating a fresh track or reframing an existing one
   - if it is a reframe, which active pair is being updated

These inputs may come from:
- the user's prompt
- short follow-up questions
- safe repo inspection

The wizard should prefer deriving what it can from local context before asking
for more.

---

## Non-Blocking Inputs

These can help, but the wizard should not block on them unless the specific
case truly requires them:

- exact candidate file list
- full validation strategy
- exact slug preference
- complete side-effect inventory
- full state transition map
- full rationale history

If those are still missing after the minimum input set is satisfied, they can
become part of the next analysis step instead of a bootstrap blocker.

---

## Stop Rule

The wizard must stop asking bootstrap questions and generate the context pair
when all of the following are true:

- the task can be treated confidently as a `refactor` track
- the refactor target is named precisely enough to route the next turn
- the preserved behavior is explicit enough to avoid accidental redesign
- the structural driver is explicit enough to justify the refactor
- the scope has at least one clear out-of-scope boundary
- the immediate next step is concrete and executable
- the output pair can be written without relying on hidden conversation memory
- create-versus-reframe mode is clear enough to avoid duplicate or drifting
  context files

If one of those is still missing, the wizard should keep asking.

---

## Keep-Asking Conditions

The wizard should continue asking when:

- it is still unclear whether the request is really a `refactor`
- the target area is still too broad or multi-track
- preserved behavior is still vague
- the structural problem is still vague or purely speculative
- no meaningful out-of-scope boundary exists yet
- the next step would still read as a generic intention instead of a real
  action
- the request signals a reframe but the active pair being reframed is still
  unclear

The wizard should ask only for the smallest missing piece that would unblock
the stop rule.

---

## Question Style

The wizard MVP should:
- ask short, concrete questions
- prefer one missing decision at a time
- avoid asking for information already visible in the repo
- avoid asking for full design proposals
- stop as soon as the stop rule is satisfied

The wizard MVP should not:
- collect a full implementation plan
- ask for final validation details too early
- ask for broad future roadmap decisions
- collapse question mode and bootstrap mode into one forced output

---

## Output Expectations

Once the stop rule is satisfied, the wizard output should be able to produce:

- a short `working_context.md` with:
  - real current focus
  - real constraints
  - real next step
- an extended `docs/contexts/<slug>.md` with:
  - purpose
  - current state
  - scope
  - known risks or gaps
  - minimum plan

The wizard does not need to produce a final perfect context.
It only needs to produce a strong first active pair that is safe to continue
from.

---

## MVP Implication

If this candidate is accepted, the next useful work is not another abstract
contract step.

The next useful work is:
- compare the current theoretical `refactor` bootstrap prompt against this
  wizard MVP candidate
- keep only the prompt content that is needed to reach the minimum input set
  and stop rule
- add explicit question-mode output and create-versus-reframe branching
- remove or defer prompt requirements that exceed MVP needs
