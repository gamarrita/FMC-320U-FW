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
| `screens.SCREEN_STARTUP_ALL_SEGMENTS.display.backlight` | Deferred | `docs/product/fmc/user_interface.md` | Phase 6A does not impose backlight state |
| `screens.SCREEN_STARTUP_ALL_SEGMENTS.automatic_transition.target` | Accepted and incorporated | `docs/product/fmc/behavior.md` | Advances to firmware version |
| `screens.SCREEN_STARTUP_ALL_SEGMENTS.key_actions.KEY_ESC` | Accepted and incorporated | `docs/product/fmc/behavior.md` | SHORT ESC has the same effect as timeout |
| `screens.SCREEN_STARTUP_ALL_SEGMENTS.startup_special_behavior` | Deferred | - | Flash validation and restore belong to persistence work |
| `screens.SCREEN_FIRMWARE_VERSION.purpose` | Accepted and incorporated | `docs/product/fmc/user_interface.md` | Startup includes visible firmware identity |
| `screens.SCREEN_FIRMWARE_VERSION.display.lower_row` | Accepted and incorporated | `docs/product/fmc/user_interface.md` | Current provisional value is `00.01.00` |
| `screens.SCREEN_FIRMWARE_VERSION.display.suffix_or_legend` | Replaced | `docs/product/fmc/user_interface.md` | Current alpha field is provisional `B0`, not legacy `VE` |
| `screens.SCREEN_FIRMWARE_VERSION.display.duration_s` | Accepted and incorporated | `docs/product/fmc/behavior.md` | Nominal 3 seconds after successful presentation |
| `screens.SCREEN_FIRMWARE_VERSION.display.backlight` | Deferred | `docs/product/fmc/user_interface.md` | Phase 6A does not impose backlight state |
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
| `screens.SCREEN_TTL_RATE.display.pulse_indicator` | Deferred | `docs/product/fmc/user_interface.md` | Depends on acquisition and pulse behavior |
| `screens.SCREEN_TTL_RATE.calculations.source` | Deferred | `docs/product/fmc/requirements.md` | Live acquisition and RATE-window ownership remain Phase 7 dependencies |
| `measurement_and_test_guidance.startup_tests[id=0x2FB396C8]` | Legacy only | - | Historical test identity retained; current tests are independently specified |
| `measurement_and_test_guidance.startup_tests[id=0x2FB39787]` | Legacy only | - | Historical test identity retained; current tests are independently specified |
| `measurement_and_test_guidance.SCREEN_TTL_RATE_tests[id=0x2FB398FC].requirements[0]` | Deferred | - | Current-consumption validation requires target hardware |
| `measurement_and_test_guidance.SCREEN_TTL_RATE_tests[id=0x2FB398FC].requirements[1]` | Deferred | - | Pulse indication depends on acquisition |
| `measurement_and_test_guidance.SCREEN_TTL_RATE_tests[id=0x2FB398FC].requirements[2]` | Deferred | - | Generator and flow behavior require acquisition |
| `measurement_and_test_guidance.SCREEN_TTL_RATE_tests[id=0x2FB398FC].requirements[3]` | Deferred | - | Generator and flow behavior require acquisition |
| `open_questions_and_gaps.items[id=OQ_001]` | Legacy only | - | Records a limitation of the legacy source; physical mapping is owned separately by the LCD technical authority |
| `open_questions_and_gaps.items[id=OQ_007]` | Under review | `docs/product/fmc/user_interface.md` | Phase 6A resolves visual overflow; absent and invalid values remain deferred |

## Extraction Discrepancies

No content discrepancy has been evaluated or recorded in this context.
