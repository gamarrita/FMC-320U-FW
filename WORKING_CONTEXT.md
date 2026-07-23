# Working Context: FMC Phase 6 Screen Specification Normalization

## Active Workstream

Prepare the product-screen behavior specification needed before implementing
Phase 6 presentation and UI state-machine code.

This is a documentation workstream. It pauses the previous Phase 6
implementation context until the selected screen behavior documents are created
and reviewed by the human.

Current source to normalize:
- `docs/specs/fmc/use_cases.yaml`

Legacy evidence:
- original legacy use-case Word document, when available to the agent

Durable phase strategy:
- `docs/roadmaps/fmc_refactoring.md`, Phase 6

Operational references:
- `AGENTS.md`
- `docs/specs/README.md`
- `docs/specs/fmc/screen_spec_style.md`
- `docs/workflow/doc_closure.md`

## Reason For Reframe

The monolithic `use_cases.yaml` mixes screen behavior, inferred guidance,
legacy evidence, test and validation notes, unresolved behavior, shared
workflows, and future features. That shape is risky for Phase 6 because an
agent could implement unexpected behavior by treating nearby evidence, legacy
test notes, or inferred content as confirmed screen requirements.

The next step is to create smaller, human-reviewable screen behavior documents
before product UI code is written.

## Milestone Outcome

Produce a minimal screen-specification set that is easier for humans and agents
to audit:
- each selected screen can be reviewed from one small document;
- behavior, legacy evidence, unresolved notes, and implementation guesses are
  not mixed together;
- inherited test or validation methods are not promoted to current
  requirements;
- legacy screen IDs are not forced into the new documents unless the agent
  justifies a useful traceability method;
- no product behavior becomes current implementation authority until the human
  reviews and accepts the new documents.

## Selected Documentation Slice

Normalize only the screens needed for the first Phase 6 implementation slice:
- startup all segments;
- firmware version;
- TTL/RATE steady user screen.

The agent may propose stable screen keys or IDs only if they improve
traceability or future implementation. Do not force legacy names such as
`SCREEN_STARTUP_ALL_SEGMENTS` to be the primary human-facing contract.

## Expected Workflow

1. Audit the current `use_cases.yaml` and available legacy Word evidence for
   the selected three screens.
2. Propose a small target document structure before doing a broad split.
3. Create only the minimal files needed for the selected documentation slice.
4. Separate confirmed behavior from inferred notes and unresolved notes.
5. Keep legacy test IDs, validation procedures, and historical acceptance notes
   out of the screen behavior contract. If retained, place them only in
   traceability evidence.
6. Mark the old `use_cases.yaml` role clearly so it does not compete silently
   with the new files.
7. Stop after producing the new documents and report them for human review.

## Human Review Gate

Do not implement Phase 6 presentation, LCD rendering, screen-state code,
navigation, timers, or product UI behavior from the new screen documents until
the human explicitly reviews and accepts them.

The agent may reorganize evidence. The agent must not convert evidence,
validation notes, or unresolved items into confirmed behavior without human
approval.

## Decisions In Force

- This context is documentation-only.
- Do not change C code, build files, generated code, CubeMX configuration, or
  protected hardware configuration files.
- Preserve `src/product/fmc` independence from ThreadX, HAL, BSP, GPIO, queues,
  timers, and LCD details as an implementation boundary for the later Phase 6
  coding context.
- Keep the current Phase 5 runtime baseline as background only: existing
  `FM_APP` owner loop, app-level queue, semantic input, and periodic refresh.
- Do not inherit the legacy validation method as current test policy.
- Do not force legacy screen IDs as the new contract shape.
- Prefer Markdown or lightweight structured documents that a human can read
  comfortably. Use YAML only when structure clearly helps.
- Prefer local unresolved notes in each screen document instead of creating a
  separate gaps document for this slice.
- Defer config screens, alarms, printer, Bluetooth, optional PT100, RTC,
  persistence, acquisition, and full menu traversal unless needed only as
  references or unresolved notes for the selected three screens.

## Boundaries

Out of scope for this documentation context:
- implementation of the UI state machine;
- LCD/BSP contract changes;
- new validation framework or inherited legacy test migration;
- full split of every future screen if it makes the first review harder;
- product decisions about unresolved behavior;
- commits unless explicitly requested.

## Closure Criteria

This context can close when:
- the selected three screen documents exist in a concise, human-reviewable
  form;
- the role of `use_cases.yaml` after the split is explicit;
- unresolved behavior is visible and not encoded as confirmed behavior;
- any retained legacy test or validation information is clearly traceability
  evidence, not the current validation contract;
- the human has reviewed the produced documents or explicitly decides how to
  proceed;
- `docs/workflow/doc_closure.md` has been applied before commit.
