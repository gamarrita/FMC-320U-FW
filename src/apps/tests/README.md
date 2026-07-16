# tests

Automatic regression apps live here.

These apps should be deterministic, runnable from the canonical build flow, and
report clear `PASS`/`FAIL` output over UART without requiring human inspection
of the LCD or debugger.

Current tests:
- `regression`: shared firmware regression harness for pure authored modules.
