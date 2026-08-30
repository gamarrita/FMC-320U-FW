# product main app

This folder contains the current FMC product application entrypoint and the
application-level wiring needed to connect board services to product runtime
contracts.

## Ownership

This layer may know both:
- board-facing contracts from `src/bsp/`;
- product runtime contracts from `src/product/fmc/`.

Use this folder for small product-app composition code such as translating an
already identified board keyboard key into an FMC semantic input event and
coordinating that input with the serialized runtime/UI owner.

Current keyboard flow:

```text
GPIO EXTI IRQ
-> port GPIO EXTI callback
-> board keyboard maps pin/polarity to board key and PRESSED/RELEASED
-> product/main queues both mechanical edges, but admits only the first enabled
   EXT_1/EXT_2 press and suppresses its raw release/bounce edges
-> FM_MAIN_Main() receives the event in the existing FM_APP ThreadX thread
-> mechanical keys use fm_main_input_recognizer for SHORT/LONG
-> EXT_1/EXT_2 use independent fm_main_ext_button timers to require released
   samples one 100 ms interval apart before enabling the next press
-> accepted fmc_input_event_t is routed directly to fmc_ui
-> fm_main_ui executes a returned RESET_ACM request exactly once through runtime
```

Mechanical-key recognition currently uses the hardware-confirmed edge order:

```text
PRESSED
-> starts one active hold
-> arms one-shot 3 second ThreadX timer

timer expiry while the key is still active
-> timer callback publishes FM_MAIN_EVENT_KEY_HOLD_TIMEOUT
-> owner loop emits one FMC LONG input

RELEASED before LONG
-> owner loop cancels the timer
-> owner loop emits FMC SHORT input

RELEASED after LONG
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
-> reads a second independent stable LPTIM4 observation and LPTIM3 timestamp
-> fm_main_acquisition admits and dispatches any frequency result
-> fmc_runtime updates RATE value, presence, and quality without changing totals
-> optional compact UART evidence reports both totals and RATE quality
-> composes one live runtime snapshot and refreshes the active user-menu screen
```

The periodic timer is created inactive and starts only after the runtime,
independent observers, LPTIM3 time source, pulse counter, initial frequency
baseline, presentation, and keyboard path are initialized. Acquisition
failures, runtime dispatch failures, and canonical total overflow are fatal
product-contract violations. Diagnostic UART output is best-effort and never
participates in acquisition control flow.

Current presentation flow:

```text
successful LCD initialization
-> all software-controllable LCD segments for a nominal 3 seconds
-> provisional firmware version for a nominal 3 seconds
-> stable live TTL/RATE immediately and on each 1 second periodic refresh
```

The one-shot presentation timer publishes its timeout through the owner queue.
A SHORT ESC uses the same semantic transition while a temporary startup view
is active. Timer ownership remains in this app composition layer; semantic
frame composition belongs to `fmc_ui`.

The pure `fmc_ui` contract owns all five Phase 8 user states. Product main now
routes accepted semantic input directly to it, supplies fresh runtime
snapshots for immediate and periodic presentation, and executes the bounded
ACM-reset request through `fm_main_ui`.

External buttons are active-low physically but the BSP hides that polarity.
Each starts disarmed, emits SHORT on the first accepted press after arming, and
rearms only after its independently sampled released level remains stable for
100 ms. A button held at boot emits no action until it is later released,
stably rearmed, and pressed again.

## Boundaries

Do not place pure product behavior here. Product rules and runtime contracts
belong under `src/product/fmc/`.

Do not place board pin, HAL, GPIO, EXTI, CubeMX, ThreadX, queue, or timer
details in product contracts.

For the current input slice, this folder owns the app-level ThreadX owner
event queue and its app-owned timers. `FM_MAIN_Main()` runs inside the
existing `FM_APP` ThreadX thread and is the only owner of the live
`fmc_runtime_t` and `fmc_ui_t`. It adapts physical controls into FMC semantic
input with action `SHORT` or `LONG` and routes it directly to UI.

The 1 second periodic refresh event is also serialized through the owner queue
so ThreadX always has a temporal wake deadline for tickless/low-power support.
Phase 7G consumes it for independent pulse-delta and frequency observations,
canonical runtime updates, and live active-user-screen presentation. The future
indefinite inactive state and activity wake mechanism remain separate work.

Do not add another product thread unless there is a concrete concurrent
responsibility, such as presentation, acquisition, communication, or timer work
that cannot live in the owner loop cleanly.

This layer implements the minimal mechanical SHORT/LONG recognizer and the
accepted independent external-button stable-release debounce. Raw external
release and bounce edges are not owner-queue traffic: after one accepted press,
the app samples the level through each button's own 100 ms timer until release
has been observed across a complete interval.

The app also owns the ten-second backlight activity controller. Successful
all-segments startup presentation requests activation. Every normalized
physical `PRESSED` transition requests the same activation before mechanical
or external-button semantic filtering, so rejected, disarmed, and menu no-op
presses still illuminate the display. Producer-side activity is coalesced into
one pending owner request; the owner commits a wrap-safe deadline, restarts its
one-shot timer, and then turns the polarity-free board output on. The timer
callback turns it off directly only when the current deadline has expired.
A later richer recognizer may replace the semantic input implementations
without changing the FMC input or backlight activity contracts.
