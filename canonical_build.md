# From “It Builds in the IDE” to a Canonical Firmware Build: Enabling Real AI-Assisted Development

## Executive summary (promotional)

Most embedded firmware projects unknowingly depend on an invisible assumption: *“if it builds in the IDE, the system is correct.”*
This assumption breaks immediately when introducing automation, CI, or AI agents.

By making the STM32 build **explicit, reproducible, and canonical outside the IDE**, this project eliminates that hidden dependency and unlocks a new capability:

> The firmware can now be **understood, modified, built, and validated consistently by both humans and AI agents**.

This is not a tooling improvement.
It is a **shift in the development contract** of the firmware.

---

# 1. The real problem: IDE-dependent builds

In STM32 + VS Code environments, the build pipeline is not fully visible:

* `cube`, `cube-cmake`, toolchains, and bundles are resolved implicitly
* environment variables are injected by the extension
* paths are versioned and not exported to the shell

As a result:

* The project builds inside VS Code
* The same project **fails from a clean terminal**
* An AI agent **cannot reproduce the build**

This is not a missing `cmake` issue.
It is a **hidden environment problem**.

---

# 2. Why this matters for AI-assisted development

Without a reproducible build, an agent can only:

* suggest code
* refactor structure
* apply naming rules
* generate plausible implementations

But it **cannot**:

* verify compilation
* distinguish environment vs code failures
* iterate safely
* close the engineering loop

This creates a fundamental limitation:

> The agent operates in a **speculative mode**, not an **engineering mode**.

---

# 3. What was implemented: canonical build + bootstrap

## 3.1 Explicit bootstrap of STM32 environment

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\stm32cube-env.ps1
```

This script:

* discovers STM32 extensions dynamically
* resolves environment via `cube`
* exports toolchain and build variables
* reconstructs `PATH` deterministically

Key property:

> The environment is **discovered, not assumed**.

---

## 3.2 Canonical build entrypoint

```powershell
cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

Selected based on:

* identical generator
* identical toolchain
* identical cache
* identical artifact

---

## 3.3 Agent-aware workflow

```powershell
where.exe cube
echo $env:CUBE_BUNDLE_PATH

.\tools\stm32cube-env.ps1

cube-cmake --preset Debug
cube-cmake --build --preset Debug
```

---

# 4. Impact: from assistance to operational capability

Before:

* No build verification
* No environment introspection
* No reliable iteration

After:

* Deterministic build
* Shared human/agent contract
* Closed engineering loop

> From **AI suggesting code** → to **AI participating in development**

---

# 5. Why the bootstrap is the central piece

It eliminates:

* IDE dependency
* environment ambiguity
* toolchain opacity
* version fragility

Without it:

> The system is non-reproducible → the agent is unreliable

---

# 6. What this does *not* solve

Canonical build is necessary, but not sufficient.

Missing layers:

* functional validation
* runtime verification
* integration confidence

---

# 7. What remains to be solved

This is the critical next phase.

## 7.1 Post-build validation contract

Currently:

> “build succeeded” is the only success criterion

This is insufficient.

Required next step:

* define **canonical post-build checks**, for example:

  * `.elf` presence and size range
  * expected sections / symbols
  * linker map sanity
  * memory usage constraints

Without this:

> The agent can produce **compiling but broken firmware**

---

## 7.2 Minimal runtime / bring-up validation

Build correctness ≠ system correctness.

Missing:

* startup sanity (vector table, reset handler)
* peripheral init expectations
* minimal “system alive” signal (e.g., log, GPIO toggle)

Recommended:

* define a **baseline runtime contract**:

  * what must be observable after boot
  * what defines “firmware alive”

---

## 7.3 Task-level validation patterns

Today tasks are well-scoped, but validation is implicit.

Next step:

* each task type should define:

  * what must compile
  * what must not break
  * what must be verified

Example:

* “new service module” → builds + no new dependencies upward
* “HAL wrapper” → builds + no leakage of HAL types

---

## 7.4 Structural invariants enforcement

The repo defines:

* folder ownership
* layering rules

But enforcement is manual.

Missing:

* lightweight checks:

  * forbidden includes (e.g., `app` including `cube`)
  * dependency direction validation
  * public/private API constraints

Without this:

> The agent may slowly degrade architecture while still passing build

---

## 7.5 CI or automated verification

Current flow is local.

Next logical step:

* minimal CI pipeline that:

  * bootstraps environment
  * runs canonical build
  * runs post-build checks

This would:

* validate reproducibility
* prevent regressions
* make agent output trustable at scale

---

## 7.6 Failure classification clarity

One subtle but important gap:

The system still relies on interpretation when build fails.

Needed:

* clear classification of failures:

  * environment not loaded
  * toolchain mismatch
  * code error
  * configuration error

This improves:

* agent diagnostics
* debugging speed
* prompt quality

---

## 7.7 Reduction of “implicit correctness”

Even with canonical build, some assumptions remain implicit:

* correct preset selection
* correct board configuration
* correct startup code alignment

Future improvement:

* make these explicit in documentation or checks

---

# 8. Key insight

The major achievement is not the script.

It is this:

> The build is now part of the **explicit contract of the repository**

What remains:

> Extending that contract beyond build → into **validation and correctness**

---

# 9. Roadmap summary

Current state:

* reproducible environment
* canonical build
* agent-compatible workflow

Next phase:

1. post-build validation
2. runtime sanity checks
3. structural enforcement
4. CI integration
5. failure classification

---

# 10. Conclusion

This project solved the hardest first problem:

> Making firmware build reproducible outside the IDE

That alone transforms AI usefulness.

But the full transformation requires one more step:

> Turning “it builds” into “it is correct”

When that layer is added:

* the agent will not only build firmware
* it will **validate, diagnose, and evolve it reliably**

That is the real end state.
