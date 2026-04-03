/*
 * @file    fm_main_simple.h
 * @brief   Ejemplo mínimo de aplicación baremetal.
 */

#ifndef FM_MAIN_SIMPLE_H_
#define FM_MAIN_SIMPLE_H_

/* =========================== Includes ==================================== */
#include <stdint.h>
#include <stdbool.h>

/* =========================== Public Macros =============================== */
#define FM_MAIN_SIMPLE_LOOP_PERIOD_MS      (10u)
#define FM_MAIN_SIMPLE_HEARTBEAT_PERIOD_MS (1000u)

/* =========================== Public Types ================================ */
typedef enum
{
    FM_MAIN_SIMPLE_OK = 0,
    FM_MAIN_SIMPLE_ERROR
} fm_main_simple_status_t;

/* =========================== Public API ================================== */
void FM_MAIN_SIMPLE_Init(void);
void FM_MAIN_SIMPLE_Main(void);

#endif /* FM_MAIN_SIMPLE_H_ */

/*** end of file ***/