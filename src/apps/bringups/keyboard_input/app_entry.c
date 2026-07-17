/**
 * @file    app_entry.c
 * @brief   Adapter from the generated app entrypoint to the keyboard bring-up.
 */
#include "app_entry.h"

#include "fm_keyboard_input_bringup.h"

void APP_ENTRY_Run(void)
{
    FM_KeyboardInputBringup_Run();
}
