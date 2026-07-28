# Working Context: Phase 8 Minimum Measurement User Interface

## Objective And Status

Execute Phase 8 as one coherent workstream that adds a complete traversable
user-menu skeleton with two live measurement screens and three inert
placeholders, without implementing the deferred functions behind those
placeholders.

Status:
- Phase 7 is completed and human-accepted;
- Phase 8-0A, evidence and route foundation, is completed and human-accepted;
- Phase 8A, five-screen UI contract, is completed and human-accepted;
- Phase 8B, pure UI and mechanical rename, is the single active slice and its
  implementation is explicitly authorized;
- Phase 8C, owner-loop, external-input, and reset integration, is the single
  next gated slice;
- no CubeMX, generated, protected-file, hardware-integration, or Phase 8C
  change is authorized by this context.

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
isolated Phase 8B rename establishes `fmc_ui`. The current `fmc_input.h`
release-trigger and runtime-consumer wording describes the implementation
before Phase 8 integration; slice 8C must reconcile it with the accepted
press-edge producer and direct UI route.

## Active Slice: Phase 8B Pure UI And Mechanical Rename

### Boundary

Implement only the RTOS- and hardware-neutral UI boundary accepted in 8A.
Begin with an isolated mechanical rename from `fmc_presentation` to `fmc_ui`,
including matching files, public symbols, types, and macros, with no
compatibility aliases or behavior change. Canonical builds and the current
regression must pass before adding new UI behavior.

After that rename evidence passes, extend the pure module with startup plus the
five user states, bounded mechanical navigation, cyclic EXT_1 navigation,
semantic frames, right-aligned `"    OFF"` placeholders, logical user-menu
POINT state, and the accepted `fmc_ui_request_t` request contract.

### Deliverables

- rename `fmc_presentation.*` and its public namespace to `fmc_ui.*` without
  compatibility aliases;
- run canonical builds and current regression after the isolated rename;
- implement the five-state user-menu behavior and complete accepted input
  consequences in the pure UI;
- compose exact live and placeholder semantic frames, including POINT;
- return only the accepted Phase 8 NONE or RESET_ACM request kinds through
  `fmc_ui_request_t`;
- add deterministic RTOS-free regression for states, transitions, frames,
  requests, no-ops, failures, and invalid arguments;
- rerun canonical builds after the behavioral addition.

### Out Of Scope

- app owner-loop integration or direct runtime reset execution;
- changing `fmc_input.h`, `fmc_runtime`, or recognizer routing;
- configuring PD3, PD4, or PE0;
- implementing external-button debounce, ACM reset routing, POINT, or
  backlight in `product/main`;
- ThreadX, HAL, GPIO, EXTI, LCD mapping, or timer dependencies inside `fmc_ui`;
- implementing printing, logged-data transfer, Bluetooth, RTC/date-time,
  temperature, alarms, configuration, resolution editing, or TTL reset;
- performing the cross-cutting `ROBUST-1` runtime failure audit;
- target hardware validation.

### Closure Criteria

Phase 8B closes only when:
- the isolated rename passes canonical builds and the preexisting regression
  before behavioral work is added;
- the pure UI implements startup plus all five user states without RTOS or
  hardware dependencies;
- every accepted input consequence or no-op, semantic frame, POINT decision,
  and request is covered deterministically;
- invalid arguments and presentation failures preserve the accepted module
  contract;
- final canonical builds and regression pass;
- the human reviews the evidence and authorizes Phase 8C.

## Next Gated Slice: Phase 8C Owner-Loop, External Input, And Reset Integration

Phase 8C remains blocked behind both accepted 8B evidence and the separate
human CubeMX/regeneration gate recorded in the roadmap.
