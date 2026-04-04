# 02_workflow.md

## Purpose

This document describes the working workflow for `firmware/`
when collaborating with AI assistants and code agents.

This is the full workflow document.
It is more detailed than `01_quickstart.md`.

Use this file to understand roles, sequencing, task framing,
handoff boundaries, and continuity management.

---

## Workflow goals

The workflow is designed to:

- keep tasks small and verifiable
- reduce accidental architecture drift
- improve consistency of generated code
- separate thinking from execution
- make progress easy to review
- preserve continuity across sessions

---

## Main workflow roles

### ChatGPT or planning assistant

Use for:

- clarifying problem framing
- comparing options
- refining naming or module boundaries
- reviewing whether a change belongs in `app/`, `services/`, or another area
- drafting prompts for execution agents
- reviewing documentation structure
- turning a large task into smaller tasks

This role should help think before editing.

### Codex or execution-oriented agent

Use for:

- inspecting target files
- implementing small scoped changes
- generating code following repository style
- adjusting file structure
- performing narrow refactors
- applying a clearly described change set

This role should execute against a concrete task.

---

## Standard task cycle

### Step 1: define the task

Describe the task in a small and testable way.

The task should identify:

- target files or module
- expected owning folder
- boundaries of what should not change
- reference docs
- whether the task is implementation, cleanup, naming correction, or split

### Step 2: identify repository references

Before editing, identify the governing references:

- `STYLE.md` for naming and structure
- `AGENTS.md` for agent constraints
- `style-examples/` for the nearest valid pattern
- `CHANGELOG.md` if continuity from earlier work matters
- `05_working_context.md` if the session depends on current live status

### Step 3: inspect current code

Read the target code and nearby files.

Confirm:

- current naming style
- current public and private split
- dependency direction
- whether the file already contains mixed responsibilities
- whether an existing local pattern should be preserved

### Step 4: perform one bounded change

Prefer one of these per iteration:

- add one missing piece
- rename one API surface
- split one module responsibility
- create one small module
- update one file pair (`.h` plus `.c`)
- document one stable rule

Avoid mixed-purpose iterations.

### Step 5: review the result

Check:

- naming consistency
- ownership correctness
- module boundaries
- minimal scope
- similarity to accepted patterns in the repo

### Step 6: record continuity if needed

If the work is incomplete or staged across sessions:

- update `05_working_context.md`
- update `CHANGELOG.md` only if the change is meaningful project history
- do not place temporary planning notes into stable docs

---

## Recommended document roles

### Stable documents

Use these for durable rules:

- `AGENT_ENTRY.md`
- `01_quickstart.md`
- `02_workflow.md`
- `STYLE.md`
- `AGENTS.md`
- `style-examples/README.md`

### Operational prompt libraries

Use these for tool-specific prompting:

- `03_chatgpt_prompts.md`
- `04_codex_prompts.md`

### Mutable continuity document

Use this for live state:

- `05_working_context.md`

### Historical log

Use this for significant milestones:

- `CHANGELOG.md`

---

## Task sizing guidance

A task is usually well sized if:

- it affects one module or one narrow behavior
- it can be reviewed without reconstructing unrelated context
- success criteria are obvious
- style checks are local and concrete

A task is probably too large if:

- it changes several ownership boundaries at once
- it introduces a new pattern and refactors old ones together
- it spans unrelated folders
- it mixes naming cleanup with architecture changes

When in doubt, split the task.

---

## Handoff guidance

When preparing an execution prompt for an agent, include:

- the exact target
- the intended folder ownership
- the governing references
- the no-go boundaries
- the expectation of minimal change

Good handoff pattern:

> Update `<target>` to `<specific result>`.  
> Follow `AGENT_ENTRY.md` and `STYLE.md`.  
> Use the closest relevant example under `style-examples/`.  
> Keep the patch minimal and do not expand scope beyond `<boundary>`.

---

## Continuity guidance

Use `05_working_context.md` only for live session continuity.

It may include:

- current focus
- current assumptions
- known next step
- incomplete tasks
- temporary decisions awaiting confirmation

It must not become the source of truth for style, architecture, or permanent workflow.

---

## Anti-patterns

Avoid these workflow failures:

- asking the agent to "fix everything"
- skipping `STYLE.md`
- using examples as architecture instead of format guidance
- relying on a mutable context file as the main source of truth
- combining planning, implementation, and cleanup in one oversized prompt
- making undocumented naming exceptions inside generated code

---

## Related documents

- Entry point: `AGENT_ENTRY.md`
- Quick start: `01_quickstart.md`
- Style and naming: `STYLE.md`
- ChatGPT prompts: `03_chatgpt_prompts.md`
- Codex prompts: `04_codex_prompts.md`
- Live context: `05_working_context.md`
