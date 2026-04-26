# src/product/fmc

## Intent

Contain FMC flow-computer product logic.

The initial split is:
- `fm_fmc_model.*`: copyable FMC state and pure queries over canonical state,
- `fm_fmc_units.*`: unit and conversion policy for this product,
- `fm_fmc_rate.*`: future instantaneous-rate calculation behavior,
- `fm_fmc_service.*`: future RTOS-facing owner of live FMC state.

## Current Boundary

The current first slice is the FMC model contract only.

It should not own:
- acquisition or capture timing,
- RTOS synchronization,
- UI/menu authorization,
- LCD rendering,
- persistence or log layout.

Those responsibilities should be added as separate product modules when their
slice becomes active.
