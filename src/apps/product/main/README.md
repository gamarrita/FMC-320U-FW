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
-> FM_MAIN_Main() receives the event in the existing FM_APP ThreadX thread
-> fm_main_input_recognizer applies the mechanical SHORT/LONG policy
-> fm_main_input_adapter converts accepted input to a runtime event
-> FMC_RUNTIME_Dispatch()
```

Mechanical-key recognition currently uses the hardware-confirmed edge order:

```text
RISING
-> starts one active hold
-> arms one-shot 3 second ThreadX timer

timer expiry while the key is still active
-> timer callback publishes FM_MAIN_EVENT_KEY_HOLD_TIMEOUT
-> owner loop emits one FMC LONG input

FALLING before LONG
-> owner loop cancels the timer
-> owner loop emits FMC SHORT input

FALLING after LONG
-> owner loop cancels/clears hold state
-> no duplicate SHORT is emitted
```

Current periodic refresh flow:

```text
ThreadX periodic timer, 1 second
-> bounded timer callback publishes FM_MAIN_EVENT_PERIODIC_REFRESH
-> same product/main app-level queue
-> FM_MAIN_Main() receives the event in the existing FM_APP ThreadX thread
-> resamples message and LED debug jumpers
-> reads one stable LPTIM4 counter observation
-> fm_main_acquisition forms and dispatches exactly one pulse-delta event
-> fmc_runtime accepts zero as a no-op or asks fmc_service to update totals
-> fmc_service adds each accepted nonzero delta once to both ACM and TTL
-> optional UART evidence reports both canonical pulse totals
-> refreshes the current Phase 6A TTL/RATE snapshot
```

The periodic timer is created inactive and starts only after the runtime,
zero-baseline observer, pulse counter, presentation, and keyboard path are
initialized. Acquisition failures, runtime dispatch failures, and canonical
total overflow are fatal product-contract violations. Diagnostic UART output
is best-effort and never participates in acquisition control flow.

Current presentation flow:

```text
successful LCD initialization
-> all software-controllable LCD segments for a nominal 3 seconds
-> provisional firmware version for a nominal 3 seconds
-> stable TTL/RATE immediately and on each 1 second periodic refresh
```

The one-shot presentation timer publishes its timeout through the owner queue.
A SHORT ESC uses the same semantic transition while a temporary startup view
is active. Timer ownership remains in this app composition layer; semantic
frame composition belongs to `fmc_presentation`.

## Boundaries

Do not place pure product behavior here. Product rules and runtime contracts
belong under `src/product/fmc/`.

Do not place board pin, HAL, GPIO, EXTI, CubeMX, ThreadX, queue, or timer
details in product contracts.

For the current input slice, this folder owns the app-level ThreadX owner
event queue and the one-shot hold timer. `FM_MAIN_Main()` runs inside the
existing `FM_APP` ThreadX thread and is the only owner of the live
`fmc_runtime_t`. It adapts mechanical board keys `DOWN`, `UP`, `ENTER`, and
`ESC` into FMC runtime input events with action `SHORT` or `LONG`.

The 1 second periodic refresh event is also serialized through the owner queue
so ThreadX always has a temporal wake deadline for tickless/low-power support.
Phase 7C consumes it for pulse observation and totalization. Phase 6A still
consumes it for presentation updates, so the app continues to supply the
documented provisional TTL/RATE snapshot until later Phase 7 slices replace
those visible inputs.

Do not add another product thread unless there is a concrete concurrent
responsibility, such as presentation, acquisition, communication, or timer work
that cannot live in the owner loop cleanly.

This layer implements only the minimal short versus long recognizer. Debounce,
wake, backlight, and menu consequences remain out of scope. A later richer
input recognizer may replace this implementation without changing the FMC
semantic input contract.

External buttons `EXT_1` and `EXT_2` are out of scope until selected
explicitly.
