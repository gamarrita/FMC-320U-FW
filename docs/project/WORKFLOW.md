# Human-Agent-Repository Workflow

## Purpose

The repository is the persistent project memory. Human-agent conversations are
temporary work sessions used to inspect, decide, implement, and verify changes.
A conversation is successful only when another new conversation can reconstruct
the accepted state and the next work slice by reading the repository alone.

This document defines the continuity method. Stable agent policy remains in
`AGENTS.md`, active scope in `WORKING_CONTEXT.md`, durable strategy in roadmaps,
accepted product knowledge in the applicable product documentation or
specification, and implemented public contracts in headers.

## Responsibilities

The human:
- defines product intent and authorizes material scope or product decisions;
- reviews ambiguities, tradeoffs, and proposed authorities;
- approves protected-file changes, commits, pushes, and other explicit gates.

The agent:
- reads repository context before acting;
- distinguishes accepted decisions from proposals, inferences, and evidence;
- keeps changes within the active cut and reports conflicts instead of
  resolving them silently;
- writes durable outcomes into their owning repository documents;
- verifies the result and reports the exact Git state used for the work.

The repository:
- records stable policy, active scope, durable strategy, accepted decisions,
  implementation contracts, evidence provenance, and remaining decisions;
- identifies enough state for work to continue without relying on chat history.

## Complete New-Conversation Cycle

1. Identify the repository, current branch, HEAD commit, and worktree status.
2. Read the user request, `AGENTS.md`, and `WORKING_CONTEXT.md`.
3. Follow the references from `WORKING_CONTEXT.md` to the applicable roadmap,
   product documentation, specifications, local READMEs, and technical
   authorities.
4. Inspect relevant code, tests, and legacy evidence only as required by the
   active cut.
5. State the interpreted objective, intended files, boundaries, authorities,
   unresolved conflicts, and proportional verification plan.
6. Obtain human approval when the active context or repository policy requires
   it, then make the smallest reviewable change.
7. Verify behavior or documentation in proportion to risk. Apply
   `docs/workflow/doc_closure.md` whenever documentation or active context
   changes.
8. Review the diff against the approved scope and confirm that no source has
   silently changed role from evidence or input into authority.
9. Record durable decisions and the next active cut in their owning repository
   files. Do not leave required state only in the conversation.
10. Report changed files, verification results, unresolved decisions, and final
    branch, HEAD commit, and worktree status.
11. Commit or push only when the human explicitly requests it. The resulting
    commit or intentionally identified uncommitted diff becomes the auditable
    handoff point.

## Permanent Decisions

Put each durable fact in one owning location:
- stable agent rules in `AGENTS.md`;
- one active cut in `WORKING_CONTEXT.md`;
- phase order, dependencies, decision gates, risks, and exit criteria in
  `docs/roadmaps/`;
- accepted product requirements, behavior, and interface decisions in the
  applicable product documentation or specification;
- hardware configuration in CubeMX-managed sources;
- technical LCD glass and mapping facts in
  `docs/specs/lcd/lcd_true_source.yaml`;
- implemented module contracts in public headers;
- historical material and provenance under `legacy/`.

Working inventories, old firmware, test notes, and conversation text do not
become authority merely because they are detailed. Promote a decision only
through explicit human review and an update to its owning repository file.

## Audit On An Identifiable Revision

Before starting and before handing off, capture:

```powershell
git branch --show-current
git rev-parse HEAD
git status --short --branch
git diff --check
git diff
```

The audit statement must identify the branch and HEAD commit. If the work is
uncommitted, the reviewed `git diff` and final worktree status are part of the
handoff. If the work is committed, the commit hash is the stable audit anchor.
Never describe a conversation alone as the project state.

## Success Criterion

Continuity is achieved when a new agent with no conversation history can read
only the repository and correctly determine:
- which sources are authoritative for the work at hand;
- the single active cut, its scope, and its exclusions;
- the durable roadmap sequence and dependencies;
- what has already been accepted or implemented;
- which decisions remain open;
- the next reviewable cut and its verification gate;
- the branch or commit against which those facts were audited.

If any of those answers exists only in chat, the workflow is not closed.

## Copyable Base Prompt

```text
Trabajaremos sobre este repositorio como memoria persistente del proyecto.
La conversación es descartable: no dependas de conversaciones anteriores.

Antes de proponer o modificar:
1. identificá rama, commit HEAD y estado del worktree;
2. leé AGENTS.md y WORKING_CONTEXT.md;
3. seguí las referencias del contexto activo hacia roadmap, documentación de
   producto, especificaciones, READMEs locales y autoridades técnicas;
4. distinguí autoridades vigentes, entradas de trabajo, evidencia legacy,
   decisiones pendientes e inferencias;
5. informá objetivo interpretado, archivos previstos, límites, conflictos y
   verificación.

Trabajá únicamente dentro del corte activo y respetá sus puertas de aprobación.
No conviertas evidencia o inventarios en requisitos sin aprobación humana.
Registrá toda decisión durable en el archivo propietario correspondiente.
Antes de cerrar, revisá el diff, aplicá doc closure si corresponde y reportá
archivos cambiados, verificaciones, pendientes, próximo corte propuesto, rama,
commit HEAD y estado final de Git. No hagas commit ni push sin autorización
explícita.
```
