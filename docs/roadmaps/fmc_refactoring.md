# FMC Refactoring Roadmap

## Purpose

Define the durable refactor roadmap for the FMC-320 firmware. This roadmap
describes phases, dependencies, decision gates, risks, and exit criteria. It is
not the active work tracker and does not define module APIs.

Accepted product behavior comes from the reviewed documentation under
`docs/product/fmc/`. The frozen extraction at
`legacy/derived/fmc/use_cases.extraction-v1.yaml` is derived legacy evidence,
not a current product contract. Public contracts for implemented modules belong
in their headers.

## Program Sequence From The Current Point

The remaining program follows this dependency order:

1. Reframe documentation and establish the human-agent-repository workflow.
2. Build the FMC product-documentation backbone.
3. Give the principal product functions broad, superficial coverage.
4. Incorporate useful requirements and use cases incrementally, with explicit
   review before they become authoritative.
5. Deepen the next visible programming slice.
6. Implement Phase 6A: the bounded initial presentation slice.
7. Add essential acquisition through the dedicated incremental acquisition
   route.
8. Add the minimum measurement user screens and their navigation.
9. Add RTC/calendar behavior with its required user and configuration screens
   and navigation.
10. Add the minimum measurement configuration screens and their navigation.
11. Retain the approved high-change state in Backup SRAM.
12. Persist the approved low-change configuration in Flash.
13. Add temperature measurement as one vertical capability, including only the
    UI and configuration it requires.
14. Add Bluetooth as one vertical capability, including only the UI and
    configuration it requires.
15. Add ticket printing as one vertical capability after its Bluetooth
    and RTC dependencies.
16. Perform integral product, power, and release validation.

Each phase depends only on the dependencies stated in its section. The numeric
order is the current delivery route, not permission to infer product behavior.
Independent vertical capabilities may be deferred without collapsing their UI
into another phase; ticket printing retains its explicit Bluetooth dependency.

This is not a requirement to complete all product documentation before
programming: breadth comes first, then depth follows the next slice.

Phases 8 and 10 are separate because operational navigation and measurement
configuration navigation have different behavior, authorization, validation,
and test boundaries. Phase 9 is a complete RTC/calendar vertical slice, including
its own user and configuration screens. Phases 13 through 15 likewise do not
defer their necessary UI to a later generic screen phase.

## Phase 1: Model And Pure Calculations

Objective:
- isolate canonical FMC model state and pure unit, volume, rate, and formatting
  calculations.

Dependencies:
- reviewed product units, totals, calibration, and rate semantics for the
  selected slice; the frozen legacy extraction may supply evidence;
- legacy source only as evidence.

Decision gates:
- numeric range and invalid-value behavior when the current specification does
  not define operator-visible output.

Risks:
- accidentally preserving legacy representation rather than current product
  behavior;
- mixing presentation labels into deeper model logic.

Exit criteria:
- pure contracts are documented in headers;
- regression tests cover valid, invalid, boundary, and unsupported cases.

## Phase 2: Live State And `fmc_service`

Objective:
- define the owner of live FMC state and the controlled operations that mutate
  or snapshot it.

Dependencies:
- Phase 1 contracts;
- reviewed reset policy and totalization requirements for the selected slice.

Decision gates:
- whether service state should remain directly inspectable for tests/restores
  or become opaque after persistence is designed.

Risks:
- letting acquisition, UI, or persistence mutate model state directly;
- adding RTOS synchronization before ownership is clear.

Exit criteria:
- service owns one live model instance;
- pulse deltas and resets preserve service invariants;
- snapshots are stable and covered by tests.

## Phase 3: `fmc_runtime` Contract

Objective:
- establish the RTOS-neutral runtime event boundary over `fmc_service`.

Dependencies:
- Phase 2 service contract.

Decision gates:
- event payload representation;
- invalid-event behavior;
- snapshot ownership and update notification semantics.

Risks:
- importing scheduler, queue, HAL, BSP, or LCD types into product contracts;
- encoding temporary UI behavior as permanent runtime API.

Exit criteria:
- runtime accepts product events without platform types;
- runtime preserves service ownership and snapshot access;
- tests cover dispatch and error paths.

## Phase 4: Semantic Input Architecture

Objective:
- preserve product input identity before assigning screen-specific
  consequences.

Dependencies:
- reviewed current-product input requirements and implemented input contracts;
- board key mapping in BSP;
- runtime event contract.

Decision gates:
- final location of app-specific composition code after a second consumer
  exists;
- handling of external buttons alongside mechanical keyboard keys.

Risks:
- losing key identity by converting input directly into presentation updates;
- coupling product input contracts to BSP, GPIO, EXTI, HAL, RTOS, or timer
  types.

Exit criteria:
- product input types include mechanical keys, external buttons, SHORT, and
  LONG;
- short/long recognition can produce runtime input without changing runtime
  input interfaces;
- pure tests prove key/action identity is preserved.

## Phase 5: ThreadX, ISR Delivery, Timers, And Low Power

Objective:
- define and demonstrate ThreadX runtime ownership, serialized ISR-to-thread
  delivery, timer ownership, and low-power interaction.

Runtime direction:
- ThreadX is the only active firmware runtime from this phase onward.
- The last bare-metal firmware state is a historical comparison baseline, not a
  second maintained architecture.
- Selectable apps continue through a ThreadX bootstrap harness. For
  `product/main`, the existing `FM_APP` thread runs the `FM_MAIN_Main()` owner
  loop directly.

Completed slice:
- use the existing `FM_APP` ThreadX thread as the only owner of `fmc_runtime`;
- deliver mechanical keyboard, key-hold timeout, and provisional periodic
  refresh events to that owner through one ThreadX queue;
- use app-level event payloads at producer-to-owner boundaries;
- use an initial queue depth of 8 events;
- treat owner queue overflow as abnormal and make it explicit;
- recognize mechanical-key `SHORT` and `LONG` in `product/main` using
  hardware-confirmed RISING/FALLING edges and a one-shot 3 second timer;
- hardware-smoke validate DOWN, UP, ENTER, and ESC for SHORT, LONG, and no
  duplicate SHORT after LONG;
- defer low-power policy, presentation ownership, wake/backlight policy,
  debounce, menu consequences, and external buttons.

Dependencies:
- semantic input contract;
- runtime contract;
- CubeMX-generated ThreadX and low-power-support configuration.

Decision gates:
- owner loop startup sequencing in the existing `FM_APP` thread;
- exact ThreadX queue storage ownership and overflow action;
- wake source and low-power ownership;
- presentation/backlight activity ownership.

Risks:
- enabling ThreadX only for long press and accidentally redesigning startup,
  low-power, queues, timers, and ownership at once;
- continuing too long on accidental bare-metal assumptions.
- letting a historical bare-metal baseline become an accidental second product
  line.

Exit criteria:
- minimal runtime runs under the selected ThreadX ownership model;
- ISR path does bounded work only;
- delivery to runtime is serialized and testable.

Parallel technical follow-up `LP-1` — ThreadX/STOP2 final-tick profile:
- State: open, non-blocking, and not part of an active workstream.
- Ownership: the Phase 5 ThreadX/timer/low-power technical domain. This does
  not reopen completed Phase 5 or make the issue a Phase 8 dependency.
- Phase 7 combined-current characterization exposed a non-blocking
  `fm_port_threadx_idle` timing-profile issue. With silent `product/main` and
  the normal one-second periodic deadline, PPK2 measured approximately
  `33.40 uA` average at `0 Hz` and `39.08 uA` average at `100 Hz`. The `0 Hz`
  profile showed one active cluster per interval; at `100 Hz`, some intervals
  showed two distinct active peaks separated by a real return to the
  approximately `20 uA` low-current level.
- Evidence points to the boundary between the LPTIM1 elapsed-time conversion,
  ThreadX bulk timer adjustment, and the final SysTick, but no root cause or
  correction is accepted. A one-tick LPTIM extension increased the number of
  peaks; a bounded attempt to combine that extension with a pending final
  SysTick retained the original profile. Both experiments were removed.
- Legacy's integer `20` conversion for the actual `20.48` LPTIM ticks per
  ThreadX tick remains comparison evidence only; its timing bias is not an
  accepted workaround.
- Entry: the human explicitly opens a separate low-power-port workstream,
  preferably on a parallel branch, without changing the active product phase.
- Exit: correlate LPTIM1 compare, elapsed-tick adjustment, final SysTick, task
  wake, and PPK2 evidence; then explicitly accept either the existing profile
  or one verified correction. Any correction must preserve ThreadX time,
  canonical builds, acquisition regressions, and the `0 Hz`/`100 Hz` physical
  behavior before `LP-1` closes.
- `LP-1` does not change Phase 7 acquisition, totalization, frequency, RATE, or
  presentation contracts and does not block Phase 8.

## Phase 6A: Initial Presentation Slice

Status:
- completed and human-accepted on target hardware.

Objective:
- validate the first bounded path from an FMC runtime snapshot to controlled,
  user-visible LCD output.

Dependencies:
- the product-documentation backbone and broad functional coverage;
- reviewed deep documentation for the selected visible slice;
- stable runtime snapshots;
- `docs/specs/lcd/lcd_true_source.yaml` as the technical LCD authority;
- existing semantic LCD and driver contracts;
- ThreadX/timer ownership only where the selected startup timing requires it.

Included scope:
- all-segments startup screen;
- firmware version screen;
- steady TTL/RATE screen;
- projection of an FMC snapshot into semantic LCD elements;
- a validated presentation-to-driver adapter;
- pure formatting tests;
- hardware bring-up with controlled values.

Decision gates:
- startup sequence ownership;
- numeric overflow/invalid display;
- exact reviewed content and timing for the three selected states;
- adapter ownership at the product/BSP boundary.

Risks:
- treating behavior in the frozen legacy extraction as approved requirements;
- freezing `fmc_presentation` APIs before the slice is active.

Exit criteria:
- the three selected states are traceable to reviewed slice documentation;
- snapshot fields project into semantic LCD elements without exposing LCD
  mapping details to the FMC product model;
- the presentation-to-driver adapter is validated;
- pure tests cover selected formatting decisions;
- controlled-value hardware bring-up is recorded;
- public module contracts live in headers;
- no complete UI state machine, complete navigation, configuration screens, or
  exact legacy reproduction has been introduced.

Completion:
- all exit criteria above are satisfied by the Phase 6A presentation,
  product-main LCD adapter, focused regression coverage, canonical builds, and
  controlled-value target validation.

## Phase 7: Essential Acquisition

Status:
- completed and human-accepted on target hardware;
- the separate ThreadX/STOP2 timing-profile follow-up is recorded as `LP-1`
  under the Phase 5 technical domain;
- edge-coherent LPTIM3 frequency capture is recorded as the parallel follow-up
  `FREQ-1`; neither follow-up is a remaining Phase 7 gate.

Objective:
- replace provisional Phase 6A TTL/RATE inputs through short, independently
  reviewable acquisition slices;
- connect accepted pulse deltas and validated RATE observations through runtime
  ownership without coupling acquisition to product totals or presentation.

Responsibility boundaries:
- physical pulse accumulation;
- counter observation and pulse-delta formation;
- ACM/TTL totalization;
- physical frequency observation;
- pulse/time observation quality;
- pure RATE mathematics;
- runtime and presentation integration.

Dependencies:
- completed Phase 6A runtime/presentation path;
- current `fmc_runtime`, `fmc_service`, and pure `fmc_rate` ownership;
- reviewed product decisions for the bounded slice;
- human-approved hardware and CubeMX changes before each hardware bring-up;
- target equipment capable of generating and independently counting pulses and
  measuring current where low-power acceptance is in scope.

Decision gates:
- supported sensor signal, frequency, pulse-width, and low-power envelope;
- pulse-delta acceptance, modulo-wrap precondition, and reset boundary;
- LPTIM4 pin, clock, filtering, autonomous-mode, and CubeMX path;
- frequency observation semantics, accuracy, latency, and quality states;
- capture, DMA, interrupt, polling, or other frequency technique;
- acceptance or rejection of any legacy workaround after target evidence.

Risks:
- designing around an unconfirmed silicon defect;
- coupling totalization correctness to frequency availability;
- losing pulses across sleep or context transitions;
- hiding counter wrap, stale data, or invalid observations as zero flow;
- expanding one bring-up into the complete acquisition subsystem.

Correct-first baseline:
- legacy firmware and field reports remain evidence, not implementation
  authority;
- documented correct STM32U5 use is exercised without assuming the reported
  historical LPTIM behavior is a silicon defect;
- pulse accumulation is completed before frequency observation can endanger
  totalization;
- physical observation remains separate from RATE mathematics;
- DMA, interrupts, polling, autonomous peripherals, and other candidates are
  evaluated from required behavior and measured energy;
- a workaround is considered only after a minimal correct technique fails or
  cannot meet an approved constraint on target.

### Incremental Acquisition Route

`7-0A -> 7-0B -> 7A -> 7B1 -> 7B2 -> 7B3 -> 7C -> 7D -> 7E1 -> 7E2 -> [7E3] -> 7F -> 7G`

`7E3` is conditional. It opens only when accepted target evidence from 7E1 or
7E2 demonstrates an unmet approved constraint or a reproducible failure.

| Slice | Bounded result | Entry or decision gate | Exit evidence |
|---|---|---|---|
| 7-0A: Evidence and hypothesis formulation | Reconstruct legacy acquisition and classify the reported STM32U575 behavior | Legacy sources and identifiable public technical evidence are available | `legacy/analysis/fmc_acquisition.md` records provenance, uncertainties, hypotheses, and the correct-first baseline without selecting hardware |
| 7-0B: Foundation and route | Establish document ownership, incremental route, approval gates, and the next slice | 7-0A evidence is reviewable | Repository documents agree on the route and one active slice; no product or hardware decision is inferred |
| 7A: Pulse-accumulation contract | Define the bounded LPTIM4 counter observation accepted as a pulse delta | Human decisions on counter assumptions, selected low-power states, observation cadence, and loss policy | Product owners contain approved outcomes and `docs/specs/fmc/acquisition.md` defines delta, modulo wrap, reset, numeric, ownership, and acceptance semantics |
| 7B1: Pulse-counter bring-up in Run | Exercise the minimum documented LPTIM4 counter technique without Stop2 or RATE | Human-approved pin, clock, filter, CubeMX change, and Run signal matrix | Target evidence demonstrates exact raw counting and stable observation at approved Run limits |
| 7B2: Pulse-counter bring-up across Stop2 and wrap | Exercise the same counter path through low power and rollover | 7B1 accepted; Stop2 matrix and current-measurement method approved | Instrumented results demonstrate counting across Stop2 and wrap; separate silent runs establish current |
| 7B3: Counter-observation and pulse-delta module | Convert trusted counter observations into bounded pulse deltas | 7B1 and 7B2 hardware behavior accepted | Regression vectors cover first sample, zero, normal delta, modulo wrap, delayed observation, and reset without RATE or totals |
| 7C: Totalization/runtime integration | Deliver accepted deltas to ACM and TTL exactly once through runtime/service | 7B3 accepted; acquisition-to-runtime boundary reviewed | Regression and target evidence show no loss, duplication, or acquisition ownership of product totals |
| 7D: Frequency-observation contract | Define a pulse/time observation with explicit quality | Accumulation is stable enough that frequency work cannot endanger totals | Specification defines time-window ownership, range, accuracy, latency, zero, absent, stale, invalid, and RATE handoff |
| 7E1: Unified functional frequency bring-up | Characterize the minimum documented frequency technique while normal ThreadX idle transparently permits Stop2 between observations | Human-approved technique, CubeMX path, signal matrix, and accuracy target | Target results cover the approved frequency range, elapsed time, wrap, quality reporting, and the normal wake/Stop2 observation cycle |
| 7E2: Silent frequency current characterization | Characterize the current of the already accepted unified technique without adding another implementation or functional signal matrix | 7E1 accepted; silent current-measurement method approved | A separate silent run establishes current for the same firmware path |
| 7E3: Conditional technique comparison | Compare a demonstrated failure with another documented technique or workaround | 7E1 or 7E2 records a minimal failure and the human approves comparison | Evidence selects, rejects, or defers alternatives without changing pulse-total correctness |
| 7F: RATE integration | Feed validated pulse/time observations into pure RATE calculation with explicit quality | 7D and required 7E evidence accepted; 7E3 closed if entered | Math vectors, boundaries, and runtime tests agree on units, elapsed time, invalid input, and zero/stale distinctions |
| 7G: Combined live integration | Combine accepted counter and frequency paths and replace provisional TTL/RATE inputs | 7C and 7F accepted; visible invalid/zero behavior approved | Combined bring-up, canonical builds, regression, target pulse accuracy, presentation, and current validation pass |

Phase 7A is a documentation-only, hardware-configuration-independent contract
slice. Phase 7B1 is the first
possible acquisition implementation and cannot start before approval of its
hardware and CubeMX gates.

### Human-Agent Bring-Up Protocol

Bring-ups use the existing debug UART as a transmit-only observation channel:

- the human flashes the board, controls the signal generator, and performs
  physical current measurement;
- the agent may monitor UART output, analyze evidence, and direct the next
  approved signal case;
- UART RX is not introduced;
- no UART transmission occurs while the MCU is in Stop2;
- bounded observations collected during Stop2 are emitted after return to Run.

Correctness and power are separate runs. Instrumented UART runs support
functional diagnosis; silent runs provide accepted current-consumption
evidence.

Human approval is required before:

- promoting acquisition behavior into product requirements;
- selecting a signal envelope, observation latency, low-power guarantee,
  pulse-loss policy, quality state, or visible failure behavior;
- selecting an LPTIM4 pin, clock, filter, detailed counter mode, interrupt, DMA
  path, autonomous mode, or CubeMX change;
- using the suspected STM32U575 behavior as a current design constraint;
- comparing or adopting the legacy wake-up workaround;
- beginning an implementation slice.

Exit criteria:
- the required acquisition slices close with their specified evidence, or are
  explicitly deferred while provisional inputs remain identified;
- acquisition updates service state only through the reviewed runtime boundary;
- pulse totals do not depend on frequency-observation availability;
- RATE receives validated pulse/time observations with explicit quality;
- selected low-power behavior and current consumption are accepted on target;
- any adopted workaround is traceable to a reproduced failure and explicit
  human decision;
- tests and bring-ups cover selected normal, boundary, low-power, and recovery
  paths.

Completion:
- all planned slices through 7G are closed; conditional 7E3 was not entered;
- LPTIM4 pulse accumulation, independent pulse-delta and frequency observers,
  runtime-owned ACM/TTL and RATE updates, and live TTL/RATE presentation are
  integrated in `product/main`;
- deterministic regression, canonical `tests/regression` and `product/main`
  builds, the physical `100 -> 0 -> 100 Hz` matrix, LCD agreement, pulse
  conservation, recovery without reset, and silent PPK2 characterization were
  human-accepted;
- no experimental ThreadX idle-port workaround remains in the Phase 7
  implementation;
- Phase 8 is the next roadmap phase.

Parallel technical follow-up `FREQ-1` — edge-coherent LPTIM3 measurement:
- State: deferred, open, important, non-blocking, and not part of an active
  workstream.
- Ownership: the Phase 7 physical-frequency technical domain. This does not
  reopen completed Phase 7 or make the work a Phase 8 dependency.
- Current baseline: the accepted first realization estimates frequency from
  cumulative LPTIM4 pulses over an approximately one-second LPTIM3 timestamp
  window. It remains valid and implemented.
- Objective: evaluate and implement the pending edge-to-edge technique in
  which the primary pulse remains the asynchronous LPTIM4 count source and
  also drives an LPTIM3 capture, allowing the time observation to be aligned
  to physical input edges.
- Entry gates: explicitly open a separate frequency-acquisition workstream;
  decide whether edge capture complements or replaces the current window
  producer; approve the required CubeMX pin/channel, interrupt, DMA, autonomous
  mode, and Stop2 path before changing hardware configuration.
- Contract gates: edge-baseline startup, missing-edge and zero-flow behavior,
  supported period/frequency range, timestamp wrap, capture loss or overrun,
  quality mapping, recovery, and interaction with the existing one-second
  observer must be decided before implementation. Legacy `0.1 Hz` behavior is
  not restored automatically.
- Architecture boundary: LPTIM4 pulse accumulation and the independent
  `pulse_delta` baseline must remain correct even when capture is unavailable,
  late, invalid, or asleep. Edge capture may supply frequency timing; it must
  not become the owner of ACM/TTL.
- Exit: deterministic capture/quality regressions plus target validation in
  Run and Stop2 demonstrate edge-coherent timing, the approved range, missing
  edges, wrap/recovery, no pulse loss or duplication, and accepted current.
  Legacy capture and the reported STM32U575 behavior remain evidence until
  reproduced through the selected documented technique.

Parallel technical follow-up `ROBUST-1` — runtime failure model audit:
- State: deferred, open, cross-cutting, non-blocking, and not part of an active
  workstream.
- Ownership: product-wide runtime robustness. This does not reopen completed
  phases, replace the active working context, or make the audit a Phase 8
  dependency.
- Intent: favor prevention and deterministic operation over speculative
  runtime recovery. A failure becomes part of a runtime contract only when it
  is observable, has a relevant consequence, and has an effective response.
- Current baseline: public contracts and application composition already
  contain argument, state, HAL, transport, and fatal-path checks, but the
  repository does not yet record a systematic per-module decision about which
  failures are detectable, which responses are useful, or when retry,
  reinitialization, fail-stop, or reset is justified.
- Entry gate: explicitly open a separate robustness workstream. Do not remove
  checks or change runtime behavior as incidental work in another phase.
- Audit method: for each in-scope operation record the failure hypothesis,
  detection mechanism and limits, consequence, natural retry opportunity,
  selected response, response owner, and verification evidence. Distinguish
  development contract defects, initialization failures, transient observable
  failures, undetectable physical corruption, and persistent integrity
  failures.
- Decision gates: retain development-facing validation where it prevents or
  exposes defects; remove false recovery semantics and checks that provide no
  useful safety only after their callers and consequences are reviewed;
  require evidence that retry, peripheral reinitialization, or reset can
  restore a known-good state without losing or duplicating measurement state.
- Documentation boundary: stable cross-cutting policy belongs in `STYLE.md`;
  accepted operation-specific behavior belongs in the owning public header or
  product contract. The audit must not duplicate those contracts in the
  roadmap.
- Phase 8 boundary: Phase 8 introduces no new LCD retry, rollback, readback, or
  recovery policy. It may preserve existing error propagation until
  `ROBUST-1` reviews it, but failure handling is not expanded into a user-menu
  product requirement.
- Exit: reviewed modules have an explicit failure classification and owner;
  approved contract and policy changes are documented at their authorities;
  focused regression and required target evidence demonstrate every changed
  response; obsolete checks and fatal paths are removed only where that
  evidence supports removal.

## Phase 8: Minimum Measurement User Screens And Navigation

Objective:
- complete a traversable five-screen operator path in normal `product/main`;
- provide live TTL/RATE and ACM/RATE behavior now;
- reserve PRINT, LOG_DOWNLOAD, and DATE_TIME positions with inert placeholders
  until their functional phases.

Responsibility boundaries:
- `fmc_ui` owns startup and user-menu state, semantic input consequences,
  semantic frames, POINT logical state, and explicit UI requests;
- product-main remains the single serialized owner, obtains fresh coherent
  runtime snapshots, consumes UI requests exactly once, owns ThreadX timers,
  and coordinates immediate and periodic presentation;
- `fmc_runtime` continues to own measurement, RATE, totals, and the ACM-reset
  primitive, but no longer retains passive menu input;
- the LCD adapter maps semantic frames only;
- existing board GPIO and keyboard facades expose physical behavior without
  leaking HAL, EXTI, active-low polarity, or ThreadX into product modules;
- `fm_main_ext_button` makes RTOS-neutral per-button debounce decisions;
- `fm_main_backlight` owns safe activation and expiry coordination in the app.

Dependencies:
- Phase 7 live TTL/RATE acquisition;
- completed Phase 6A presentation path;
- semantic input and runtime-owner contracts;
- coherent runtime snapshots containing ACM, TTL, and RATE;
- `docs/specs/lcd/lcd_true_source.yaml` for physical LCD capabilities and
  mapping facts;
- before slice 8C, a human-applied CubeMX regeneration that configures PD3 and
  PD4 as active-low EXTI inputs with internal pull-ups and both edges, and PE0
  as the active-low LCD backlight output with the approved safe initial state.

Accepted product decisions:
- user-menu order is TTL/RATE, ACM/RATE, PRINT, LOG_DOWNLOAD, DATE_TIME;
  startup enters TTL/RATE;
- mechanical SHORT DOWN/UP traverse boundedly; EXT_1 SHORT traverses forward
  cyclically; the product UI contract records every other action or no-op;
- PRINT, LOG_DOWNLOAD, and DATE_TIME are visible static placeholders with no
  workflows, peripherals, configuration, RTC data, or timeouts;
- temperature, alarms, configuration, resolution shortcuts, printing,
  logged-data transfer, and date/time functionality remain outside Phase 8;
- LONG ENTER and EXT_2 SHORT reset ACM directly and only on ACM/RATE; there is
  no confirmation state; TTL reset remains privileged future work;
- ACM/RATE reuses the accepted TTL/RATE numeric, RATE-quality, unit, time-base,
  and overflow rules, with ACM in the upper row;
- every screen is presented at most once per accepted one-second cycle; live
  views are also immediate on entry and ACM/RATE is immediate after reset;
- POINT is a transverse accepted-pulse-observation witness: nonzero delta
  toggles once, zero delta turns it off;
- every valid physical press activates and restarts a fixed ten-second
  backlight interval without consuming the action;
- EXT_1 and EXT_2 use a press-edge SHORT with independent stable-release
  rearm; they have no LONG or repeat behavior.

Accepted architecture decisions:
- first rename `fmc_presentation` to `fmc_ui` as an isolated mechanical change,
  with matching public names and no compatibility aliases;
- `fmc_ui` later owns startup plus the five user states and returns only
  `FMC_UI_REQUEST_NONE` or `FMC_UI_REQUEST_RESET_ACM`;
- semantic/BSP ACM indicator names remain positional as `ACM_TOP` and
  `ACM_BOTTOM`; legacy `ACM_1` and `ACM_2` names are not propagated into the
  new UI contract, and no global rule disables either physical indicator;
- `fmc_ui` does not call runtime, LCD, ThreadX, HAL, GPIO, or timers;
- the input handler returns `fm_status_t`, initializes its output request to
  `FMC_UI_REQUEST_NONE`, and never invokes callbacks or runtime operations;
- product-main consumes reset requests once, executes the runtime reset, takes
  a fresh snapshot, and presents the updated ACM/RATE frame;
- recognizers deliver `fmc_input_event_t` to product-main, which routes menu
  input directly to `fmc_ui`; Phase 8 removes the passive runtime input event
  and retained last-input fields;
- external-button debounce remains outside product UI and is driven by
  product-main using two independent one-shot timers and GPIO reads;
- the existing `fm_board_keyboard` facade is extended for EXT_1 and EXT_2 and
  uses the existing EXTI route; no new external-button BSP is introduced;
- `fm_main_ext_button` has no ThreadX, HAL, queue, backlight, UI, or runtime
  dependency. Per button it owns armed/release-candidate decisions and returns
  semantic SHORT plus timer start, restart, or cancel instructions;
- product-main owns the two ThreadX debounce timers, performs required GPIO
  reads, and routes each timer callback as a serialized timeout event back to
  the helper;
- `fm_port_gpio` and `fm_board` expose backlight On/Off operations that hide
  active-low polarity. They are `void` operations with no fabricated status,
  readback, or software state, and `FM_BOARD_Init()` reasserts off;
- the app-facing backlight controller exposes initialization and activation
  request rather than requiring callers to balance independent On/Off calls;
- backlight activation commits a valid expiry before the active-low output is
  turned on. Its ThreadX one-shot callback turns the output off directly
  instead of depending on a queue that may be full; deadline/generation state
  prevents a stale callback from defeating a newer request. Timer create or
  rearm failure leaves the nonessential backlight off/disabled, reports the
  failure once, and keeps measurement running;
- Phase 8 adds no LCD readback, retry, rollback, reinitialization, or
  transactional navigation layer. Cross-cutting failure policy remains in
  deferred follow-up `ROBUST-1`.

Hardware configuration gate before 8C:
- human configures CubeMX PD3 as `EXT_BUTTON_1` and PD4 as `EXT_BUTTON_2`, EXTI
  rising/falling, internal pull-up;
- human configures PE0 as `LCD_BACKLIGHT`, GPIO output push-pull, no pull, low
  speed, initial high/off;
- generated changes are reviewed before repository-facing wrappers or app
  integration proceed;
- `FM_BOARD_Init()` reasserts backlight off through the board facade.

Legacy evidence:
- the implemented legacy user path placed TTL/RATE before ACM/RATE;
- broader legacy menus also included date/time, printing, a Bluetooth-named
  download window, and optional temperature;
- `legacy/analysis/fmc_user_navigation.md` reconstructs the preserved flow and
  records evidence conflicts without promoting legacy behavior.

Legacy boundary:
- the current five-screen order and bounded mechanical navigation replace the
  incomplete and internally inconsistent legacy order;
- LOG_DOWNLOAD replaces the ambiguous Bluetooth screen name because it states
  operator purpose independently of transport;
- direct ACM reset through LONG ENTER and EXT_2 is deliberately retained, but
  only in the active ACM/RATE context;
- the current external-button trigger, debounce, POINT, backlight, refresh, and
  ownership rules replace timer-coupled legacy mechanisms;
- configuration entry, resolution shortcuts, alarm actions, real deferred
  workflows, and temperature remain evidence only.

Risks:
- copying the monolithic legacy `fm_user` or `fmx` ownership shape;
- coupling product navigation to LCD mapping, ThreadX, HAL, timers, GPIO, or
  reset primitives;
- allowing inert placeholders to start deferred workflows or fabricate data;
- losing, duplicating, or delaying accepted input while integrating EXTI
  bounce filtering with the owner queue;
- allowing a stale backlight expiry to defeat a newer activation request;
- resetting acquisition counters or baselines together with ACM;
- changing acquisition cadence, Stop2 behavior, or pulse conservation while
  adding presentation activity;
- mixing `ROBUST-1` cleanup into the functional slices.

### Incremental User-Interface Route

`8-0A -> 8A -> 8B -> 8C -> 8D -> 8E`

| Slice | Bounded result | Entry or decision gate | Exit evidence |
|---|---|---|---|
| 8-0A: Evidence and route foundation | Reconstruct relevant legacy navigation, expose conflicts, define Phase 8 ownership boundaries, and establish the incremental route | Phase 7 is closed; current product/UI authorities and preserved legacy sources are available | Human-accepted evidence analysis and route foundation |
| 8A: Five-screen UI contract | Consolidate the reviewed screen, navigation, input, reset, refresh, POINT, backlight, hardware-prerequisite, and architecture decisions without changing code | 8-0A is accepted and the sequential human review has closed the listed decisions | Product owners and the legacy coverage register agree; the complete transition table and implementation gates are reviewable |
| 8B: Pure UI and mechanical rename | First rename `fmc_presentation` to `fmc_ui` in isolation, verify it, then implement the RTOS-neutral five-state navigation, semantic frames, placeholder content, logical POINT state, and reset request | 8A is human-accepted and implementation is authorized | Canonical builds after the isolated rename; deterministic regression covers startup, five states, every input consequence, frames, POINT decisions, requests, failures, and invalid arguments without ThreadX or LCD hardware |
| 8C: Owner-loop, external input, and reset integration | Remove passive runtime input, connect UI to coherent snapshots, extend the existing board keyboard facade for EXT_1/EXT_2, add independent app debounce, and execute authorized ACM reset | 8B is accepted; human CubeMX gate is complete and generated diff reviewed | Regression and canonical builds show serialized input, five visible screens, bounded/cyclic navigation, bounce/hold/boot-held behavior, exactly-once reset, and live updates |
| 8D: Transverse POINT and safe backlight | Complete periodic POINT application and add the last functional slice: safe startup/input backlight activation with committed ten-second expiry | 8C is accepted; PE0 generated configuration and board path are available | Focused tests and target evidence cover POINT observations, startup and input activation, deadline restart, stale expiry, fail-off behavior, and no measurement/low-power regression |
| 8E: Combined target validation | Validate the complete Phase 8 operator path on target | Required slices 8A through 8D are accepted | Canonical builds, regression, and target evidence cover five-screen order and bounds, EXT_1 cycle, both ACM reset paths and no-ops, bounce/hold/boot-held inputs, POINT at zero/isolated/continuous flow, backlight startup/restart/expiry, and Run/Stop2 pulse conservation |

Phase 8-0A is closed and human-accepted. Contract consolidation belongs to the
documentation-only 8A slice. `WORKING_CONTEXT.md` owns its current active
status. No source, test, CubeMX, protected, or generated file change belongs to
8A.

Human approval is required before:
- beginning an implementation slice;
- applying or regenerating the accepted CubeMX hardware configuration;
- editing any protected or generated code outside allowed `USER CODE`
  boundaries.

Exit criteria:
- all five user-menu positions are visible and navigable in normal
  `product/main`;
- every in-scope semantic input has an explicit accepted consequence or no-op;
- ACM resets directly and only through LONG ENTER or EXT_2 while ACM/RATE is
  active; TTL and acquisition counter/baselines remain untouched;
- transitions, static placeholders, formatting, quality display, live updates,
  and reset refresh have focused regression and target validation;
- external buttons produce one press action despite bounce and require stable
  release before rearm;
- POINT and backlight behavior match their accepted temporal contracts;
- navigation remains independent from LCD physical mapping, ThreadX types,
  acquisition ownership, and persistence;
- configuration, alarms, temperature, and real placeholder workflows remain
  out of scope;
- acquisition and Run/Stop2 behavior show no regression.

## Phase 9: RTC, Calendar, And Associated Screens

Objective:
- add RTC/calendar as one complete vertical capability;
- include its user screen, configuration screen, and navigation in the same
  phase.

Dependencies:
- Phase 8 user navigation;
- approved time display and editing use cases;
- available CubeMX RTC configuration as the hardware authority.

Decision gates:
- RTC validity, initialization, format, edit, and recovery behavior;
- user-screen placement and return behavior;
- configuration authorization, save/cancel, and invalid-time feedback;
- backup-supply behavior before general Backup SRAM retention is introduced;
- which later reporting, logging, or communication functions may consume time.

Legacy evidence:
- legacy provides both a user date/time screen and date/time configuration;
- the legacy ticket formatter later consumes RTC date and time.

Exit criteria:
- valid and invalid time are distinguishable;
- approved date/time can be displayed and edited through reviewed navigation;
- formatting, calendar boundaries, save/cancel, reset, and recovery behavior
  have focused tests and target validation;
- RTC UI and configuration are complete inside this phase;
- general Backup SRAM and Flash ownership remain in Phases 11 and 12.

## Phase 10: Minimum Measurement Configuration Screens And Navigation

Objective:
- add the minimum authorized configuration path required by acquisition and
  measurement;
- validate and apply edits to active state before persistence is introduced.

Dependencies:
- Phase 8 operational navigation;
- approved editable measurement fields and ranges;
- current model separation between canonical configuration and derived values.

Reuse boundary:
- if Phase 9 is already implemented, reuse its reviewed
  configuration-navigation pattern without making RTC a prerequisite.

Minimum candidate scope:
- configuration entry or authorization screen;
- calibration factor K;
- ACM/TTL volume unit and resolution;
- RATE time base and resolution;
- navigation, edit buffer, validation, apply, and exit behavior.

Decision gates:
- exact minimum field set and authorization;
- save/cancel and invalid-edit behavior;
- whether any resolution shortcut remains on an operational screen;
- whether applied configuration remains volatile until Phases 11 and 12.

Legacy evidence:
- the implemented legacy setup contains password, factor, volume-unit, and
  time-unit screens;
- linearization, alarms, and unfinished settings do not enter the minimum slice
  automatically.

Exit criteria:
- the approved measurement settings can be edited and applied through focused
  configuration navigation;
- unit and factor changes update derived TTL, ACM, and RATE consistently;
- tests cover navigation, validation, cancel/apply, and boundary values;
- Backup SRAM, Flash, alarms, and advanced capability settings remain out of
  scope.

## Phase 11: Backup SRAM Retention

Objective:
- retain a human-approved minimal set of frequently changing runtime state,
  initially expected to fit within 1 KiB, while the backup supply remains
  valid.

Dependencies:
- stable canonical state from implemented release-scope capabilities, including
  RTC only when selected;
- approved reset classes and retention guarantees.

Decision gates:
- exact retained variables;
- integrity, version, initialization, and recovery;
- synchronization between live state and the backup domain;
- interaction with RTC backup-supply state without merging their ownership.

Risks:
- copying the legacy memory layout instead of retaining current canonical
  state;
- treating backup-powered retention as survival after loss of all supplies.

Exit criteria:
- retained state survives approved reset and supply cases;
- invalid retained data recovers according to approved policy;
- RTC and general retained state have explicit, non-overlapping ownership;
- Flash persistence remains separate.

## Phase 12: Flash Persistence

Objective:
- persist the approved low-change configuration that must survive loss of the
  backup supply.

Dependencies:
- Phase 10 measurement-configuration ownership;
- Phase 11 retained-state boundaries;
- approved factory/default and recovery behavior.

Decision gates:
- exact persisted variables and format versioning;
- write timing, atomicity, integrity, wear, and recovery;
- migration or factory reset behavior.

Risks:
- persisting derived values instead of canonical configuration;
- unnecessary writes or ambiguous recovery after interrupted updates.

Exit criteria:
- approved configuration survives full power removal;
- corruption and interrupted-write cases recover deterministically;
- Flash and Backup SRAM ownership does not overlap ambiguously.

## Phase 13: Temperature Measurement

Objective:
- add temperature as one complete vertical capability, from physical
  observation through product state and any required user/configuration
  screens and navigation.

Dependencies:
- stable runtime, UI, and persistence boundaries;
- human selection of the temperature capability and supported sensor.

Decision gates:
- sensor and electrical interface, range, resolution, accuracy, cadence, and
  energy budget;
- invalid/stale behavior and any compensation semantics;
- required operator view and editable parameters.

Legacy evidence:
- the extraction calls the optional sensor PT100 and sketches temperature and
  expansion-coefficient screens;
- preserved legacy firmware does not establish those sketches as a completed
  product implementation.

Exit criteria:
- temperature acquisition and quality are validated independently;
- approved temperature behavior is integrated with focused tests and target
  evidence;
- necessary temperature screens, configuration, navigation, and persistence
  are included in this phase rather than deferred to a generic UI phase.

## Phase 14: Bluetooth

Objective:
- add Bluetooth as one complete vertical communication capability, including
  only its required operator and configuration flows.

Dependencies:
- stable runtime, power, UI, and persistence boundaries;
- approved communication purpose, security boundary, and energy budget.

Decision gates:
- module and transport contract;
- connection roles, activation window, timeout, errors, and authorization;
- required screen, configuration, and diagnostic behavior.

Legacy evidence:
- legacy firmware provides a bounded Bluetooth connection window and uses the
  module as the transport for the printer;
- those control flows remain evidence, not current protocol authority.

Exit criteria:
- connection, transfer, timeout, recovery, and power behavior meet approved
  requirements;
- required Bluetooth UI/configuration/navigation is complete;
- printing is not implemented in this phase.

## Phase 15: Ticket Printing

Objective:
- add ticket printing as one complete vertical capability, including ticket
  content, Bluetooth transport integration, operator flow, configuration, and
  navigation.

Dependencies:
- Phase 9 RTC/calendar for approved ticket date/time;
- Phase 14 Bluetooth transport;
- stable ACM/TTL snapshot and persistence ownership.

Decision gates:
- ticket content, numbering, formatting, and privacy;
- print trigger, progress, timeout, retry, cancellation, and error behavior;
- whether RTC date/time is mandatory;
- printer compatibility and energy budget.

Legacy evidence:
- the preserved ticket contains ticket number, TTL, date, time, and ACM;
- the legacy user flow powers Bluetooth, connects to the printer, and sends the
  ticket in staged steps.

Exit criteria:
- approved ticket data is formed from coherent product state;
- Bluetooth transport and all operator-visible print states are validated;
- necessary print settings and navigation are complete inside this phase.

## Phase 16: Integral Validation, Power, And Product Behavior

Objective:
- validate the refactored firmware against confirmed product behavior and
  field constraints.

Dependencies:
- previous phases implemented for the selected release scope.

Decision gates:
- release acceptance criteria;
- current consumption thresholds;
- hardware bring-up coverage;
- unresolved requirements that must be settled before release.

Risks:
- passing unit tests while missing field behavior;
- current consumption regressions;
- drift between specification, tests, and firmware.

Exit criteria:
- canonical builds pass;
- regression tests and bring-ups cover selected product paths;
- current consumption and pulse behavior meet confirmed requirements;
- remaining unresolved items are explicitly out of release scope or decided.
