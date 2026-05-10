# Safety Pass

## Role

Focused methodological pass for safety-like review and small hardening work.

This folder is a non-bootstrap container.
It groups the stable contract, file guide, support prompt, and examples for the
pass.

## When to use

Use it when safety-like review or hardening is the explicit task inside the
current working context.

Typical cases:
- ISR versus foreground behavior review
- blocking-path review
- timeout and wait-path review
- init and state-contract review
- focused hardening after a concrete issue has been identified

## What it does

- checks safety-relevant concerns in the requested scope
- reports concrete safety-like findings
- supports small, reviewable hardening work
- keeps safety review separate from broad redesign

## What it does NOT do

- does not create or replace `WORKING_CONTEXT.md`
- does not become the default concern of all stages
- does not replace `STYLE.md` for naming or code structure
- does not justify unrelated redesign by default

## Working Context

- requires existing `WORKING_CONTEXT.md`: Yes
- can modify it: No
- creates it: No
- pass type: non-bootstrap

The support prompt under `prompts/` may help bootstrap a safety review inside
the existing context, but it does not change the formal pass type.

## Files In This Folder

- `README.md`
  Entry point for this container.
- `CONTRACT.md`
  Stable contract for safety-pass scope, concerns, and expected outputs.
- `FILES.md`
  File guide, hierarchy, and precedence rules.
- `prompts/bootstrap_review.md`
  Optional support prompt to start a safety review inside the current context.
- `examples/`
  Illustrative, non-normative examples.

## Recommended Reading Order

1. `WORKING_CONTEXT.md`
2. The active context file referenced by `WORKING_CONTEXT.md`, if relevant
3. `docs/workflow/README.md`
4. `docs/workflow/passes/README.md`
5. `docs/workflow/passes/safety/README.md`
6. `docs/workflow/passes/safety/CONTRACT.md`
7. `STYLE.md` only when naming or code structure matters
8. `docs/workflow/passes/safety/prompts/` and `examples/` only as support material

## Output

A focused safety-oriented review or a small hardening pass over the requested
scope, with explicit boundaries and explicit validation limits.
