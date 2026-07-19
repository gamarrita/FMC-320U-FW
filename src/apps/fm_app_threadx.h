/**
 * @file    fm_app_threadx.h
 * @brief   ThreadX harness for the selected firmware app.
 *
 * This module adapts the repository's selectable `APP_ENTRY_Run()` app model
 * to the CubeMX ThreadX bootstrap. It creates one generic app task and runs
 * the selected app entry from that task.
 */
#ifndef FM_APP_THREADX_H
#define FM_APP_THREADX_H

#include "tx_api.h"

/**
 * @brief Create the ThreadX task that runs the selected app entry.
 *
 * @param memory_ptr Pointer supplied by CubeMX ThreadX startup. The current
 *        implementation expects a `TX_BYTE_POOL *`.
 *
 * @return `TX_SUCCESS` when the app task was created.
 * @return ThreadX error status from resource creation on failure.
 */
UINT FM_APP_THREADX_Init(VOID *memory_ptr);

#endif /* FM_APP_THREADX_H */
