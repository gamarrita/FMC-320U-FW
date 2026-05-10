# Refactor Wizard Sandbox

## Purpose

This folder is a sandbox for exercising the `refactor` bootstrap wizard MVP on
multiple plausible repo cases without turning those trials into active
workstreams.

It is not stable policy.
It is not a source of truth for actual module ownership.

Use it to:
- test whether the current wizard prompt stops at the right time
- test whether the current wizard asks too much or too little
- identify prompt or contract gaps before running a real refactor bootstrap

## Expected Contents

Each trial should stay lightweight and should normally capture:
- the user-style request
- what the wizard can infer safely from the repo
- what remains blocking or non-blocking
- whether the wizard should bootstrap now or keep asking
- a short checklist verdict
- the concrete lesson extracted from that case

## Boundary

Trials here do not create a real active `WORKING_CONTEXT.md`.
They are only evidence for improving the wizard MVP.
