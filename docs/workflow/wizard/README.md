# Workflow Wizard

## Purpose

This folder is the human-facing entrypoint for starting or reframing a new
workstream context.

Use it when the goal is:
- create a new `WORKING_CONTEXT.md` pair
- reframe an existing active context pair
- start a new `refactor` or `new_feature` track without forcing it into the
  current active context

This folder is the interface.
`docs/workflow/passes/` remains the backend methodology area.

---

## Trigger

The simplest trigger is the keyword:
- `wizard`

Examples:
- `wizard para iniciar un refactor sobre src/port/fm_port_time.*`
- `wizard para reencuadrar este track`
- `wizard para crear un nuevo working context`

The wizard should also be used when:
- the request clearly asks to start a new workstream
- the request clearly asks to reframe an existing one
- the request falls outside the active `WORKING_CONTEXT.md`

---

## Current Supported Scope

Right now, this wizard layer supports:
- `refactor` MVP

It does not yet claim full support for:
- `new_feature`

The current backend for the `refactor` wizard lives under:
- `docs/workflow/passes/refactor/prompts/bootstrap_context.md`

---

## Process Now

Today, the intended flow is:

1. the user asks for a new track or says `wizard`
2. the agent checks whether the request belongs to the active
   `WORKING_CONTEXT.md`
3. if it does not belong there, the agent leaves the current track and enters
   the wizard flow
4. the agent chooses the wizard type
   - currently: `refactor`
5. the wizard infers what it can from the request and the repo
6. if blocking information is still missing, the wizard asks only short,
   concrete questions
7. as soon as the stop rule is satisfied, the wizard generates:
   - `WORKING_CONTEXT.md`
   - `docs/contexts/<slug>.md`

The wizard should not force a large interview.
It should ask only for the minimum information needed to bootstrap safely.

---

## User Responsibility

The user should provide at least a rough request, for example:
- the target module or folder
- whether this is a new refactor track or a reframe
- what behavior must stay correct if that is already known

The user does not need to provide:
- a full plan
- a full file inventory
- a final validation strategy

---

## Agent Responsibility

The agent should:
- route to the wizard when the request opens a new track
- avoid forcing new work into the current active context
- infer safe local context before asking
- ask only for blocking missing input
- stop asking as soon as a good first context pair can be created

---

## Current Output

The wizard should currently produce:
- `WORKING_CONTEXT.md`
- `docs/contexts/<slug>.md`

The short file should be execution state.
The extended file should be rationale, scope, risks, and backlog.

---

## Future Improvements

To make this flow more intuitive, a later version may add:
- `wizard/refactor.md`
- `wizard/new_feature.md`
- `wizard/templates/working_context.template.md`
- `wizard/templates/extended_context.template.md`
- a simple manual fallback:
  - copy templates into place
  - fill the minimum fields
  - then tell the agent to continue from that new context

The goal is that a human can either:
- invoke the wizard interactively
- or bootstrap manually from templates and then resume with the agent

---

## Practical Rule

If the request continues the active workstream:
- stay in `WORKING_CONTEXT.md`

If the request starts or reframes another workstream:
- use the wizard
