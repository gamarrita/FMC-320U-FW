/*
 * @file    fmx_kernel_basic.h
 * @brief   Ejemplo mínimo de integración con ThreadX.
 */

#ifndef FMX_KERNEL_BASIC_H_
#define FMX_KERNEL_BASIC_H_

/* =========================== Includes ==================================== */
#include <stdint.h>
#include "tx_api.h"

/* =========================== Public Defines ============================== */
#define FMX_KERNEL_BASIC_THREAD_STACK_BYTES   (1024u)
#define FMX_KERNEL_BASIC_THREAD_PRIO          (5u)
#define FMX_KERNEL_BASIC_QUEUE_DEPTH          (8u)

/* =========================== Public Types ================================ */
typedef struct
{
    TX_THREAD tcb_main;
    TX_MUTEX  mcb_lock;
    TX_QUEUE  qcb_events;
} fmx_kernel_basic_objs_t;

/* =========================== Public API ================================== */
void FMX_KERNEL_BASIC_Init(fmx_kernel_basic_objs_t *objs,
                           void (*entry_main)(ULONG),
                           void *stack_ptr,
                           ULONG stack_size,
                           UINT priority);

#endif /* FMX_KERNEL_BASIC_H_ */

/*** end of file ***/