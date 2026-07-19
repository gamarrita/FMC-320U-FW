/**
 * @file    fm_app_threadx.c
 * @brief   ThreadX harness for the selected firmware app.
 */
#include "fm_app_threadx.h"

#include <stddef.h>

#include "app_entry.h"
#include "fm_debug.h"

#define FM_APP_THREADX_STACK_SIZE_BYTES       (4096U)
#define FM_APP_THREADX_PRIORITY               (10U)
#define FM_APP_THREADX_PREEMPTION_THRESHOLD   (10U)
#define FM_APP_THREADX_TIME_SLICE             (TX_NO_TIME_SLICE)

static TX_THREAD g_fm_app_threadx_thread;

static void fm_app_threadx_entry_(ULONG input);

UINT FM_APP_THREADX_Init(VOID *memory_ptr)
{
    TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL *) memory_ptr;
    VOID *stack_ptr = NULL;
    UINT status;

    if (byte_pool == NULL)
    {
        return TX_PTR_ERROR;
    }

    status = tx_byte_allocate(byte_pool,
                              &stack_ptr,
                              FM_APP_THREADX_STACK_SIZE_BYTES,
                              TX_NO_WAIT);
    if (status != TX_SUCCESS)
    {
        return status;
    }

    status = tx_thread_create(&g_fm_app_threadx_thread,
                              "FM_APP",
                              fm_app_threadx_entry_,
                              0U,
                              stack_ptr,
                              FM_APP_THREADX_STACK_SIZE_BYTES,
                              FM_APP_THREADX_PRIORITY,
                              FM_APP_THREADX_PREEMPTION_THRESHOLD,
                              FM_APP_THREADX_TIME_SLICE,
                              TX_AUTO_START);

    return status;
}

static void fm_app_threadx_entry_(ULONG input)
{
    (void) input;

    APP_ENTRY_Run();

    FM_DEBUG_PanicMsg("APP_THREADX:APP_ENTRY_RETURNED");
}
