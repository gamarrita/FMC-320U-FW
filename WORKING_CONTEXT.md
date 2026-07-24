# Working Context: FMC Phase 6A Closure

## Objective And Status

Phase 6A is completed and human-accepted. It established the first bounded path
from an FMC presentation snapshot to controlled LCD output without introducing
complete navigation, configuration, acquisition, persistence, or backlight
policy.

## Implemented Behavior

- After successful LCD initialization, startup presents all software-controlled
  segments, provisional firmware version `00.01.00 B0`, and steady TTL/RATE in
  that order.
- The two temporary views each have a nominal 3 second dwell. TTL/RATE appears
  immediately on entry and refreshes once per second.
- The controlled snapshot presents `1234.5 Lt` for TTL and `12.3 Lt/min` for
  RATE. The two-character alphanumeric field renders `Lt`; dedicated `TTL`,
  `RATE`, slash, and minute indicators complete the visible units.
- Values use one decimal, right alignment, and least-significant visual
  overflow.
- Presentation state and dwell advance only after successful LCD delivery.
- A SHORT ESC has the same semantic transition effect as timeout while a
  temporary startup view is active.
- LCD initialization and keyboard EXTI setup no longer reset the controller
  after the first frame. Backlight state is never read or changed.

## Active Sources

- product obligations: `docs/product/fmc/requirements.md`;
- cross-cutting behavior: `docs/product/fmc/behavior.md`;
- visible and operable behavior: `docs/product/fmc/user_interface.md`;
- durable sequence and next phase: `docs/roadmaps/fmc_refactoring.md`;
- release version policy: `docs/project/firmware_versioning.md`;
- physical LCD facts: `docs/specs/lcd/lcd_true_source.yaml`;
- implemented contracts: public headers under `src/product/fmc/`,
  `src/apps/product/main/`, `src/bsp/devices/lcd/`, and `src/port/`;
- legacy evidence: `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- reviewed legacy disposition: `docs/workflow/fmc_legacy_coverage.md`.

## Verification Baseline

- canonical `product/main` build passes;
- canonical `tests/regression` build passes;
- regression coverage includes startup order, all-segments RAM coverage,
  version and TTL/RATE frames, `Lt` alpha mapping, formatting, rounding,
  alignment, overflow, refresh input, and LCD-sink failure propagation;
- the final `product/main` image was programmed and verified through ST-LINK;
- the complete three-state scene was visually accepted on target hardware;
- generated code, CubeMX configuration, LCD technical YAML, backlight, and
  legacy sources remain unchanged.

## Deferred And Limitations

- SHORT ESC semantics are implemented and regression-tested, but target-hardware
  audit of ESC-driven startup transitions and all later ESC/navigation
  consequences is deferred.
- Backlight behavior, wake coupling, and activity policy are deferred.
- TTL and RATE remain controlled dummy values; live acquisition, RATE windows,
  zero-flow policy, and pulse-loss behavior are deferred.
- RTC validity, persistence, retained-state recovery, and first-programming
  behavior are deferred until required by the next selected slice.
- Retry and recovery after LCD I/O failure, invalid-value presentation,
  adaptive refresh, complete navigation, configuration, alarms, and Phase 6B
  remain outside this cut.
- The LCD authority confirms both 14-segment character electrode sets and
  operational character rendering, while exact A-to-N naming remains a recorded
  technical-source gap.

## Next Recommended Cut

Define the smallest Phase 7 acquisition slice needed to replace the provisional
TTL/RATE snapshot with runtime-owned live values. Add RTC or persistence only
when that selected slice has a concrete dependency on them; Phase 6B follows
after the required runtime data is available.
