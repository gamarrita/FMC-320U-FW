/*
 *
 * Instrucciones:
 * - #include "fm_main.h // copiar esta linea en main.c
 * - Nueva carpeta con el nombre FLOWMEET en el workspace
 * - Hacer que la carpeta FLOWWMEET ser "source floder"
 * - En las propiedades del proyecto la carpeta FLOWMEET de ser "include paths" en C/C++ Build->Settings MCU/MPU GVV Compiler
 * - Agregar la sentencia FM_MAIN_Main(); en el while infinico del main.c
 */

#ifndef FM_USART_H_
#define FM_USART_H_

// includes
#include "main.h"
#include "usart.h"
#include "fmx.h"


// Defines
#define FM_USART_RX3_BUF_SIZE 1024 // Cantidad máxima de bytes en un ThreadX queue msg
#define FM_USART_TX3_BUF_SIZE 1024 // Cantidad máxima de bytes en un ThreadX queue msg

// Typedef

// Enum.

// Defines, typedef, enum

// Macros, defines, microcontroller pins (dhs).

// Varibles extern
extern char fm_usart_rx3_buf[FM_USART_RX3_BUF_SIZE];
extern char fm_usart_tx3_buf[FM_USART_RX3_BUF_SIZE];


// Defines.

// Prototipos
fmx_status_t FM_USART_Uart3PowerOn();
fmx_status_t FM_USART_Uart3TransmitDma(const char *m, UINT wait_ms);
void FM_USART_RtosInit(VOID *memory_ptr);



#endif  // FM_MAIN_H

/*** end of file ***/
