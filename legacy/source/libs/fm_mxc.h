/*
 *
 * Instrucciones:
 * - #include "fm_main.h // copiar esta linea en main.c
 * - Nueva carpeta con el nombre FLOWMEET en el workspace
 * - Hacer que la carpeta FLOWWMEET ser "source floder"
 * - En las propiedades del proyecto la carpeta FLOWMEET de ser "include paths" en C/C++ Build->Settings MCU/MPU GVV Compiler
 * - Agregar la sentencia FM_MAIN_Main(); en el while infinico del main.c
 */

#ifndef FM_MXC_H_
#define FM_MXC_H_

// includes
#include "main.h"
#include "fmx.h"
#include "tx_api.h"

// Typedef y enum.

typedef enum
{
    FM_MXC_MODE_OFF,  // MXChip sin alimentación, el modulo consume 0 uA.
    FM_MXC_MODE_ON,  // MXChip alimentado 63mA de consumo en modo activo.
    FM_MXC_MODE_DISABLE,  // MXChip alimentado pero en modo disable.
    FM_MXC_MODE_ENABLE  // RMXChip alimentado pero en modo enable.
} fm_mxc_mode_t;

// Macros, defines, microcontroller pins (dhs).

// Varibles extern

// Function prototypes
void FM_MXC_Mode(fm_mxc_mode_t mode);
fmx_status_t FM_MXC_ConnectMaster();
fmx_status_t FM_MXC_ConnectSlave();
void FM_MXC_PowerOn();
void FM_MXC_PowerOff();
void FM_MXC_ATMode();
void FM_MXC_Sleep();
void FM_MXC_Wakeup();

#endif  // FM_MAIN_H

/*** end of file ***/
