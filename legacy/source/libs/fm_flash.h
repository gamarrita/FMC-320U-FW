/**
 * @file fm_flash.h
 * @brief Flash access helpers for configuration data and logging areas.
 */

#ifndef FM_FLASH_H_
#define FM_FLASH_H_

#include <stdint.h>

// --- Constants ---

#define FM_FLASH_BLOCK_SIZE        (16u)
#define FM_FLASH_CHIP_INFO_SIZE    (FM_FLASH_BLOCK_SIZE * 1u)

#define FM_FLASH_LOG_START         (0x08104000u)
#define FM_FLASH_LOG_END           (0x081FFFFFu)
#define FM_FLASH_LOG_SIZE          ((FM_FLASH_LOG_END - FM_FLASH_LOG_START) + 1u)

// --- Types ---

typedef union {
    uint8_t data[FM_FLASH_CHIP_INFO_SIZE];
    struct {
        uint16_t reset_counter;
        uint8_t  reset_factory;
        uint32_t reserved_3;
        uint32_t reserved_4;
    };
} flash_chip_info_t;

// --- API ---

flash_chip_info_t FM_FLASH_ChipInfoRead(void);
void FM_FLASH_ChipInfoWrite(flash_chip_info_t info);
uint16_t FM_FLASH_NewReset(void);
uint32_t FM_FLASH_Read(uint32_t address, uint8_t *data, uint16_t data_length);
uint32_t FM_FLASH_Write(uint32_t address, const uint8_t *data, uint16_t data_length);

#endif // FM_FLASH_H_

