# Scratch (Temporary Work)

## Purpose

Workspace for **quick, disposable experiments**.

Used for:
- immediate validation
- bring-up attempts
- fast iteration

---

## What belongs here

- LED blink tests
- first UART attempts
- quick clock configs
- throwaway experiments

Examples:

scratch/
  led_blink/
  uart_test/
  clock_try/

---

## Characteristics

Projects here:
- are not stable
- may be incomplete
- can be deleted anytime
- have no guarantees

---

## Rule

Everything in this folder must be safe to delete without impact.

---

## Typical workflow

1. Create quick test
2. Outcome:
   - useless → delete
   - useful but simple → replicate in targets/
   - useful and complex → move to lab/

---

## Gray area

If a project starts growing:

→ move it to lab/

Do not let scratch accumulate important work.

---

## Anti-patterns (avoid)

- keeping old unused projects
- relying on scratch for real code
- using it as a permanent workspace

---

## Decision rule

Would I care if this disappears tomorrow?

- No → stays here
- Yes → move to lab/ or targets/