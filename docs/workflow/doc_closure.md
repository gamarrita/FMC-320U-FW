# Doc Closure

Use only when the active workstream is formally closed, deferred, or replaced.
It is not required for an individual task, slice, commit, handoff, or
conversation.

Prompt:

```text
Aplicar doc closure del workstream.
```

Meaning: consolidate the completed workstream, remove documentation drift, and
leave the repository ready to identify the next active workstream.

## Rules

- Do not keep documentation as backup history; Git does that.
- Move every durable outcome to its owning product document, specification,
  roadmap, public header, or legacy-disposition register.
- Before deleting or materially compressing a current product document or
  technical specification, identify the current owner of every durable,
  accepted contract it contains. Delete it only when all such content is
  preserved by identified authorities or has been explicitly retired by human
  decision. Keep the document when it still owns a coherent cross-cutting
  contract. If completeness, ownership, or retirement is uncertain, stop and
  ask the human before changing it.
- Delete or compress temporary narration, superseded plans, duplicated
  rationale, and validation history already visible in code, tests, specs, or
  Git.
- Keep `WORKING_CONTEXT.md` only when the workstream is explicitly deferred and
  its resumption state remains operationally necessary. Otherwise remove or
  reframe it for the next workstream.
- Do not change the authority of evidence, proposals, or historical material
  while consolidating documentation.

## Closure Checks

- The roadmap records the completed or deferred phase outcome and any remaining
  gates.
- Accepted product behavior and technical contracts are in their owning
  documents, not only in `WORKING_CONTEXT.md`.
- Every deleted or materially compressed product document or technical
  specification has an identified absorbing authority or an explicit human
  retirement decision. Documents that still uniquely own a coherent
  cross-cutting contract are preserved.
- No other `.md` claims to define the closed or replaced active workstream.
- References to superseded contexts, routes, files, and temporary decisions are
  removed.
- A new agent can identify the next active workstream, or determine explicitly
  that none is active.

## Final Scrape

Run:

```powershell
rg --files -g "*.md"
rg -n "<closed_workstream>|<replaced_context>|<deleted_file>|obsolete_route|TODO" -g "*.md"
```

Adapt the second command to the workstream being closed.

Report:
- documentation added, deleted, or compressed;
- stale references found or remaining;
- durable outcomes moved to their owners;
- product documents and technical specifications preserved because of unique
  ownership, and the absorbing authority or human disposition for each one
  removed;
- whether build verification was skipped because the closure was
  documentation-only.
