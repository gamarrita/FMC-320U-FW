# Working Context: Phase 7 Minimum Acquisition Slice Definition

## Objective

Define the smallest acquisition slice that can replace the provisional
TTL/RATE inputs with runtime-owned live values. This workstream produces a
reviewed definition only; it does not implement acquisition or select hardware
unilaterally.

## Authorities And Evidence

- product obligations: `docs/product/fmc/requirements.md`;
- cross-cutting observable behavior: `docs/product/fmc/behavior.md`;
- visible behavior, if a decision affects presentation:
  `docs/product/fmc/user_interface.md`;
- durable phase dependencies and decision gates:
  `docs/roadmaps/fmc_refactoring.md`;
- implemented domain boundaries: `src/product/fmc/fmc_runtime.h`,
  `src/product/fmc/fmc_service.h`, and `src/product/fmc/fmc_rate.h`;
- configured hardware facts: `fmc-320u-v2.ioc`, read only; CubeMX remains the
  hardware-configuration source of truth;
- legacy evidence: `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- reviewed legacy disposition: `docs/workflow/fmc_legacy_coverage.md`.

Legacy material may support review but does not define current product
behavior. Phase 6A is closed; its accepted behavior remains in product
documentation, implemented contracts, tests, the roadmap, and Git history.

## Accepted Starting Point

- Both ACM and TTL accumulate the same accepted raw pulse deltas.
- `fmc_runtime` serializes product events and updates its owned `fmc_service`;
  acquisition does not own product totals.
- RATE is derived from a pulse delta and a positive elapsed-time window using
  active measurement configuration.
- Presentation consumes accepted TTL and RATE values; it does not capture
  pulses or own the RATE observation window.
- Current public contracts accept a `uint64_t` raw pulse delta observed since
  the last accepted update and report counter overflow explicitly.

These statements are constraints on the definition, not approval of a physical
capture design.

## Definition Questions

The active workstream must prepare explicit human decisions for:

- ownership and lifecycle of the acquisition window;
- meaning, bounds, and acceptance point of each pulse delta;
- behavior when observations are lost, unavailable, saturated, delayed, or
  absent during a completed window;
- the distinction between zero observed flow and an invalid or missing
  observation;
- how an accepted window produces totalization input, RATE calculation, and
  serialized runtime updates without double-counting pulses;
- timing and low-power assumptions that the selected minimum slice actually
  requires;
- the physical input and peripheral candidate, including whether CubeMX
  configuration is required;
- whether RTC or persistence is a real dependency of the selected slice.

## Human Approval Gates

Human approval is required before:

- promoting candidate or unresolved acquisition behavior into product
  requirements or observable behavior;
- choosing a pin, peripheral, capture mode, clock, interrupt, DMA path, or
  CubeMX change;
- adopting a loss, saturation, zero-flow, retry, or recovery policy;
- adding RTC, persistence, or visible failure behavior to the slice;
- beginning implementation from the completed definition.

If current specifications, legacy evidence, hardware constraints, and existing
contracts conflict, record the conflict and stop rather than choosing silently.

## Out Of Scope

- acquisition implementation or firmware changes;
- `.ioc` edits or CubeMX regeneration;
- unilateral pin or peripheral selection;
- RTC or persistence changes;
- new visible product behavior without human approval;
- navigation, configuration, backlight, alarms, or Phase 6B;
- reopening or revalidating Phase 6A.

## Closure Criteria

This definition workstream is complete when:

- one bounded acquisition flow is documented from physical observation through
  pulse delta, RATE window, and serialized runtime update;
- window ownership, delta semantics, numeric limits, and acceptance boundaries
  are explicit;
- loss, saturation, absent-observation, and zero-flow cases are either
  human-approved for the slice or explicitly excluded with a safe
  pre-implementation gate;
- the physical input/peripheral choice is approved, or the exact CubeMX/human
  decision gate that blocks implementation is recorded;
- RTC and persistence are included only if a concrete dependency is
  demonstrated and approved;
- accepted product decisions are written only in their owning product
  documents, while legacy sources remain evidence;
- no firmware, generated source, protected file, `.ioc`, RTC, or persistence
  content changes;
- documentation closure and final Git audit pass.
