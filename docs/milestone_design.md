## Current Milestone: M2 - Coding rules and tooling baseline

### Goal
Define the minimum set of rules and tools required to write consistent embedded C code,
compatible with both human development and AI-assisted generation.

---

### Constraints
- Must be simple and practical
- Must avoid over-engineering
- Must not conflict with AI-generated code (Codex)
- Must be easy to maintain long term
- Must not introduce unnecessary dependencies

---

### Non-goals
- No firmware architecture definition
- No CubeMX setup
- No RTOS integration
- No complex static analysis setup
- No process-heavy documentation

---

### Key questions
- How strict should naming rules be?
- How much should be enforced by formatter vs manual rules?
- Do we include static analysis now or later?
- How to avoid conflict between human style and generated code?

---

### Candidate approach

Minimal baseline:

- `.editorconfig`
  → whitespace, encoding, line endings

- `.clang-format`
  → automatic formatting (single source of truth for layout)

- `docs/c_style.md`
  → naming, file structure, API rules

- No static analysis for now
  → postpone until real code exists

---

### Tradeoffs

Pros:
- fast to implement
- low friction
- compatible with Codex
- avoids premature complexity

Cons:
- less strict enforcement initially
- potential inconsistencies until rules stabilize

---

### Decisions (to confirm)

- Use clang-format as the only formatter
- Keep style rules minimal and human-readable
- Avoid static analysis in this milestone
- Prioritize consistency over strictness

---

### Open points (if any)
- Naming strictness level (to finalize in `c_style.md`)
- Whether to define error handling conventions now or later