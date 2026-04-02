# Current Milestone

## Name
M1 - Workflow definition

## Goal
Define a clear repository workflow for discussing work with ChatGPT, consolidating decisions in the repo, and generating executable prompts for Codex.

## Scope
- Define the role of `docs/roadmap.md`
- Define the role of `docs/current_milestone.md`
- Define the role of `docs/milestone_design.md`
- Define the role of `docs/agent_prompt.md`
- Define the role of `AGENTS.md`
- Establish the workflow from discussion to execution

## Out of scope
- C coding style
- Formatting rules
- Static analysis
- Firmware architecture decisions
- CubeMX
- Firmware implementation

## Deliverables
- updated `docs/roadmap.md`
- updated `docs/current_milestone.md`
- new `docs/milestone_design.md`
- new `docs/agent_prompt.md`
- new `AGENTS.md`

## Source of truth rule
`docs/current_milestone.md` is the single source of truth for the active milestone.

## Workflow
1. Human and ChatGPT discuss alternatives
2. Relevant reasoning is summarized in `docs/milestone_design.md`
3. Final milestone scope is consolidated in `docs/current_milestone.md`
4. A clean execution prompt is written in `docs/agent_prompt.md`
5. Codex executes from `docs/agent_prompt.md`
6. Milestone is closed only when deliverables and exit criteria are satisfied

## Exit criteria
- The role of each workflow file is explicit
- The active milestone source of truth is defined
- The path from discussion to Codex execution is unambiguous
- The repo can be used as continuity for future milestones

## Notes
- Keep the workflow lightweight
- Avoid duplicate sources of truth
- Store decisions in the repo, not only in chat