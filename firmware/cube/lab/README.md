# Lab (Technical Exploration)

## Purpose

Contains **experiments with long-term value**.

These are not part of the product, but:
- capture knowledge
- validate technical decisions
- serve as reference

---

## What belongs here

- non-trivial peripheral validation
- timing experiments
- power mode validation
- reduced test environments

Examples:

lab/
  uart_dma_probe/
  stop2_power_test/
  external_flash_timing/

---

## Characteristics

Projects here:
- may persist for a long time
- can be referenced later
- can influence design decisions

But:
- are not product code
- must not be dependencies of firmware

---

## Typical workflow

1. Create experiment in lab/
2. Validate behavior
3. Extract learnings
4. Apply to:
   - targets/
   - bsp/
   - port/
   - services/

---

## What does NOT belong here

- trivial tests (LED blink)
- disposable code
- quick checks

---

## Gray area

If a project becomes foundational:

→ it must be promoted or absorbed into targets/

Do not leave critical configuration here.

---

## Dependency rule

Code in:
- app/
- services/
- bsp/
- port/

must NOT depend on lab/.

Lab is reference only.

---

## Decision rule

Will this help understand or validate something later?

- Yes → stays here
- No → goes to scratch/