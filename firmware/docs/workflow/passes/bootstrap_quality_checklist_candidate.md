# Bootstrap Quality Checklist Candidate

## Purpose

This file is a candidate acceptance checklist for evaluating the quality of a
newly bootstrapped:
- `WORKING_CONTEXT.md`
- `docs/contexts/<slug>.md`

It is not yet stable policy.
It is not yet the final bootstrap contract.

Use it as a practical evaluation tool while industrializing context bootstrap
for:
- `new_feature`
- `refactor`

---

## Confidence Level

This checklist mixes different confidence levels:

- validated criteria
  - based mainly on:
    - `AGENTS.md`
    - the archived LCD case under `docs/contexts/archive/lcd_stack_closed/`
- should-have criteria
  - useful in practice, but not all equally validated
- experimental criteria
  - hypotheses to test, not current truth

Do not promote this file to stable truth without additional validation.

---

## Must-Have

If any item here is `No`, the bootstrap should be treated as not ready.

### Entry And Focus

- [ ] `WORKING_CONTEXT.md` defines the real active work, not general backlog
- [ ] `WORKING_CONTEXT.md` defines a concrete next executable step
- [ ] the bootstrap makes the active scope explicit
- [ ] the bootstrap makes lateral boundaries explicit

### Operability

- [ ] a human or agent can continue the next turn without large hidden
      assumptions
- [ ] `Constraints` reduce scope creep in a real, actionable way
- [ ] `Current focus` describes current execution state, not history
- [ ] `Next Step` is an action, not only an intention

### Short Versus Extended Ownership

- [ ] `WORKING_CONTEXT.md` behaves as short execution state
- [ ] `docs/contexts/<slug>.md` behaves as extended technical context
- [ ] the split of responsibility between those two files is explicit
- [ ] immediate execution authority is explicit
- [ ] alignment and maintenance expectations are explicit

### Policy And Ownership

- [ ] stable policy from `AGENTS.md` is not duplicated unnecessarily
- [ ] ownership of the workstream is clear enough to avoid accidental spread
- [ ] affected areas or modules are identified with enough precision
- [ ] the bootstrap does not invent architecture without justification

### Continuity

- [ ] the context can be reframed later without destroying the track
- [ ] the bootstrap result does not depend on remembering the whole thread
- [ ] the short context can remain short even if the extended context grows

---

## Should-Have

Missing several items here means the bootstrap is usable but weak.

- [ ] `References` are minimal and useful
- [ ] the extended context distinguishes:
  - current state
  - completed work
  - remaining work
  - out-of-scope boundaries
- [ ] update and maintenance rules are explicit
- [ ] the short context contains a strong rule preventing documentation bloat
- [ ] validated versus pending state is clear enough for the next turn
- [ ] the extended context keeps rationale without becoming a raw conversation
- [ ] out-of-scope boundaries are explicit in at least one of the two files
- [ ] the wording is usable by both human and agent

---

## Experimental

These items are worth testing, but should not yet be treated as hard truth.

- [ ] `Stage` always follows the exact stage vocabulary from
      `docs/workflow/README.md`
- [ ] `Active pass` should always map directly to a pass folder name
- [ ] `new_feature` and `refactor` need separate bootstrap prompts
- [ ] one shared extended-context shape is sufficient for both track types
- [ ] a first bootstrap can often be strong enough without an early reframe

---

## Warning Signs

If any of these appear, the bootstrap likely needs rework before it becomes the
active context of a real track.

- [ ] the short context reads like general documentation
- [ ] the extended context repeats too much of the short context
- [ ] it is unclear when to update one file, the other, or both
- [ ] the next step still requires major interpretation before acting
- [ ] unrelated topics are bundled into the same track
- [ ] the context depends on implicit thread memory to stay understandable

---

## Verdict Rule

Use this simple evaluation:

- approved
  - all `Must-Have` items are `Yes`
  - most `Should-Have` items are `Yes`
- approved with risk
  - all `Must-Have` items are `Yes`
  - several `Should-Have` items are `No`
- not approved
  - one or more `Must-Have` items are `No`

---

## Evaluation Template

```md
Bootstrap evaluation:
- Candidate:
  - `WORKING_CONTEXT.md`
  - `docs/contexts/<slug>.md`

Must-Have:
- [ ] active work is real and current
- [ ] next step is executable
- [ ] constraints reduce scope creep
- [ ] short vs extended ownership is clear
- [ ] alignment rule exists
- [ ] no unnecessary stable policy duplication
- [ ] ownership is clear
- [ ] the next turn can continue without hidden assumptions

Should-Have:
- [ ] references are minimal and useful
- [ ] validated vs pending state is clear
- [ ] out-of-scope is explicit
- [ ] maintenance/update rules are explicit
- [ ] short context stays short
- [ ] extended context preserves rationale cleanly

Experimental:
- [ ] stage/pass semantics feel consistent
- [ ] bootstrap shape seems reusable for this track type

Warning Signs:
- [ ] short context reads like documentation
- [ ] extended context repeats too much of the short context
- [ ] update responsibility is unclear
- [ ] next step still needs major interpretation
- [ ] lateral topics leaked into the track
- [ ] context depends on implicit thread memory

Verdict:
- approved
- approved with risk
- not approved

Notes:
- ...
```
