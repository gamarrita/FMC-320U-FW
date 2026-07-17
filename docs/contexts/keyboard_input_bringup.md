# Keyboard Input Bring-Up

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

## Closed Result

Validated on hardware:
- PE10 `KEY_DOWN`
- PE11 `KEY_UP`
- PE12 `KEY_ENTER`
- PE13 `KEY_ESC`
- falling-edge EXTI short-press observation over UART
- `port` EXTI adapter and board keyboard BSP mapping

Not configured in this slice:
- PD3 `KEY_EXT_1`
- PD4 `KEY_EXT_2`
- PE0 `LED_BACKLIGHT`

## Intended Layering

```text
CubeMX EXTI IRQ path
  -> HAL_GPIO_EXTI_Falling_Callback()
  -> fm_port_gpio_exti
  -> fm_board_keyboard
  -> FM_DEBUG_LogConstISR()
  -> bring-up app FM_DEBUG_Flush()
```

This validates the physical path without making product code depend on ThreadX.
A future product input service can replace `fm_debug` observation with an
RTOS-neutral event API or a ThreadX-backed queue.

## Deferred

- product input service
- ThreadX queues/timers
- long press and skip-short-after-long behavior
- external buttons and LCD backlight
