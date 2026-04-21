# Safety Pass Contract

## Purpose

This contract defines the scope and expected behavior of the safety pass in
`firmware/`.

The pass is for safety-like review and small hardening work.
It is not a certification process and it is not a general redesign pass.

## Main Goal

Use this pass to identify and tighten behaviors that could create unstable,
unsafe, or misleading runtime behavior in the requested scope.

The pass should prefer:
- explicit context
- explicit ownership
- explicit failure handling
- small, reviewable hardening steps

## What Belongs In This Pass

Safety-like concerns in this repo include:
- ISR versus foreground misuse
- blocking calls used in the wrong context
- waits, delays, and timeout paths with unclear behavior
- missing or weak argument validation at module boundaries
- weak init or state contracts
- unclear ownership of hardware access
- unclear ownership of shared mutable state
- error propagation that hides failure origin
- panic paths that are inconsistent with the module role
- insufficient observability to validate a safety-relevant behavior

## What Does Not Belong By Default

Do not use this pass by default for:
- naming cleanup
- style cleanup unrelated to safety behavior
- broad API redesign
- speculative architecture improvement
- generic performance tuning
- moving code between layers unless safety ownership is the real issue
- contract cleanup that is not materially tied to a safety-like risk

If one of those becomes necessary, report it as a boundary or follow-up item.

## Operating Rules

### ISR Versus Foreground

- Identify whether a path runs in ISR, foreground, or both
- Treat blocking or formatting work in ISR as a concrete concern
- Prefer thin ISR entry points and deferred foreground work
- If a function is foreground-only, that limitation should be explicit

### Blocking Calls And Context

- Check whether blocking UART, SPI, delay, or wait operations are used
- Check whether the calling context is declared or inferable
- Treat undeclared blocking behavior as a review concern when it can mislead usage

### Timeouts And Waits

- Timeouts should be explicit where operations can fail or stall
- Busy waits and delays should be justified by module role
- Review whether wait behavior is bounded and understandable

### Preconditions And Validation

- Public and boundary-crossing functions should validate meaningful inputs
- Invalid pointers, invalid ranges, and invalid state should be handled explicitly
- Prefer predictable failure codes or clear panic ownership over silent misuse

### Init And State Contracts

- Modules should make initialization requirements clear
- Review whether state-dependent APIs fail predictably before init or after reset
- Do not expand the pass into a full state-machine redesign unless explicitly requested

### Ownership

- Hardware access ownership should be clear at the active layer boundary
- Shared mutable state should have clear write/read ownership
- If ownership is mixed, report the safety implication before proposing structure cleanup

### Error Propagation Versus Panic

- Review whether a failure should propagate or panic based on module role
- Panic paths should be explicit and reserved for justified fatal conditions
- Report unclear recovery ownership as a safety finding

### Observability

- A safety-relevant behavior should be reviewable through code evidence, debug output,
  or another explicit observation path
- If validation depends on observability that does not exist yet, report that limit
  explicitly instead of inventing a larger change

## Expected Output

A safety pass should produce:
- the reviewed scope
- the concrete safety-like concerns found or the explicit statement that none were found
- the risk or failure mode implied by each concern
- the smallest hardening direction supported by the evidence
- the validation limit when something could not be confirmed
- the boundary intentionally left untouched

## Scope Discipline

This pass should not turn into a broad redesign.

Prefer:
- local evidence
- local fixes
- explicit follow-up notes

Do not use safety language as a reason to widen scope without a concrete,
repo-relevant failure mode.
