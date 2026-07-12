/*
 * Autor: Daniel H Sagarra
 *
 * Notas:
 *
 */

#ifndef FMX_LP_H_
#define FMX_LP_H_

// includes
#include "main.h"
#include "tx_api.h"

// Typedef y enum.

// Macros, defines, microcontroller pins (dhs).

// Varibles extern

// Defines.

// Function prototypes
ULONG FMX_LP_Adjust(void);
void FMX_LP_Delay(ULONG countery);
void FMX_LP_Enter(void);
void FMX_LP_Exit(void);
void FMX_LP_Setup(ULONG count);

#endif /* MODULE_H */

/*** end of file ***/
