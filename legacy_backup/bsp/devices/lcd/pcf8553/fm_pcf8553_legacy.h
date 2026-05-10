/**
 * @file    fm_pcf8553_legacy.h
 * @brief   Frozen legacy PCF8553 backend interface.
 *
 * This header preserves the historical controller path as a legacy backup
 * while the redesigned backend evolves independently in `fm_pcf8553.h`.
 */

#ifndef FM_PCF8553_LEGACY_H_
#define FM_PCF8553_LEGACY_H_

#include <stdint.h>

#define FM_PCF8553_LEGACY_RAM_SIZE         20U
#define FM_PCF8553_LEGACY_SEGMENTS_ON      0xFFU
#define FM_PCF8553_LEGACY_SEGMENTS_OFF     0x00U

extern uint8_t g_fm_pcf8553_legacy_ram_map[FM_PCF8553_LEGACY_RAM_SIZE];

void FM_PCF8553_LEGACY_ClearBuffer(void);
void FM_PCF8553_LEGACY_Init(void);
void FM_PCF8553_LEGACY_Refresh(void);
void FM_PCF8553_LEGACY_Reset(void);
void FM_PCF8553_LEGACY_WriteAll(uint8_t p_data);
void FM_PCF8553_LEGACY_WriteByte(uint8_t p_address, uint8_t p_data);

#endif /* FM_PCF8553_LEGACY_H_ */

/*** end of file ***/
