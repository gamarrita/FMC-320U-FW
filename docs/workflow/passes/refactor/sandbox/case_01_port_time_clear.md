# Case 01: Clear Fresh Refactor

## User-Style Request

Necesito un refactor chico en `src/port/fm_port_time.*`.
Quiero mantener el comportamiento actual de `FM_PORT_TIME_SleepMs()` basado en
`HAL_Delay()`, pero dejar mejor separado el punto donde vive el helper de sleep
de cualquier futura consulta de tiempo.
Fuera de alcance: el resto de `src/port/`.
El siguiente paso puede ser revisar `src/port/fm_port_time.*` y buscar sus call
sites.

## Safe Repo Evidence

- `src/port/fm_port_time.c` hoy expone un helper mínimo de sleep en plataforma
- `src/port/README.md` define `src/port/` como adaptación dependiente de plataforma
- el pedido ya nombra target, comportamiento a preservar, problema
  estructural, límite fuera de alcance y siguiente paso

## Wizard Decision

Bootstrap immediately.

No blocking question is needed.

## Checklist-Oriented Outcome

Expected verdict:
- approved

Why:
- active work is real and current
- next step is executable
- scope is explicit
- lateral boundary is explicit
- preserved behavior is explicit enough to avoid redesign

## Lesson

The wizard must allow a zero-question bootstrap when the user request already
satisfies the minimum input set.
