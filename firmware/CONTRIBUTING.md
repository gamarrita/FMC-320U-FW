# CONTRIBUTING.md

## Purpose

This file describes repository hygiene for contributions under `firmware/`.

It is about how to manage changes in the repo.
It is not the workflow entrypoint for agents.
It is not the style guide.

For task routing, use `WORKING_CONTEXT.md`.
For style and naming, use `STYLE.md`.

---

## Scope

Use this document for:
- commits
- changelog updates
- release or tag-related handling when applicable
- keeping changes integrable and easy to review

Do not use this document as the source of truth for:
- coding style
- workflow stages
- active refactor context

---

## Commit Expectations

Prefer commits that are:
- small
- scoped to one clear change
- easy to review

Avoid mixing unrelated topics in one commit.

Good examples:
- one bug fix
- one local refactor
- one documentation cleanup
- one validation-related adjustment

Less useful examples:
- behavior change mixed with naming sweep
- implementation mixed with unrelated cleanup
- large mechanical noise without a clear purpose

---

## Changelog

Update `CHANGELOG.md` when the change is repository-visible and worth preserving as project history.

Typical examples:
- a new stable workflow or documentation contract
- a new canonical tool or entrypoint
- a meaningful architectural milestone
- a removed or replaced repository mechanism

Do not force a changelog entry for every small local patch.

When updating the changelog:
- keep the entry short
- describe the repository-visible outcome
- avoid patch-level narration

---

## Tags And Releases

Tags and releases should be handled intentionally, not as part of routine coding work.

If a task explicitly includes release preparation:
- make sure the relevant changelog state is ready
- make sure the intended build and validation state is known
- avoid mixing unrelated last-minute cleanup into the release change

If the task does not explicitly include release work:
- do not introduce tag or release changes speculatively

---

## Before Considering A Change Ready

Before treating a contribution as ready to integrate:
- confirm the task scope is clear
- confirm the change is reasonably reviewable
- confirm stable docs were updated only when needed
- confirm the change does not leave the repo in a more confusing state

If a stable repository rule changed:
- update the one file that should own that rule
- avoid duplicating the same rule across multiple markdown files

---

## Related Documents

- task routing: `WORKING_CONTEXT.md`
- style and naming: `STYLE.md`
- workflow stages: `docs/workflow/README.md`
