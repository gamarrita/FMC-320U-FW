# FMC Legacy Coverage Register

## Purpose

Track which parts of the frozen FMC legacy extraction have been evaluated and
record their human-decided disposition. This is a mutable traceability record,
not a product specification, requirement list, screen catalog, firmware
progress measure, or functional authority.

## Source Chain

Primary historical source:
- repository path: `legacy/specs/fmc/use_cases.docx`;
- use: exceptional audit of extraction ambiguity, omission, or suspected error;
- preservation: do not modify or consult routinely.

Frozen structured extraction:
- path: `legacy/derived/fmc/use_cases.extraction-v1.yaml`;
- classification: `legacy/derived/fmc/README.md`;
- SHA-256 at classification:
  `FBF4987193AC60777754136331B311BBDFF709FF818B6100B8FE246AF11662F2`;
- role: preferred legacy evidence for later consultation;
- preservation: do not edit, clean, split, shorten, or use as a task list.

Current product documentation:
- index: `docs/product/fmc/README.md`;
- obligations: `docs/product/fmc/requirements.md`;
- cross-cutting observable behavior: `docs/product/fmc/behavior.md`;
- visible and operable experience:
  `docs/product/fmc/user_interface.md`.

## Coverage States

- `Not evaluated`: no human disposition has been recorded.
- `Under review`: an explicit review is active but no disposition is final.
- `Accepted and incorporated`: a human-approved decision is represented in its
  current product owner.
- `Rejected`: the legacy content was reviewed and deliberately not adopted.
- `Replaced`: a reviewed current decision supersedes the legacy content.
- `Deferred`: review or incorporation is deliberately postponed.
- `Legacy only`: the content remains useful only as historical evidence.

These are legacy-review dispositions. They do not replace the product decision
states used under `docs/product/fmc/`.

## Register Rules

- Use an existing legacy identifier when one is available; otherwise use the
  stable YAML structural path.
- Do not invent identifiers that could be confused with identifiers inside the
  frozen extraction.
- Do not copy full use cases or requirement text into this register.
- Link the current owning product document only after a human-approved
  disposition makes that relationship valid.
- Do not infer `Accepted and incorporated`, `Rejected`, or `Replaced`.
- Evaluated entries remain in the register even after pending work shrinks.
- Record suspected conversion errors as discrepancies; do not edit the frozen
  extraction. A material correction requires evaluation of a new extraction
  version.

## Coverage

All entries are initialized without a functional disposition.

| Legacy YAML locator | Coverage state | Current product owner | Review note |
|---|---|---|---|
| `document` | Not evaluated | - | Extraction metadata and policy |
| `product` | Not evaluated | - | Awaiting human review |
| `ui_scope` | Not evaluated | - | Awaiting human review |
| `system_requirements` | Not evaluated | - | Awaiting human review |
| `persistence_model` | Not evaluated | - | Awaiting human review |
| `boot_and_reset_behavior` | Not evaluated | - | Awaiting human review |
| `hardware_configuration` | Not evaluated | - | Awaiting human review |
| `input_model` | Not evaluated | - | Awaiting human review |
| `screen_catalog` | Not evaluated | - | Awaiting human review |
| `screens` | Not evaluated | - | Awaiting human review |
| `GLOBAL_USER_SCREEN_ALARMS` | Not evaluated | - | Awaiting human review |
| `configuration_screens` | Not evaluated | - | Awaiting human review |
| `printing_and_connectivity` | Not evaluated | - | Awaiting human review |
| `data_logging_monitor` | Not evaluated | - | Awaiting human review |
| `measurement_and_test_guidance` | Not evaluated | - | Awaiting human review |
| `open_questions_and_gaps` | Not evaluated | - | Awaiting human review |

## Phase 6A Granular Review

The top-level entries above remain `Not evaluated`. The following rows cover
only the exact legacy statements reviewed for Phase 6A.

| Legacy YAML locator | Coverage state | Current product owner | Review note |
|---|---|---|---|
| `screen_catalog.startup_sequence.ordered_screens` | Accepted and incorporated | `docs/product/fmc/user_interface.md` | Accepted order: all segments, version, TTL/RATE |
| `screens.SCREEN_STARTUP_ALL_SEGMENTS.purpose` | Accepted and incorporated | `docs/product/fmc/user_interface.md` | Visual check and startup signal |
| `screens.SCREEN_STARTUP_ALL_SEGMENTS.display.behavior` | Replaced | `docs/product/fmc/user_interface.md` | Current decision explicitly includes every software-controllable glass element |
| `screens.SCREEN_STARTUP_ALL_SEGMENTS.display.duration_s` | Accepted and incorporated | `docs/product/fmc/behavior.md` | Nominal 3 seconds after successful presentation |
| `screens.SCREEN_STARTUP_ALL_SEGMENTS.display.backlight` | Replaced | `docs/product/fmc/behavior.md` | Phase 8 requests backlight activation when this view is successfully presented and uses the common fixed interval rather than coupling it to the full screen duration |
| `screens.SCREEN_STARTUP_ALL_SEGMENTS.automatic_transition.target` | Accepted and incorporated | `docs/product/fmc/behavior.md` | Advances to firmware version |
| `screens.SCREEN_STARTUP_ALL_SEGMENTS.key_actions.KEY_ESC` | Accepted and incorporated | `docs/product/fmc/behavior.md` | SHORT ESC has the same effect as timeout |
| `screens.SCREEN_STARTUP_ALL_SEGMENTS.startup_special_behavior` | Deferred | - | Flash validation and restore belong to persistence work |
| `screens.SCREEN_FIRMWARE_VERSION.purpose` | Accepted and incorporated | `docs/product/fmc/user_interface.md` | Startup includes visible firmware identity |
| `screens.SCREEN_FIRMWARE_VERSION.display.lower_row` | Accepted and incorporated | `docs/product/fmc/user_interface.md` | Current provisional value is `00.01.00` |
| `screens.SCREEN_FIRMWARE_VERSION.display.suffix_or_legend` | Replaced | `docs/product/fmc/user_interface.md` | Current alpha field is provisional `B0`, not legacy `VE` |
| `screens.SCREEN_FIRMWARE_VERSION.display.duration_s` | Accepted and incorporated | `docs/product/fmc/behavior.md` | Nominal 3 seconds after successful presentation |
| `screens.SCREEN_FIRMWARE_VERSION.display.backlight` | Replaced | `docs/product/fmc/behavior.md` | Backlight follows the common startup and physical-input interval; it is not guaranteed on for the full version view |
| `screens.SCREEN_FIRMWARE_VERSION.automatic_transition.target` | Accepted and incorporated | `docs/product/fmc/behavior.md` | Advances to TTL/RATE |
| `screens.SCREEN_FIRMWARE_VERSION.key_actions.KEY_ESC` | Accepted and incorporated | `docs/product/fmc/behavior.md` | SHORT ESC has the same effect as timeout |
| `screens.SCREEN_TTL_RATE.display.upper_row.field` | Accepted and incorporated | `docs/product/fmc/user_interface.md` | TTL uses the upper row |
| `screens.SCREEN_TTL_RATE.display.upper_row.unit_display` | Replaced | `docs/product/fmc/user_interface.md` | Shared alpha `Lt` applies to TTL and RATE |
| `screens.SCREEN_TTL_RATE.display.upper_row.persistence` | Deferred | - | TTL retention and persistence belong to Phases 11 and 12 |
| `screens.SCREEN_TTL_RATE.display.upper_row.digit_count` | Accepted and incorporated | `docs/product/fmc/user_interface.md` | Upper row uses its eight physical positions |
| `screens.SCREEN_TTL_RATE.display.upper_row.decimal_position_options` | Deferred | `docs/product/fmc/user_interface.md` | Phase 6A fixes one decimal; configurable resolution remains later work |
| `screens.SCREEN_TTL_RATE.display.lower_row.field` | Accepted and incorporated | `docs/product/fmc/user_interface.md` | RATE uses the lower row |
| `screens.SCREEN_TTL_RATE.display.lower_row.resolution_editable_from_screen` | Deferred | `docs/product/fmc/user_interface.md` | Configuration and editing remain outside Phase 6A |
| `screens.SCREEN_TTL_RATE.display.lower_row.units_source` | Deferred | `docs/product/fmc/user_interface.md` | Phase 6A fixes `Lt/min`; configured units remain later work |
| `screens.SCREEN_TTL_RATE.display.pulse_indicator` | Replaced | `docs/product/fmc/behavior.md` | POINT now has a precise periodic-observation rule across the five Phase 8 user-menu screens |
| `screens.SCREEN_TTL_RATE.calculations.source` | Deferred | `docs/product/fmc/requirements.md` | Live acquisition and RATE-window ownership remain Phase 7 dependencies |
| `measurement_and_test_guidance.startup_tests[id=0x2FB396C8]` | Legacy only | - | Historical test identity retained; current tests are independently specified |
| `measurement_and_test_guidance.startup_tests[id=0x2FB39787]` | Legacy only | - | Historical test identity retained; current tests are independently specified |
| `measurement_and_test_guidance.SCREEN_TTL_RATE_tests[id=0x2FB398FC].requirements[0]` | Deferred | - | Current-consumption validation requires target hardware |
| `measurement_and_test_guidance.SCREEN_TTL_RATE_tests[id=0x2FB398FC].requirements[1]` | Deferred | - | Pulse indication depends on acquisition |
| `measurement_and_test_guidance.SCREEN_TTL_RATE_tests[id=0x2FB398FC].requirements[2]` | Deferred | - | Generator and flow behavior require acquisition |
| `measurement_and_test_guidance.SCREEN_TTL_RATE_tests[id=0x2FB398FC].requirements[3]` | Deferred | - | Generator and flow behavior require acquisition |
| `open_questions_and_gaps.items[id=OQ_001]` | Legacy only | - | Records a limitation of the legacy source; physical mapping is owned separately by the LCD technical authority |
| `open_questions_and_gaps.items[id=OQ_007]` | Under review | `docs/product/fmc/user_interface.md` | Phase 6A resolves visual overflow; absent and invalid values remain deferred |

## Phase 7A Granular Review

The following disposition covers only the legacy pulse-conservation statement
reviewed for the bounded pulse-accumulation contract.

| Legacy YAML locator | Coverage state | Current product owner | Review note |
|---|---|---|---|
| `screens.SCREEN_TTL_RATE.calculations.invariants[0]` | Accepted and incorporated | `docs/product/fmc/requirements.md` | Accepted in bounded form: no counted pulse may be lost or duplicated after the primary counter is armed, including through Stop2; pulses before rearm after MCU reset are excluded |

## Phase 8 Granular Review

The following rows record only the legacy statements explicitly reviewed while
defining the Phase 8 user-menu contract. Current behavior is owned by the
linked product documents; this table records disposition and provenance.

| Legacy YAML locator | Coverage state | Current product owner | Review note |
|---|---|---|---|
| `hardware_configuration.jumpers_j3.PD3` | Accepted and incorporated | `docs/roadmaps/fmc_refactoring.md` | EXT_1 is the active-low external button on PD3; the accepted CubeMX gate requires the MCU pull-up |
| `hardware_configuration.jumpers_j3.PD4` | Accepted and incorporated | `docs/roadmaps/fmc_refactoring.md` | EXT_2 is the active-low external button on PD4; the accepted CubeMX gate requires the MCU pull-up |
| `system_requirements.keypad_semantics.external_pushbuttons.behavior` | Accepted and incorporated | `docs/product/fmc/user_interface.md` | External buttons retain a single SHORT semantic action |
| `system_requirements.keypad_semantics.external_pushbuttons.trigger` | Replaced | `docs/product/fmc/behavior.md` | Current behavior acts on the accepted falling press transition, not on release |
| `input_model.valid_key_action_combinations.external_buttons.actions` | Accepted and incorporated | `docs/product/fmc/requirements.md` | EXT_1 and EXT_2 expose SHORT only |
| `input_model.valid_key_action_combinations.external_buttons.note` | Replaced | `docs/product/fmc/behavior.md` | Current press-edge action and stable-release rearm supersede release-triggered SHORT |
| `input_model.event_model.rule` | Replaced | `docs/roadmaps/fmc_refactoring.md` | Key and action identity remain semantic, but product-main routes operator input directly to UI rather than retaining it in measurement runtime; unrelated asynchronous service results use their own app/domain route |
| `screen_catalog.user_menu_order` | Replaced | `docs/product/fmc/user_interface.md` | Current order is TTL/RATE, ACM/RATE, PRINT, LOG_DOWNLOAD, DATE_TIME; optional temperature is excluded |
| `screens.SCREEN_TTL_RATE.navigation` | Replaced | `docs/product/fmc/user_interface.md` | Current bounded mechanical traversal and cyclic EXT_1 behavior are fully specified |
| `screens.SCREEN_TTL_RATE.key_actions` | Replaced | `docs/product/fmc/user_interface.md` | Configuration entry and resolution shortcuts are deferred to Phase 10; all Phase 8 consequences are explicit |
| `screens.SCREEN_TTL_RATE.external_actions` | Accepted and incorporated | `docs/product/fmc/user_interface.md` | EXT_1 advances and EXT_2 is a no-op |
| `screens.SCREEN_ACM_RATE.display` | Replaced | `docs/product/fmc/user_interface.md` | Current contract fixes ACM on the upper row and reuses the accepted RATE representation plus user-menu POINT |
| `screens.SCREEN_ACM_RATE.key_actions` | Replaced | `docs/product/fmc/user_interface.md` | LONG ENTER directly resets ACM; alarm reset is excluded and navigation follows the accepted five-screen order |
| `screens.SCREEN_ACM_RATE.external_actions` | Accepted and incorporated | `docs/product/fmc/user_interface.md` | EXT_1 advances and EXT_2 directly resets ACM |
| `screens.SCREEN_PRINTER.purpose` | Deferred | `docs/product/fmc/user_interface.md` | Printing remains Phase 15 work; Phase 8 only reserves the PRINT menu position |
| `screens.SCREEN_PRINTER.display` | Replaced | `docs/product/fmc/user_interface.md` | Phase 8 uses the static `PR` / right-aligned `OFF` placeholder without progress or result states |
| `screens.SCREEN_PRINTER.key_actions` | Replaced | `docs/product/fmc/user_interface.md` | The placeholder only navigates; it cannot start printing |
| `screens.SCREEN_PRINTER.external_actions` | Replaced | `docs/product/fmc/user_interface.md` | EXT_1 navigates; EXT_2 cannot start printing in Phase 8 |
| `screens.SCREEN_BLUETOOTH_WINDOW.purpose` | Replaced | `docs/product/fmc/user_interface.md` | Current screen identity is LOG_DOWNLOAD; Bluetooth is a possible future transport, not the operator purpose |
| `screens.SCREEN_BLUETOOTH_WINDOW.display` | Replaced | `docs/product/fmc/user_interface.md` | Phase 8 uses the static `LD` / right-aligned `OFF` placeholder with no connection window or timeout |
| `screens.SCREEN_BLUETOOTH_WINDOW.protocol_summary` | Deferred | `docs/product/fmc/requirements.md` | Logged-data transfer and transport behavior require later focused decisions |
| `screens.SCREEN_BLUETOOTH_WINDOW.key_actions` | Replaced | `docs/product/fmc/user_interface.md` | The placeholder only navigates and has no connection action |
| `screens.SCREEN_BLUETOOTH_WINDOW.external_actions` | Replaced | `docs/product/fmc/user_interface.md` | EXT_1 navigates according to the current menu; EXT_2 is a no-op |
| `screens.SCREEN_DATE_TIME.purpose` | Deferred | `docs/product/fmc/user_interface.md` | Actual calendar content remains Phase 9 work; Phase 8 only reserves the DATE_TIME position |
| `screens.SCREEN_DATE_TIME.display` | Replaced | `docs/product/fmc/user_interface.md` | Phase 8 uses the static `DT` / right-aligned `OFF` placeholder |
| `screens.SCREEN_DATE_TIME.side_effects` | Rejected | `docs/product/fmc/user_interface.md` | Merely viewing DATE_TIME does not enable Bluetooth or data download |
| `screens.SCREEN_DATE_TIME.timeout` | Replaced | `docs/product/fmc/user_interface.md` | The placeholder has no auto-return or timeout |
| `GLOBAL_USER_SCREEN_ALARMS.pulse_indicator` | Replaced | `docs/product/fmc/behavior.md` | POINT is driven on user-menu screens by accepted periodic pulse observations with explicit toggle/off semantics |
| `configuration_screens.shared_rules.backlight_policy` | Replaced | `docs/product/fmc/behavior.md` | The fixed ten-second restart policy now applies to every valid physical press in startup and user contexts; future configuration adopts the same transverse behavior |
| `open_questions_and_gaps.items[id=OQ_005]` | Replaced | `docs/product/fmc/user_interface.md` | LOG_DOWNLOAD separates download purpose from DATE_TIME and from the Bluetooth transport name |
| `open_questions_and_gaps.items[id=OQ_006]` | Deferred | `docs/product/fmc/user_interface.md` | Alarm reset remains outside Phase 8 |
| `open_questions_and_gaps.items[id=OQ_008]` | Replaced | `docs/product/fmc/behavior.md` | Current physical-input activity and backlight timing are explicitly defined |

## Extraction Discrepancies

No content discrepancy has been evaluated or recorded in this context.
