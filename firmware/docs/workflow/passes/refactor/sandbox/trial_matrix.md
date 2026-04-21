# Refactor Wizard Trial Matrix

## Purpose

This file summarizes the sandbox trials used to pressure-test the current
`refactor` wizard MVP.

## Trial Set

### Case 01
- file:
  - `docs/workflow/passes/refactor/sandbox/case_01_port_time_clear.md`
- type:
  - fresh bootstrap
- expected wizard behavior:
  - bootstrap immediately
- result:
  - current MVP behavior is strong for this case

### Case 02
- file:
  - `docs/workflow/passes/refactor/sandbox/case_02_app_entry_partial.md`
- type:
  - fresh bootstrap with blocking gaps
- expected wizard behavior:
  - ask questions first
- result:
  - exposed a response-mode gap in the current prompt wording

### Case 03
- file:
  - `docs/workflow/passes/refactor/sandbox/case_03_debug_panic_reframe.md`
- type:
  - explicit reframe request
- expected wizard behavior:
  - detect reframe mode before touching output files
- result:
  - exposed a create-versus-reframe gap

## Adjustments Justified By The Trials

These trial results justify two concrete adjustments:

1. The prompt must support two response modes:
   - question mode when blocking inputs are still missing
   - bootstrap mode only after the stop rule is satisfied

2. The prompt must treat create-versus-reframe as an explicit branch when the
   user request already signals a reframe.

## What The Trials Did Not Justify

The trials did not justify:
- broad changes to `new_feature`
- broader workflow changes outside `refactor`
- a new final contract layer above the current candidates

## Verdict

The current wizard MVP was directionally correct, but not yet ready for a real
trial without:
- question-mode wording
- explicit create-versus-reframe handling
