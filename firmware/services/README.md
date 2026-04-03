# services

## Intent
Contain reusable system services with a clear technical responsibility.

## Should contain
- reusable technical logic,
- services that mediate between lower layers and the application,
- modules with a clear interface and well-defined responsibility.

## Should NOT contain
- concrete physical board details,
- product use cases,
- generated or regenerable code,
- platform adaptation.

## Relationship with other folders
`services/` should act as a reusable technical layer.  
It may rely on `../bsp/`, `../port/`, or `../cube/` in a controlled way, but should not be polluted with application logic.

## Practical guideline
If it solves a technical capability of the system and can be reused by more than one application flow, it likely belongs here.