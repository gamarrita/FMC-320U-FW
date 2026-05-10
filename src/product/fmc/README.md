# src/product/fmc

## Intent

Contain FMC flow-computer product logic.

The initial split is:
- `fmc_model.*`: copyable FMC canonical state and structural helpers,
- `fmc_units.*`: unit and conversion policy for this product,
- `fmc_rate.*`: future instantaneous-rate calculation behavior,
- `fmc_service.*`: future RTOS-facing owner of live FMC state.

## Current Boundary

The current implemented slices are:
- `fmc_model.*`: canonical model state and structural helpers,
- `fmc_units.*`: product unit policy and operative factor helpers.

These modules should not own:
- acquisition or capture timing,
- RTOS synchronization,
- UI/menu authorization,
- LCD rendering,
- persistence or log layout.

Those responsibilities should be added as separate product modules when their
slice becomes active.

For the active refactor context, use:
- `WORKING_CONTEXT.md`
- `docs/contexts/fmc_presentation_refactor.md`
