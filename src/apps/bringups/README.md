# bringups

Bring-up apps validate one hardware path, driver behavior, or human-observed
runtime capability in isolation.

They may require UART, LEDs, LCD glass, debugger state, or external equipment.
They do not need to end with automatic `PASS`/`FAIL`; that belongs under
`src/apps/tests/`.

Keep runtime UART state and telemetry records compact and within the debug
transport's intentional 96-byte per-call limit. Do not fragment long prose
across multiple transmissions; place detailed human procedures and arithmetic
in the bring-up contract or applicable documentation.

Current bringups:
- `debug_panic`: panic, fault, and debug-stop path calibration.
- `lcd`: LCD glass, numeric rows, decimals, indicators, and alpha characters.
- `lcd_blink`: LCD logical blink behavior.
- `display_format_lcd`: formatted numeric text rendered on the LCD.
- `keyboard_input`: keyboard GPIO EXTI short-press events.
- `pulse_counter`: LPTIM4 primary-pulse counting and stable raw observations
  across Run and the configured ThreadX idle Stop2 path.
- `frequency_observation`: LPTIM4 pulse deltas paired with extended LPTIM3
  elapsed time, observation-window quality, and human arithmetic for the
  independent 1 kHz temporal check.
