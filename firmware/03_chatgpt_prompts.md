# 03_chatgpt_prompts.md

## Purpose

This file contains reusable prompts for working with ChatGPT as a planning,
analysis, and review assistant inside `firmware/`.

Use this file when the goal is to:

- clarify a task
- reduce scope
- review ownership
- review naming
- evaluate module boundaries
- improve documentation
- prepare a better execution prompt

This file is not for direct code execution prompts.
For execution-oriented prompts, use `04_codex_prompts.md`.

Stable repository rules live in:

- `AGENT_ENTRY.md`
- `01_quickstart.md`
- `02_workflow.md`
- `STYLE.md`
- `AGENTS.md`

---

## When to use ChatGPT

Use ChatGPT when you need help to think before changing code.

Typical use cases:

- turn a vague request into a small task
- decide which folder should own a change
- review whether a file mixes concerns
- compare two naming alternatives
- review whether a module split makes sense
- reorganize docs
- prepare a prompt for Codex or another execution-oriented agent

---

## Prompt: turn a vague request into a small task

```text
We are working in the `firmware/` folder of this repository.

Help me turn this request into a small, concrete, reviewable task.

Raw request:
<paste request>

Use these references:
- firmware/AGENT_ENTRY.md
- firmware/01_quickstart.md
- firmware/STYLE.md
- firmware/AGENTS.md

I want:
1. a clearer task statement
2. expected owning folder
3. likely affected files
4. what should be explicitly out of scope
5. a final task description ready for execution
```

---

## Prompt: decide folder ownership

```text
Help me decide where this change should live inside `firmware/`.

Change description:
<paste change>

Candidate folders:
- app/
- bsp/
- cube/
- port/
- services/

Use these references:
- firmware/AGENT_ENTRY.md
- firmware/01_quickstart.md
- firmware/02_workflow.md
- firmware/STYLE.md

Please answer:
1. the most appropriate owner
2. why that folder is the best fit
3. which alternatives are weaker and why
4. what existing repository pattern should be followed
```

---

## Prompt: review naming choices

```text
Help me choose repository-consistent naming for this module or API.

Context:
<paste module purpose>

Candidate names:
<paste names>

Use these references:
- firmware/STYLE.md
- firmware/style-examples/README.md

Please evaluate:
1. best public function names
2. best private helper names
3. callback naming if applicable
4. wrapper naming if applicable
5. rationale based on repository consistency
```

---

## Prompt: review a file for mixed responsibilities

```text
Analyze whether this file appears to mix responsibilities.

Input file or snippet:
<paste content>

Use these references:
- firmware/AGENT_ENTRY.md
- firmware/01_quickstart.md
- firmware/STYLE.md

I want:
1. the responsibilities currently present
2. what responsibility the file seems to own primarily
3. what looks out of place
4. whether this should be split now or only contained
5. the smallest reasonable next step
```

---

## Prompt: review a proposed module design

```text
Review this proposed module design before implementation.

Proposal:
<paste short design>

Use these references:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md

Check:
1. ownership
2. API surface size
3. public and private split
4. naming consistency
5. whether the structure is too large for one task
6. what should be simplified before coding
```

---

## Prompt: prepare an execution prompt

```text
Help me prepare a precise execution prompt for Codex or another code agent.

Task:
<paste task>

Constraints:
- keep the change minimal
- do not expand scope
- follow repository naming and structure
- match the closest relevant example

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md

Return:
1. a compact execution prompt
2. a stricter execution prompt
3. a refactor-only version if applicable
```

---

## Prompt: documentation cleanup

```text
Analyze the documentation structure under `firmware/` and propose a cleaner split.

Use these references:
- firmware/AGENT_ENTRY.md
- firmware/01_quickstart.md
- firmware/02_workflow.md
- firmware/STYLE.md
- firmware/AGENTS.md

I want:
1. duplicated topics
2. which file should own each topic
3. suggested renames if needed
4. what should be stable docs versus prompt library versus working context
5. a final recommended structure
```

---

## Prompt: review whether a task is too large

```text
Review this task and tell me whether it is appropriately scoped.

Task:
<paste task>

Use these references:
- firmware/AGENT_ENTRY.md
- firmware/01_quickstart.md
- firmware/02_workflow.md

Answer:
1. whether the task is too large
2. what makes it too large, if applicable
3. how to split it into smaller reviewable tasks
4. which one should be done first
```

---

## Notes

Use ChatGPT prompts for:

- planning
- analysis
- comparison
- review
- preparation

Do not use this file as the source of truth for repository rules.
If a stable rule is missing, add it to the appropriate stable document.
