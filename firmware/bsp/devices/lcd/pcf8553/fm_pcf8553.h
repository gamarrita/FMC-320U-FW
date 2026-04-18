/**
 * @file    fm_pcf8553.h
 * @brief   Public device-level API for the PCF8553 LCD driver.
 *
 * @details
 *  - This module belongs to the commercial FMC-320U firmware product line and
 *    targets the same board hardware used by the previous firmware.
 *  - The target MCU in this repository is STM32U575VIT6.
 *  - This repository is a near-from-scratch firmware rewrite that keeps the
 *    same user-visible product behavior while replacing the former
 *    STM32CubeIDE/Eclipse-oriented project structure with a VS Code + CMake
 *    workflow.
 *  - The hardware has not changed; the migration is architectural and
 *    tooling-oriented.
 *  - This driver file is transitional. It was inherited from the previous
 *    firmware codebase and is being adapted to the layer model of the current
 *    repository.
 *  - Near-term goal: enable a minimal PCF8553 bring-up on the current board.
 *  - Expected destination: device logic stays in `bsp/devices/lcd/pcf8553/`,
 *    while MCU transport details move behind dedicated `port/` backends.
 *  - Because repository modules will be progressively reworked by agents, this
 *    header intentionally documents migration context to reduce false
 *    assumptions during refactors.
 */

#ifndef FM_PCF8553_H_
#define FM_PCF8553_H_

/* Includes. */

#include <stdint.h>

/* Defines. */

/*
 * Tamaño de la memoria interna del pcf8553 para controlar el encendido/apagado
 * de los segmentos, con 20 bytes se controlan 20 * 8 =  160 segmentos.
 *
 */
#define PCF8553_RAM_SIZE         20U
#define PCF8553_SEGMENTS_ON      0xFFU
#define PCF8553_SEGMENTS_OFF     0x00U

/* Extern. */

extern uint8_t pcf8553_ram_map[PCF8553_RAM_SIZE];

/* Public function prototypes. */

void FM_PCF8553_ClearBuffer();
void FM_PCF8553_Init();
void FM_PCF8553_Refresh();
void FM_PCF8553_Reset();
void FM_PCF8553_WriteAll(uint8_t data);
void FM_PCF8553_WriteByte(uint8_t add, uint8_t data);

#endif /* FM_PCF8553_H_ */

/*** end of file ***/
