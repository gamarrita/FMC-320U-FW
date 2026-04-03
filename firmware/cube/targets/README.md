# Targets (Product)

## Purpose

Contains **official CubeMX projects** that define real firmware targets.

These projects are:
- stable
- maintained
- part of the product
- safe to regenerate

---

## What belongs here

- `.ioc` files for real targets
- generated code (Core/, Drivers/, etc.)
- minimal integration adjustments

Example:

targets/
  fmc320u_main/
    FMC-320U.ioc
    Core/
    Drivers/

---

## What does NOT belong here

- quick tests
- experiments
- exploratory setups
- temporary hello world projects

---

## Expected usage

Code in this folder should only:

- initialize hardware
- call into application layer

Example:

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_USARTx_UART_Init();

    FM_APP_Init();

    while (1)
    {
        FM_APP_Process();
    }
}

---

## Rules

- Avoid modifying generated files beyond integration points
- Do not add application logic here
- Must support regeneration

---

## When to create a new target

Only if there is a real change in:

- MCU
- board
- boot mode (bootloader vs app)
- incompatible base configuration

---

## Decision rule

Does this represent a real executable firmware target?

- Yes → stays here
- No → goes to lab/ or scratch/