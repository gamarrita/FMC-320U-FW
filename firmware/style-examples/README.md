# style-examples/README.md

## Purpose

This directory contains code examples that illustrate repository conventions.

These examples are reference material for:

- naming
- public and private split
- `.h` and `.c` structure
- callback forwarding
- wrapper style
- minimal module shape

They are examples of form and local convention.
They are not the target architecture for the full firmware.

---

## How to use these examples

Before creating or editing a module:

1. identify the responsibility of the target code
2. choose the closest relevant example
3. inspect naming, layout, and symbol visibility
4. copy the pattern shape, not the architecture blindly
5. adapt only what is required by the real module responsibility

---

## What to learn from examples

Use examples to understand:

- how public functions are named
- how private helpers are named
- how headers stay minimal
- how source files keep internals local
- how callbacks forward into project-local helpers
- how wrappers are named and scoped
- how a small module is laid out

---

## What not to infer from examples

Do not assume from an example that:

- every feature should use the same architecture
- every module needs the same file count
- every module needs wrappers
- every module needs callbacks
- every pattern should be generalized

Examples are intentionally small.
Real modules must still respect ownership and responsibility boundaries.

---

## Example selection guidance

Use the example closest to your task:

- module creation example for new `.h` and `.c` pairs
- callback example for HAL or framework callback handling
- wrapper example for adaptation of external APIs
- service-like example for reusable internal logic
- app-like example for orchestration patterns, if present

If more than one example seems relevant,
prefer the one closest in responsibility and public versus private split.

---

## Relationship with `STYLE.md`

`STYLE.md` is the source of truth for rules.

This directory shows how those rules look in code.

If an example conflicts with `STYLE.md`:
- follow `STYLE.md`
- then update the example later if needed

---

## Maintenance rule

Keep examples:

- small
- clear
- stable
- focused on one pattern each

Do not turn this directory into a dump of old code fragments.
Each example should remain intentionally useful.
