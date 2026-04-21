# WORKING_CONTEXT.md

## Current Work

Stage:
- analysis

Active pass:
- new_feature

Operational context:

Area:
- `AGENTS.md`
- `docs/workflow/`
- `docs/workflow/passes/`
- `docs/contexts/`

Core structure:
- `AGENTS.md` defines stable agent policy and the documentation split
- `working_context.md` is the active execution state for the current workstream
- `docs/contexts/` holds extended active context and archived closed contexts
- `docs/workflow/README.md` defines the repository stage model
- `docs/workflow/passes/new_feature/` and `refactor/` hold the bootstrap tools
  for starting new work

Current focus:
- define a reproducible bootstrap process for high-quality `working_context.md`
  and extended context files
- use the archived LCD track as the validated reference case
- limit the industrialization effort to `new_feature` and `refactor`

Constraints:
- do not reopen the closed LCD stack context except as reference evidence
- do not change unrelated passes outside `new_feature` and `refactor`
- treat `AGENTS.md` as stable unless a true policy gap is proven
- do not assume undocumented behavior; promote only validated patterns into the
  bootstrap design
- optimize for human + agent repeatability, not one-off prompt cleverness

## Next Step

- extract the bootstrap contract from the archived LCD case
- define acceptance criteria for a high-quality initial `working_context.md` and
  extended context
- only then decide which templates, prompts, or helper docs need changes

## References

- `AGENTS.md`
- `docs/contexts/context_bootstrap_industrialization.md`
- `docs/contexts/archive/lcd_stack_closed/working_context.md`
- `docs/contexts/archive/lcd_stack_closed/lcd_redesign.md`
- `docs/workflow/README.md`
- `docs/workflow/passes/README.md`
- `docs/workflow/passes/new_feature/README.md`
- `docs/workflow/passes/refactor/README.md`
- `docs/workflow/passes/new_feature/prompts/bootstrap_context.md`
- `docs/workflow/passes/refactor/prompts/bootstrap_context.md`

## Invocation Rule

Every time this file is used:
- confirm stage, active pass, current focus, constraints, and next step
- for immediate execution, `Constraints` and `Next Step` are authoritative
- use the archived LCD context as reference evidence, not as the active track
- if the real work drifted, update this file and its referenced `docs/contexts/`
  file before continuing
- if the request falls outside this context, reframe instead of forcing it into
  the current track
- load only the references needed for the current step
- if a pass becomes explicit, treat that pass as dominant for the task
- after any material progress, refresh current focus, constraints, and next
  step

## Maintenance Rule

Keep this file updated when any of these change:
- stage
- active pass
- operational focus
- constraints
- next step
- referenced active context file

Keep the referenced file under `docs/contexts/` updated when any of these
change:
- extended technical rationale
- detailed scope
- validated or pending state
- remaining work
- active technical decisions
- contract details that no longer fit in this file

Update both together when:
- the short execution state and the extended context would otherwise diverge
- this file starts pointing to a different active context file
- the active line of work changes materially

Refresh this file immediately after:
- completing the current step
- a verification result changes the next step or constraints
- a blocker or direction change redefines the active line of work

## Strong Rule

This is execution state, not documentation.
No history.
No long explanation.
Detailed rationale stays in `docs/contexts/context_bootstrap_industrialization.md`.
Keep `working_context.md` and its referenced `docs/contexts/` file aligned.
Repeat here only the most important active facts needed to execute the current
step safely.
Do not let this file gradually absorb extended design detail that belongs in
`docs/contexts/context_bootstrap_industrialization.md`.
