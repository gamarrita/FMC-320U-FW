# Firmware Versioning

## Scope

This document owns forward-looking firmware version and release traceability
policy. Hardware revision remains independent. Historical tags are not renamed,
moved, or reinterpreted by this policy.

## Logical Version

Firmware versions use `MAJOR.MINOR.PATCH`:

- `MAJOR` changes for incompatible product behavior, persistent configuration,
  or communication interfaces.
- `MINOR` changes for new compatible functionality.
- `PATCH` changes for compatible corrections without relevant new
  functionality.

Examples:

- stable: `v1.4.2`;
- beta: `v1.4.2-beta.1`.

Published betas range from `beta.1` through `beta.9`. If more beta releases are
needed, increment `PATCH` and restart at `beta.1`.

## LCD Representation

The constrained LCD representation is `MM.mm.pp`:

- each component uses exactly two digits with leading zeros;
- each component is limited to `0..99`;
- a component above `99` must fail the build;
- the value is never truncated to fit the LCD.

A stable `v1.4.2` displays as `01.04.02` with an empty alphanumeric field. Beta
1 displays as `01.04.02 B1`. `B0` is reserved exclusively for the unpublished
Phase 6A dummy version.

This restricted display form is not complete Semantic Versioning
compatibility.

## Release Traceability

- Every stable or beta firmware installed on equipment or sent to production
  must correspond to one unique Git tag.
- Internal-only builds may be identified by commit.
- Published tags are immutable and must not be moved or reused.
- Production firmware must be built from the exact tagged commit with a clean
  worktree.
