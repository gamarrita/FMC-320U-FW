# product main app

This folder contains the current FMC product application entrypoint and the
application-level wiring needed to connect board services to product runtime
contracts.

## Ownership

This layer may know both:
- board-facing contracts from `src/bsp/`;
- product runtime contracts from `src/product/fmc/`.

Use this folder for small product-app composition code such as translating an
already identified board keyboard key into an FMC runtime input event.

Current keyboard flow:

```text
GPIO EXTI IRQ
-> port GPIO EXTI callback
-> board keyboard maps pin to board key and edge
-> product/main keyboard callback sends an app-level event to a ThreadX queue
-> product/main runtime owner thread receives the event
-> fm_main_input_adapter converts the board key to a provisional FMC SHORT
-> FMC_RUNTIME_Dispatch()
```

## Boundaries

Do not place pure product behavior here. Product rules and runtime contracts
belong under `src/product/fmc/`.

Do not place board pin, HAL, GPIO, EXTI, CubeMX, ThreadX, queue, or timer
details in product contracts.

For the current input slice, this folder owns the app-level ThreadX keyboard
queue and one dedicated owner thread for `fmc_runtime`. It adapts mechanical
board keys `DOWN`, `UP`, `ENTER`, and `ESC` into provisional FMC runtime input
events with action `SHORT`.

This layer does not implement final short versus long recognition. It observes
the current board keyboard edge only to produce provisional `SHORT` events.
Debounce, hold timing, timers, wake, backlight, and menu consequences remain
out of scope. A later input recognizer may replace the provisional `SHORT`
producer without changing the FMC semantic input contract.

External buttons `EXT_1` and `EXT_2` are out of scope until selected
explicitly.
