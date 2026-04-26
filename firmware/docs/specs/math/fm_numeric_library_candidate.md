# FM Numeric Library Candidate

Purpose:
- capture decisions about the future numeric/fixed-point support library before
  freezing a public code API
- avoid forcing `fmc_model` to keep legacy numeric types such as `ufp3_t`
- keep a writable place for review comments while the FMC model is still under
  semantic clarification

Status:
- candidate
- not a stable contract
- not yet a code API

Context:
- the legacy FMC module used `ufp3_t` as a practical fixed-point strategy
- the refactor direction now prefers:
  - a dedicated numeric library
  - `fmc_model` depending on that library if needed
  - LCD formatting living in another layer

Current decision:
- `ufp3_t` should not remain owned by `fmc_model`
- a separate numeric library is preferred
- `fmc_model` may depend publicly on that library
- legacy decimal-selection policy (`fm_fmc_fp_sel`) also points toward this
  future numeric library instead of the FMC core model
- the FMC core model should preserve pulse counters as canonical backing state
  for totals
- derived volume/rate calculations may use `double` where appropriate, while
  editable or persistent decimal quantities may later use exact scaled numeric
  types
- when a value is stored as an integer scaled by 1000, use an explicit `_milli`
  suffix in variable names, for example `factor_k_milli`
- do not use `_milli` for unscaled `double` values

What this future library is expected to provide:
- domain-neutral numeric types
- deterministic arithmetic helpers
- explicit scaling rules
- conversion helpers
- any explicit precision/decimal policy that affects arithmetic strategy
- no LCD formatting
- no FMC-specific semantics

What this future library should not own:
- ACM/TTL/RATE semantics
- volume units or time-base semantics
- LCD row formatting
- alpha/unit rendering

Open questions:
- should the first public type be a generic fixed-point type or a narrower
  decimal/scaled-number type?
- should scale be runtime-carried in the type or fixed by a family of types?
- should conversion helpers return status codes on overflow/range loss?
- how much of the arithmetic needs to be public in the first slice?
- should the legacy simplification survive:
  - treat values mathematically as if they always carry three decimals
  - then vary only presentation
  or should the new numeric contract make precision ownership more explicit?

Candidate layering:
1. `fm_num.*`
   - generic numeric support
2. `fmc_model.*`
   - FMC domain semantics
3. `fmc_presentation.*`
   - FMC-to-display semantics
4. LCD adapter
   - rendering onto the validated LCD stack

Why this exists now:
- too many FMC model decisions currently depend on how numeric state will be
  represented
- legacy decimal selection affected both arithmetic and display, which means
  the numeric boundary still needs clarification before freezing the FMC model
- creating a small review artifact now is safer than prematurely freezing a
  code header

Reviewer notes:
- free-form comments are welcome here
- once the numeric direction stabilizes, this file can be replaced by:
  - a real `src/libs/fm_num.h`
  - or a tighter YAML/contract artifact if that becomes more useful
