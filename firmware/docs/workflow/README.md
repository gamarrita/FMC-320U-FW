# Workflow Model

## Purpose

This directory contains stage-specific workflow documents.

Its job is simple:
- define the default stage model used in this repo
- explain what each stage is for
- point to more specific pass documents when needed

This directory is not the task entrypoint.
Use `WORKING_CONTEXT.md` first.

---

## Default Stage Model

Routine work in this repo should normally be understood through these stages:

1. analysis
2. plan
3. implementation
4. validation
5. closure

This is the default sequence, not a rigid ceremony.

Small tasks may combine stages.
Example:
- a tiny fix may combine analysis and plan
- a very local change may go from implementation directly to validation

---

## Stage Meanings

### 1. Analysis

Use this stage to understand the current code before changing it.

Typical outputs:
- current responsibility
- local patterns already in use
- risks
- what should stay untouched

### 2. Plan

Use this stage to define the smallest correct change.

Typical outputs:
- target files
- desired outcome
- out-of-scope boundaries
- recommended implementation sequence

### 3. Implementation

Use this stage to make the code or documentation change.

Typical outputs:
- the requested change
- a small summary of what changed
- explicit note of what remains for later passes

### 4. Validation

Use this stage to verify that the result is acceptable.

Typical outputs:
- what was validated
- how it was validated
- what remains unvalidated

### 5. Closure

Use this stage after the main behavior is stable.

Closure passes are for convergence work, not for inventing new behavior.

Typical closure examples:
- comment pass
- small naming cleanup
- final documentation adjustment
- focused polish that depends on the implementation already being stable

---

## Specialized Closure Passes

Some closure passes have their own documents.

Current example:
- `docs/workflow/comment_pass.md`

Use a specialized pass document only when that pass is the actual task.

---

## Practical Rule

Do not assume every request must do all stages at once.

Prefer explicit requests such as:
- analyze this module
- analyze and propose a plan
- implement this change
- validate this change
- run a comment pass on these files

This keeps tasks smaller and reviewable.

---

## Relationship To Other Repo Docs

- `WORKING_CONTEXT.md`
  - task routing, active contexts, current priorities
- `STYLE.md`
  - naming and code structure
- `CONTRIBUTING.md`
  - repo hygiene such as commits, changelog, and release handling

If documents overlap:
- `WORKING_CONTEXT.md` decides where to start
- `STYLE.md` wins on naming and structure
