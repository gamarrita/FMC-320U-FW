# src/product/fmc

## Intent

Contain FMC flow-computer product logic.

The initial split is:
- `fmc_model.*`: copyable FMC canonical state and structural helpers,
- `fmc_units.*`: unit and conversion policy for this product,
- `fmc_rate.*`: future instantaneous-rate calculation behavior,
- `fmc_service.*`: future RTOS-facing owner of live FMC state.

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
