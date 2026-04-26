# port

## Intent
Contain platform-dependent adaptation (MCU, toolchain, or runtime environment).

## Should contain
- code tied to the porting layer,
- adaptations required to run on this platform,
- details that would change if the target or base environment changes.

## Should NOT contain
- application logic,
- use cases,
- board-specific logic that clearly belongs in `../bsp/`,
- higher-level reusable services.

## Relationship with other folders
`src/port/` resolves platform dependencies.
It should not become a layer for product functional logic.

## Practical guideline
If the code exists because the platform or base environment changes, it likely belongs here.
