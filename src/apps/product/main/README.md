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

## Boundaries

Do not place pure product behavior here. Product rules and runtime contracts
belong under `src/product/fmc/`.

Do not place board pin, HAL, GPIO, EXTI, CubeMX, ThreadX, queue, or timer
details in product contracts.

For the current input slice, this folder only adapts mechanical board keys
`DOWN`, `UP`, `ENTER`, and `ESC` into provisional FMC runtime input events with
action `SHORT`.

This layer does not recognize short versus long presses. It does not observe
press/release edges, debounce, hold timing, timers, ISR delivery, wake,
backlight, or menu consequences. A later input recognizer may replace the
provisional `SHORT` producer without changing the FMC semantic input contract.

External buttons `EXT_1` and `EXT_2` are out of scope until selected
explicitly.
