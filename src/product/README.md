# product

## Intent

Contain product-specific firmware logic for the FMC-320U.

This layer is not intended to be a portable library boundary. It may assume the
chosen RTOS, repository architecture, and product semantics.

## Should contain

- FMC-320U domain models and behavior,
- product services that own runtime state,
- product-specific orchestration that is shared by application flows,
- code that would not make sense outside this firmware/product line.

## Should NOT contain

- CubeMX-generated code,
- vendor HAL or middleware code,
- board wiring or physical device drivers,
- low-level MCU/runtime adaptation,
- selectable app entrypoints.

## Relationship with other folders

- `src/port/` adapts the MCU, toolchain, and RTOS/base runtime.
- `src/bsp/` owns board and device-facing hardware details.
- `src/services/` owns reusable technical services that are not product semantics.
- `src/apps/` selects runnable firmware profiles and should stay thin.

## Practical guideline

If the code describes what this flow computer is or does, it likely belongs
here.
