# WORKING_CONTEXT.md

## Active Workstream

Stage:
- refactor

Short name:
- apps organization

Extended context:
- `docs/contexts/apps_organization.md`

## Scope Now

Active files/folders:
- `src/apps/`
- `CMakeLists.txt`
- `docs/contexts/apps_organization.md`

Current target:
- organize selectable firmware apps by role without changing runtime behavior.

## Current State

- `src/apps` contains product, bring-up, debug/fault calibration, template, and
  regression-style firmware profiles.
- The prior flat layout made app purpose less clear as new bringups and tests
  were added.
- `tests/regression` is the default automatic UART regression app.
- `bringups/*` apps remain focused human/hardware validation profiles.
- Cross references, canonical builds, flash, and UART regression were verified
  after the folder refactor.

## Decisions In Force

- Use `product/`, `bringups/`, `tests/`, and `template/`.
- Do not create `demos/` yet.
- Keep app selection explicit through `FM_ACTIVE_APP`.
- Keep `app_entry.c` thin in every selectable app.
- Do not split the regression harness into suites in this refactor.

## Boundaries

Do not add in this slice:
- new product behavior
- new test framework machinery
- changes to LCD, FMC, or display-format module behavior
- protected/generated code changes

## Next Step

1. Review and commit the refactor when accepted.
2. Continue feature work from the cleaned `src/apps` layout.

## References

- `AGENTS.md`
- `STYLE.md`
- `src/apps/README.md`
- `docs/contexts/apps_organization.md`
