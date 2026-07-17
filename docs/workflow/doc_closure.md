# Doc Closure

Use before commit when documentation or active context changed.

Prompt:

```text
Aplicar doc closure pre-commit.
```

Meaning: apply this file, clean documentation drift, then report added,
deleted, compressed, and still-questionable docs.

## Rules

- Run before commit. Post-commit use is audit only.
- Do not keep documentation as backup history; Git does that.
- Documentation must become shorter, clearer, or more directly useful.
- Delete or compress old plans, option comparisons, slice narration,
  duplicated validation history, and rationale already visible in code, tests,
  specs, or Git history.

## Active Route

After closure, the active route must be:

```text
AGENTS.md -> WORKING_CONTEXT.md -> docs/contexts/<active>.md
```

Required:
- `WORKING_CONTEXT.md` names exactly one active workstream.
- `WORKING_CONTEXT.md` names exactly one active expanded context.
- No other `.md` claims to define the current task or active workstream.
- A new agent can reach every task-relevant `.md` in three hops or fewer.
- The active expanded context explains the workstream but does not replace
  `WORKING_CONTEXT.md` as the routing entrypoint.

## Ownership

- Stable agent policy: `AGENTS.md`
- Active execution state: `WORKING_CONTEXT.md`
- Active workstream rationale: one file under `docs/contexts/`
- Folder ownership: local `README.md` files
- Product or engineering contracts: `docs/specs/`
- Historical legacy evidence: `legacy/`

Closed contexts are deleted by default. Keep one only if it contains concise
evidence needed by the active workstream.

## Final Scrape

Run:

```powershell
rg --files -g "*.md"
rg -n "<old_context>|<deleted_file>|active extended context|TODO" -g "*.md"
```

Adapt the second command to the files and terms touched by the slice.

Report:
- docs added, deleted, or compressed
- whether stale references remain
- whether build verification was skipped because the change was documentation-only
