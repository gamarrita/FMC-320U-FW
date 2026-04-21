# Safety Pass File Guide

## Purpose

This file explains the internal structure of `docs/workflow/passes/safety/`.

Use it to understand:
- which file to read first
- which file defines policy
- which file only supports execution
- which document wins if two texts pull in different directions

## Folder Tree

```text
docs/workflow/passes/safety/
|-- README.md
|-- CONTRACT.md
|-- FILES.md
|-- prompts/
|   `-- bootstrap_review.md
`-- examples/
    |-- README.md
    |-- review_example.md
    `-- hardening_example.md
```

## File Roles

- `README.md`
  Entry point and navigation file for the container.
- `CONTRACT.md`
  Stable contract for what the safety pass covers and how it should behave.
- `FILES.md`
  Structure, hierarchy, and precedence guide for this folder.
- `prompts/bootstrap_review.md`
  Reusable operational prompt to start a safety review inside the current
  working context.
- `examples/README.md`
  Explains the role and limits of the example files.
- `examples/review_example.md`
  Illustrative example of a safety review output.
- `examples/hardening_example.md`
  Illustrative example of a small, allowed hardening output.

## Internal Hierarchy

1. `README.md`
   Entry point and reading order.
2. `CONTRACT.md`
   Stable pass contract.
3. `FILES.md`
   Folder organization and precedence clarification.
4. `prompts/`
   Reusable operational support.
5. `examples/`
   Illustrative material only.

## Precedence Rules

Across the repository:
- `WORKING_CONTEXT.md` wins on current context and current priority
- `STYLE.md` wins on naming and code structure
- `docs/workflow/README.md` wins on workflow stage model
- `docs/workflow/passes/README.md` wins on pass classification

Inside this folder:
- `README.md` defines how to navigate the container
- `CONTRACT.md` defines safety-pass policy and output expectations
- `FILES.md` explains structure and precedence
- `prompts/` helps operate the pass but does not define policy
- `examples/` illustrate outcomes but do not define policy

## If There Is Tension

Use this order:
1. `WORKING_CONTEXT.md`
2. `STYLE.md` for naming and structure
3. `docs/workflow/passes/README.md` for pass type and bootstrap status
4. `docs/workflow/passes/safety/CONTRACT.md` for safety-pass criteria
5. `docs/workflow/passes/safety/README.md` for navigation
6. `docs/workflow/passes/safety/prompts/`
7. `docs/workflow/passes/safety/examples/`

Practical rule:
- examples never override the contract
- prompts never reclassify the pass
- the safety contract does not replace `STYLE.md`
