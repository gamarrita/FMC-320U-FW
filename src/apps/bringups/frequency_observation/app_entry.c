/**
 * @file    app_entry.c
 * @brief   Adapter from the shared ThreadX harness to frequency bring-up.
 */

#include "app_entry.h"

#include "fm_frequency_observation_bringup.h"

void APP_ENTRY_Run(void)
{
    FM_FrequencyObservationBringup_Run();
}
