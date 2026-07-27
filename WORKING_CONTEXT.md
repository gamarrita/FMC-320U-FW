# Working Context: Phase 8 Minimum Measurement User Interface

## Objective And Status

Execute Phase 8 as one coherent workstream that adds a complete traversable
user-menu skeleton with two live measurement screens and three inert
placeholders, without implementing the deferred functions behind those
placeholders.

Status:
- Phase 7 is completed and human-accepted;
- Phase 8-0A, evidence and route foundation, is completed and human-accepted;
- Phase 8A, five-screen UI contract, is the single active slice and is
  documentation-only;
- Phase 8B, pure UI and mechanical rename, is the single next gated slice;
- no Phase 8 source, test, CubeMX, generated, or protected-file change is
  authorized by this context.

## Authorities And Evidence

- durable Phase 8 strategy, route, dependencies, accepted architecture,
  hardware gates, risks, approvals, and exit evidence:
  `docs/roadmaps/fmc_refactoring.md`;
- visible screens, exact content, navigation, and input consequences:
  `docs/product/fmc/user_interface.md`;
- product obligations: `docs/product/fmc/requirements.md`;
- refresh, reset boundary, input debounce, POINT, and backlight timing:
  `docs/product/fmc/behavior.md`;
- physical LCD facts: `docs/specs/lcd/lcd_true_source.yaml`;
- current implemented product boundaries: `src/product/fmc/fmc_input.h`,
  `src/product/fmc/fmc_runtime.h`, `src/product/fmc/fmc_service.h`, and
  `src/product/fmc/fmc_presentation.h`;
- current product-app ownership: `src/apps/product/main/README.md`;
- bounded legacy navigation evidence:
  `legacy/analysis/fmc_user_navigation.md`;
- frozen legacy evidence:
  `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- human-reviewed legacy disposition:
  `docs/workflow/fmc_legacy_coverage.md`.

Legacy documents and code are evidence, not current product authority. The
current `fmc_presentation` header remains implementation authority until the
isolated Phase 8B rename establishes `fmc_ui`.

## Active Slice: Phase 8A Five-Screen UI Contract

### Boundary

Consolidate the sequentially reviewed Phase 8 decisions at their proper
authorities. The accepted user-menu order is:

```text
TTL_RATE -> ACM_RATE -> PRINT -> LOG_DOWNLOAD -> DATE_TIME
```

TTL/RATE and ACM/RATE are live screens. PRINT, LOG_DOWNLOAD, and DATE_TIME are
static inert placeholders. The active slice records complete navigation,
direct ACM reset, external-button semantics, periodic presentation, transverse
POINT, safe backlight behavior, later implementation ownership, and CubeMX
prerequisites.

### Deliverables

- update `docs/product/fmc/user_interface.md` with exact screen composition and
  the complete startup/user-menu input table;
- update `docs/product/fmc/behavior.md` with temporal reset, refresh, external
  debounce, POINT, and backlight behavior;
- update `docs/product/fmc/requirements.md` with concise normative obligations;
- record reviewed legacy dispositions in
  `docs/workflow/fmc_legacy_coverage.md`;
- reconcile the Phase 8 roadmap and legacy analysis with the accepted
  decisions;
- keep Phase 8B as the one next gated slice.

### Out Of Scope

- source, header, test, build-system, CubeMX, generated-code, or protected-file
  changes;
- creating `fmc_ui`, renaming `fmc_presentation`, or changing runtime/app
  ownership;
- configuring PD3, PD4, or PE0;
- implementing external-button debounce, ACM reset routing, POINT, or
  backlight;
- implementing printing, logged-data transfer, Bluetooth, RTC/date-time,
  temperature, alarms, configuration, resolution editing, or TTL reset;
- performing the cross-cutting `ROBUST-1` runtime failure audit;
- canonical build or target validation.

### Closure Criteria

Phase 8A closes only when:
- product authorities agree on the five screens, exact placeholder content,
  full transition/no-op table, direct ACM reset, refresh, POINT, and backlight;
- the legacy coverage register records the reviewed dispositions without
  treating evidence as authority;
- the roadmap defines the accepted architecture and hardware gate without
  duplicating detailed product contracts;
- documentation links, formatting, and frozen-evidence integrity checks pass;
- the human reviews the consolidated contract and authorizes Phase 8B.

## Next Gated Slice: Phase 8B Pure UI And Mechanical Rename

Phase 8B begins only after Phase 8A is accepted and implementation is
explicitly authorized.

Its first bounded change is an isolated mechanical rename from
`fmc_presentation` to `fmc_ui`, including matching files, public symbols,
types, and macros, with no compatibility aliases. Canonical builds and current
regression must pass before new behavior is added.

The remainder of 8B stays RTOS- and hardware-neutral. It adds the five UI
states, bounded mechanical and cyclic EXT_1 navigation, exact semantic frames,
static placeholders, logical POINT state, and the sole explicit request
`FMC_UI_REQUEST_RESET_ACM`. It does not connect GPIO, EXTI, ThreadX timers,
runtime reset, backlight, or CubeMX configuration.

Phase 8C remains blocked behind both accepted 8B evidence and the separate
human CubeMX/regeneration gate recorded in the roadmap.
