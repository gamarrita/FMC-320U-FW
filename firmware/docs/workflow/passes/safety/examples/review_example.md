# Example: Safety Review Output

This is an illustrative example.
It is not a required output template.

Reviewed scope:
- `src/port/fm_port_rtc.c`
- `src/libs/fm_debug.c`

Findings:
- `HAL_RTCEx_WakeUpTimerEventCallback()` forwards into project code from IRQ context
- the project path is thin, but the review should confirm that no blocking transport is called there

Risk:
- if blocking or formatting work leaks into the IRQ path later, wakeup handling can become timing-sensitive and misleading to debug

Recommendation:
- keep the HAL callback thin
- keep the project IRQ hook non-blocking
- keep any UART formatting or transmit work in foreground flush paths

Validation limit:
- code review confirms the current forwarding shape
- runtime timing behavior is not fully validated without target observation

Out of scope:
- renaming RTC symbols
- broader RTC architecture cleanup

Why this stays small:
- the finding is about execution context and ownership
- it does not require redesign of unrelated RTC behavior
