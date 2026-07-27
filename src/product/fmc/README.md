# src/product/fmc

## Intent

Contain FMC flow-computer product logic.

The initial split is:
- `fmc_model.*`: copyable FMC canonical state and structural helpers,
- `fmc_input.h`: semantic product input event contract,
- `fmc_presentation.*`: startup and live TTL/RATE semantic frames,
- `fmc_version.h`: bounded provisional firmware version metadata,
- `fmc_units.*`: unit and conversion policy for this product,
- `fmc_rate.*`: pure instantaneous-rate calculation from pulse/time windows,
- `fmc_volume.*`: pure visible-volume calculation from ACM/TTL pulse counters,
- `fmc_service.*`: live FMC state owner and snapshot/update boundary,
- `fmc_runtime.*`: RTOS-neutral product event dispatch boundary over the
  service, including current RATE value, presence, and observation quality.

## Current Boundary

The current implemented slices are:
- `fmc_model.*`: canonical model state and structural helpers,
- `fmc_input.h`: product key/action identity after hardware translation,
- `fmc_presentation.*`: RTOS-neutral all-segments, version, and TTL/RATE
  presentation state,
- `fmc_version.h`: compile-time checked provisional version components,
- `fmc_units.*`: product unit policy and operative factor helpers,
- `fmc_rate.*`: active-unit rate derived from captured pulse/time windows,
- `fmc_runtime.*`: product event dispatch, service ownership, current
  RATE/quality state, and presentation-update pending boundary,
- `fmc_service.*`: live state owner and snapshot/update boundary,
- `fmc_volume.*`: active-unit volume derived from ACM/TTL pulse counters.

These modules should not own:
- acquisition or capture timing,
- RTOS synchronization,
- UI/menu authorization,
- LCD rendering,
- persistence or log layout.

Those responsibilities should be added as separate product modules when their
slice becomes active.

For an active refactor context, use `WORKING_CONTEXT.md` when present and the
applicable section of `docs/roadmaps/fmc_refactoring.md`.
