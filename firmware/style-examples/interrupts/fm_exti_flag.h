/*
 * @file    fm_exti_flag.h
 * @brief   Minimal interrupt event example.
 */

#ifndef FM_EXTI_FLAG_H_
#define FM_EXTI_FLAG_H_

/* =========================== Includes ==================================== */
#include <stdint.h>
#include <stdbool.h>

/* =========================== Public API ================================== */
void FM_EXTI_FLAG_Init(void);
void FM_EXTI_FLAG_OnInterrupt(void);
bool FM_EXTI_FLAG_GetAndClear(void);
uint32_t FM_EXTI_FLAG_GetCount(void);

#endif /* FM_EXTI_FLAG_H_ */
