# bringups

Bring-up apps validate one hardware path, driver behavior, or human-observed
runtime capability in isolation.

They may require UART, LEDs, LCD glass, debugger state, or external equipment.
They do not need to end with automatic `PASS`/`FAIL`; that belongs under
`src/apps/tests/`.

Current bringups:
- `debug_panic`: panic, fault, and debug-stop path calibration.
- `lcd`: LCD glass, numeric rows, decimals, indicators, and alpha characters.
- `lcd_blink`: LCD logical blink behavior.
- `display_format_lcd`: formatted numeric text rendered on the LCD.
- `keyboard_input`: keyboard GPIO EXTI short-press events.
