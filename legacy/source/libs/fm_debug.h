/**
 * @file fm_debug.h
 * @brief Debug utilities for the FMC-320U firmware.
 */

#ifndef FM_DEBUG_H_
#define FM_DEBUG_H_

#include "main.h"
#include <string.h>
#include <stdio.h>

/** Available actions for the diagnostic LEDs. */
typedef enum {
    FM_DEBUG_LED_OFF,
    FM_DEBUG_LED_ON,
    FM_DEBUG_LED_TOGGLE,
} fm_debug_led_t;

// --- API ---

void FM_DEBUG_Init(void);
void FM_DEBUG_ItmMsg(const char *msg, uint8_t len);
void FM_DEBUG_LedActive(int status);
void FM_DEBUG_LedError(int status);
void FM_DEBUG_LedSignal(int status);
void FM_DEBUG_UartMsg(const char *p_msg, uint8_t len);
void FM_DEBUG_UartUint8(uint8_t num);
void FM_DEBUG_UartUint16(uint16_t num);
void FM_DEBUG_UartUint32(uint32_t num);
void FM_DEBUG_UartInt32(int32_t num);
void FM_DEBUG_UartFloat(float num);

#endif // FM_DEBUG_H_

