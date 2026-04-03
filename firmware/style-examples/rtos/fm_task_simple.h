/*
 * @file    fm_task_simple.h
 * @brief   Simple periodic task example.
 */

#ifndef FM_TASK_SIMPLE_H_
#define FM_TASK_SIMPLE_H_

/* =========================== Includes ==================================== */
#include <stdint.h>
#include "tx_api.h"

/* =========================== Public Macros =============================== */
#define FM_TASK_SIMPLE_PERIOD_TICKS   (10u)

/* =========================== Public API ================================== */
void FM_TASK_SIMPLE_Init(void);
void FM_TASK_SIMPLE_Entry(ULONG thread_input);

#endif /* FM_TASK_SIMPLE_H_ */

/*** end of file ***/