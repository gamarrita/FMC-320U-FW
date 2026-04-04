# 04_codex_prompts.md

## Purpose

This file contains reusable prompt templates for Codex or other execution-oriented agents.

Use this file when the task is already clear and the goal is to inspect, modify,
or generate code with minimal ambiguity.

This file is for direct code inspection and modification prompts.

This file is not the source of truth for style or workflow.
Stable rules live in:

- `AGENT_ENTRY.md`
- `01_quickstart.md`
- `02_workflow.md`
- `STYLE.md`
- `AGENTS.md`

---

## General guidance

Execution prompts should be:

- narrow
- explicit
- bounded
- linked to the correct references
- clear about what must not change

Prefer prompts that target one module, one responsibility, or one local refactor at a time.

---

## Prompt template: small implementation

```text
Work in `firmware/`.

Task:
Implement <specific change> in <target file or module>.

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md
- closest relevant example under style-examples/

Constraints:
- keep the change minimal
- do not expand scope
- do not introduce new architecture
- keep public and private split consistent
- follow existing repository naming

Expected result:
- only the necessary files changed
- consistent naming
- reviewable patch
```

---

## Prompt template: small refactor

```text
Work in `firmware/`.

Task:
Perform a small refactor in <target file or module> to achieve <specific goal>.

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md

Constraints:
- refactor only
- no behavior expansion
- no unrelated cleanup
- keep file ownership clear
- do not rename unrelated symbols

Expected result:
- minimal patch
- improved structure
- unchanged intended behavior
```

---

## Prompt template: split mixed-responsibility module

```text
Work in `firmware/`.

Task:
Analyze and split mixed responsibilities in <target module> only if a minimal split is justified.

References:
- firmware/AGENT_ENTRY.md
- firmware/01_quickstart.md
- firmware/STYLE.md
- firmware/style-examples/README.md

Constraints:
- preserve behavior
- prefer minimal movement
- do not redesign the architecture
- keep naming repository-consistent
- explain the new ownership boundary

Expected output:
- proposed split
- affected files
- rationale for ownership
- minimal implementation
```

---

## Prompt template: create new module

```text
Work in `firmware/`.

Task:
Create a new module for <specific responsibility> in the appropriate folder.

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md
- closest relevant example under style-examples/

Constraints:
- choose the correct owning folder
- create only the minimum necessary public API
- keep helpers private in the `.c` file
- follow repository naming exactly
- do not add extra abstraction layers

Expected result:
- new `.h` and `.c` pair if justified
- clear responsibility
- minimal API surface
- consistent structure
```

---

## Prompt template: naming cleanup

```text
Work in `firmware/`.

Task:
Adjust naming in <target file or module> to match repository conventions.

References:
- firmware/STYLE.md
- firmware/style-examples/README.md

Constraints:
- rename only what is necessary
- keep scope local
- do not combine with architecture changes
- preserve behavior

Expected result:
- repository-consistent names
- minimal patch
- no unrelated edits
```

---

## Prompt template: review before edit

```text
Work in `firmware/`.

Task:
Inspect <target file or module> and report:
1. main responsibility
2. public API
3. private helpers
4. naming mismatches against STYLE.md
5. whether the file appears to mix concerns
6. minimal recommended next step

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/style-examples/README.md

Do not modify code yet.
Only inspect and report.
```

---

## Prompt template: documentation-aligned code update

```text
Work in `firmware/`.

Task:
Update code in <target> so it aligns with the documented naming and module conventions.

References:
- firmware/AGENT_ENTRY.md
- firmware/STYLE.md
- firmware/AGENTS.md
- firmware/style-examples/README.md

Constraints:
- no broad cleanup
- no speculative improvements
- keep the patch reviewable
- use the existing local pattern whenever valid

Expected result:
- code aligned with docs
- minimal necessary edits
```

---

## Notes

Use this file after the task has already been clarified.

Do not use execution prompts as a substitute for:
- architecture decisions
- workflow definitions
- stable style rules

If the prompt needs to explain repository rules in full,
the stable documentation is incomplete and should be improved.
