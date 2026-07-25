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
- whether build verification was skipped because the closure was
  documentation-only.
