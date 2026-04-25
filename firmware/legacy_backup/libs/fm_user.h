/*
 * Autor: Daniel H Sagarra
 *
 * Notas:
 *
 */

#ifndef FM_USER_H_
#define FM_USER_H_

// includes
#include "fm_fmc.h"
//#include "fmx.h"
#include "tx_api.h"

// Typedef y enum.

// Macros, defines, microcontroller pins (dhs).

// Varibles extern, declaradas en fm_module.c y usadas en otros módulos.

// Defines.

// Function prototypes
uint8_t FM_USER_MenuNav(fmx_events_t new_event);

// Entrada del hilo de conexión esclava Bluetooth
void FM_USER_ThreadEntryBluetoothSlave(ULONG input);

#endif /* MODULE_H */

/*** end of file ***/
