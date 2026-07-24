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

## Extraction Discrepancies

No content discrepancy has been evaluated or recorded in this context.
