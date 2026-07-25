/**
 * @file    app_entry.c
 * @brief   Adapter from the shared ThreadX harness to pulse-counter bring-up.
 */

#include "app_entry.h"

#include "fm_pulse_counter_bringup.h"

void APP_ENTRY_Run(void)
{
    FM_PulseCounterBringup_Run();
}
