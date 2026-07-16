#include "app_entry.h"

#include "fm_debug_panic_bringup.h"

void APP_ENTRY_Run(void)
{
    FM_DebugPanicBringup_Run();
}
