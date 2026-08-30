/**
 * @file    fm_main_ui.h
 * @brief   RTOS-neutral product-main coordination of FMC runtime and UI.
 *
 * This app-local module obtains coherent runtime snapshots, routes semantic
 * input directly to `fmc_ui`, and executes the bounded Phase 8 UI request
 * vocabulary. It owns no hardware, RTOS object, queue, timer, or LCD mapping.
 */
#ifndef FM_MAIN_UI_H
#define FM_MAIN_UI_H

#include <stdbool.h>

#include "fm_status.h"
#include "fmc_input.h"
#include "fmc_runtime.h"
#include "fmc_ui.h"

typedef struct
{
    fmc_ui_state_t previous_state;
    fmc_ui_state_t current_state;
    bool presentation_performed;
    bool acm_reset_executed;
} fm_main_ui_result_t;

/**
 * @brief Compose one UI snapshot from a coherent serialized runtime state.
 *
 * @return Runtime snapshot/getter status unchanged.
 */
fm_status_t FM_MAIN_UI_MakeSnapshot(
    const fmc_runtime_t *p_runtime,
    fmc_ui_snapshot_t *p_snapshot);

/**
 * @brief Route one semantic input to UI and execute its request exactly once.
 *
 * Navigation is presented immediately by `fmc_ui` from a fresh snapshot. A
 * RESET_ACM request dispatches one runtime reset, obtains a new post-reset
 * snapshot, and presents ACM/RATE once. Successfully presented user-menu
 * state clears the runtime presentation-pending flag.
 *
 * @param p_runtime Serialized runtime owner.
 * @param p_ui Initialized UI instance.
 * @param p_input Semantic input to route directly.
 * @param p_result Caller-owned operation result, initialized on entry.
 *
 * @return `FM_STATUS_OK` on success.
 * @return Dependency status unchanged on validation, snapshot, UI, reset, or
 *         presentation failure.
 */
fm_status_t FM_MAIN_UI_HandleInput(
    fmc_runtime_t *p_runtime,
    fmc_ui_t *p_ui,
    const fmc_input_event_t *p_input,
    fm_main_ui_result_t *p_result);

/**
 * @brief Present the active user screen from one fresh runtime snapshot.
 *
 * On success the runtime presentation-pending flag is acknowledged. Startup
 * states are rejected by the underlying UI contract.
 *
 * @return Dependency status unchanged.
 */
fm_status_t FM_MAIN_UI_Refresh(
    fmc_runtime_t *p_runtime,
    fmc_ui_t *p_ui);

#endif /* FM_MAIN_UI_H */
