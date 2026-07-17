# Keyboard Input Bring-Up

## Purpose

This context tracks the first keyboard/input slice.

The goal is to validate the physical key inputs and define an input event
boundary that can later be used by the product runtime without making most code
care whether the system runs bare metal or under ThreadX.

## Selected Direction

Use the recommended staged path:
1. human configures keyboard GPIO/EXTI in CubeMX - done
2. agent reviews generated changes - done
3. agent adds a short-press-only bring-up - done
4. agent keeps the event path RTOS-neutral - done

Do not enable ThreadX just for the first short-press keyboard bring-up.

## Legacy Evidence

Primary source:
- `legacy/source/FLOWMEET/fmx.c`

Relevant legacy behavior:
- `HAL_GPIO_EXTI_Falling_Callback()` emits short key events on release
- `HAL_GPIO_EXTI_Rising_Callback()` arms the long-press timer
- `TimerEntryKeyThreeSeconds()` emits long-press events through ThreadX timers
- skip flags prevent a long press from also producing a short press
- external keys used a firmware debounce timer
- events are delivered through a ThreadX queue consumed by the main UI thread

Decision for this slice:
- preserve the short-press-on-release product behavior
- defer long press and all timer behavior
- do not copy ThreadX queue calls into the HAL callback
- do not copy firmware debounce unless hardware evidence shows it is needed

## CubeMX Configuration Used

The current `fmc-320u-v2.ioc` now configures:
- PE10 `KEY_DOWN`
- PE11 `KEY_UP`
- PE12 `KEY_ENTER`
- PE13 `KEY_ESC`
- EXTI/NVIC on falling edge for the release/short-press event

Not configured in this slice:
- PD3 `KEY_EXT_1`
- PD4 `KEY_EXT_2`
- PE0 `LED_BACKLIGHT`

Use `legacy/100_main.ioc` only as comparison evidence. It shows:
- PE10 `KEY_DOWN`
- PE11 `KEY_UP`
- PE12 `KEY_ENTER`
- PE13 `KEY_ESC`
- PD3 `KEY_EXT_1`
- PD4 `KEY_EXT_2`
- PE0 `LED_BACKLIGHT`

Do not hand-edit `.ioc` or generated init code.

## Implemented Scope

Implemented:
- short press event recognition
- UART reporting from a bring-up app
- a generic `port` EXTI adapter
- a board keyboard BSP mapping
- a regression case for CubeMX pin-to-key mapping
- `fm_debug` ISR-safe logging from the IRQ path

Do not implement yet:
- long press
- firmware debounce
- ThreadX queues
- ThreadX timers
- menu navigation
- backlight timeout behavior
- product runtime integration

## Intended Layering

Current shape:

```text
CubeMX EXTI IRQ path
  -> HAL_GPIO_EXTI_Falling_Callback()
  -> fm_port_gpio_exti
  -> fm_board_keyboard
  -> FM_DEBUG_LogConstISR()
  -> bring-up app FM_DEBUG_Flush()
```

This validates the physical path without making product code depend on ThreadX.
A future product input service can replace the `fm_debug` bring-up observation
with an RTOS-neutral event API or a ThreadX-backed queue.

## ThreadX Decision

ThreadX is deferred for this slice.

Introduce ThreadX when at least one of these becomes active:
- long press timers
- menu refresh timing
- backlight timeout timing
- product UI runtime thread
- concurrent communication workflow
- pulse acquisition/runtime coordination

Before enabling ThreadX in `fmc-320u-v2.ioc`, expect generated startup changes
that may affect all existing apps. When that transition happens, bring-ups
should become simple ThreadX app threads rather than relying on manual edits to
generated `main.c`.

## Validation Result

Completed:
- canonical build for `tests/regression`
- canonical build for `bringups/keyboard_input`
- human hardware validation of the bring-up
- UART observation of short events for the four configured keys

Expected UART style:

```text
KEY_INPUT_BRINGUP:START
KEY_INPUT_BRINGUP:READY KEYS=DOWN,UP,ENTER,ESC EDGE=FALLING
KEY_INPUT_BRINGUP:KEY=ESC EDGE=FALLING
KEY_INPUT_BRINGUP:KEY=ENTER EDGE=FALLING
KEY_INPUT_BRINGUP:KEY=UP EDGE=FALLING
KEY_INPUT_BRINGUP:KEY=DOWN EDGE=FALLING
```

## Remaining Decisions

- where to introduce the product input service
- when to enable ThreadX
- how to model long press and skip-short-after-long behavior
- whether `KEY_EXT_1`, `KEY_EXT_2`, and backlight should be configured in the
  next keyboard/UI-related CubeMX pass
