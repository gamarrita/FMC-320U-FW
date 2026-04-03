# CubeMX Workspace

This directory contains all CubeMX projects used during development.

The structure separates projects by **lifecycle and intent**, not by feature.

cube/
targets/
lab/
scratch/


---

# Purpose

CubeMX projects are **generation artifacts**, not firmware modules.

They define:
- hardware configuration
- clocks
- pinout
- peripheral initialization
- HAL setup

They do NOT define:
- application logic
- services
- drivers architecture

That logic lives outside `cube/`.

---

# Folder Overview

## `targets/`
Official CubeMX projects used by the firmware.

- stable
- versioned
- part of the product
- safe to regenerate

## `lab/`
Experiments with **technical value**.

- used to validate ideas
- may be referenced later
- not part of the product

## `scratch/`
Temporary workspace.

- quick tests
- disposable
- no long-term value

---

# Key Principle

CubeMX defines:

> how the hardware starts

The rest of the repository defines:

> what the firmware does

---

# Relationship with Firmware

CubeMX projects should remain minimal and focused on initialization.

Typical flow:

cube/targets → hardware init
bsp/ → board mapping
port/ → platform abstraction
services/ → reusable logic
app/ → application behavior


There is no 1:1 relationship between CubeMX projects and firmware modules.

---

# Workflow

## 1. Exploration

Start in `scratch/` when:
- testing something quickly
- validating basic behavior

## 2. Consolidation

Move to `lab/` when:
- the experiment has value
- results may be reused
- behavior is non-trivial

## 3. Integration

Move or replicate into `targets/` when:
- configuration becomes part of the product
- it defines a real firmware target

---

# Lifecycle

scratch → lab → targets


- `scratch`: disposable
- `lab`: useful experiments
- `targets`: official configuration

---

# Rules

## General

- Do not implement application logic inside CubeMX projects
- Keep generated code clean and regenerable
- Avoid modifying generated files beyond integration points

---

## Dependency rules

- `targets/` supports firmware
- `lab/` is reference only
- `scratch/` must not be used as dependency

Code in:
- `app/`
- `services/`
- `bsp/`
- `port/`

must NOT depend on:
- `lab/`
- `scratch/`

---

# Decision Guide

Before creating or keeping a CubeMX project:

### Question 1
Does this represent a real firmware target?

- Yes → `targets/`
- No → continue

### Question 2
Does this have long-term technical value?

- Yes → `lab/`
- No → `scratch/`

---

# Anti-patterns

- Using CubeMX projects to organize features
- Keeping all experiments forever
- Mixing application logic into generated code
- Creating multiple targets for the same hardware without need

---

# Summary

- `targets/` → product
- `lab/` → knowledge
- `scratch/` → temporary

CubeMX is a **tool for defining hardware initialization**, not a place to structure firmware logic.