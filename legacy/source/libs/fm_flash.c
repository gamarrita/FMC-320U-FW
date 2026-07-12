/**
 * @file fm_flash.c
 * @brief Flash access routines for the non-volatile storage window.
 */

#include "main.h"
#include "fm_flash.h"
#include "fm_debug.h"

// --- Memory layout ---

#define FLASH_START             (0x08100000u)
#define FLASH_END               (0x081FFFFFu)
#define FLASH_CHIP_INFO_START   (0x08100000u)
#define FLASH_CHIP_INFO_END     (0x08101FFFu)
#define FLASH_DEVICE_START      (0x08102000u)
#define FLASH_DEVICE_END        (0x08103FFFu)

#define PAGE_SIZE               FLASH_PAGE_SIZE

// --- Persistent data ---

static flash_chip_info_t chip_info __attribute__((section(".FLASH_CHIP_Section"))) = {
    .reset_counter = 0,
    .reset_factory = 0,
};

// --- API ---

/**
 * Reads the chip information structure stored in Flash.
 */
flash_chip_info_t FM_FLASH_ChipInfoRead(void)
{
    flash_chip_info_t info;
    FM_FLASH_Read(FLASH_CHIP_INFO_START, info.data, FM_FLASH_CHIP_INFO_SIZE);
    return info;
}

/**
 * Writes the chip information structure back to Flash.
 * @param info Structure copy to persist.
 */
void FM_FLASH_ChipInfoWrite(flash_chip_info_t info)
{
    FM_FLASH_Write(FLASH_CHIP_INFO_START, info.data, FM_FLASH_CHIP_INFO_SIZE);
}

/**
 * Increments and returns the MCU reset counter.
 */
uint16_t FM_FLASH_NewReset(void)
{
    flash_chip_info_t info = FM_FLASH_ChipInfoRead();
    info.reset_counter++;
    FM_FLASH_ChipInfoWrite(info);
    return info.reset_counter;
}

/**
 * Writes to the Flash window emulating non-volatile storage.
 * @param address Absolute address to start writing.
 * @param data Pointer to the data block (must align to FM_FLASH_BLOCK_SIZE).
 * @param data_length Number of bytes to program.
 * @return Number of bytes written.
 */
uint32_t FM_FLASH_Write(uint32_t address, const uint8_t *data, uint16_t data_length)
{
    uint8_t quad_word[FM_FLASH_BLOCK_SIZE];
    uint32_t error_status = 0;
    uint16_t aligned_length = data_length - (data_length % FM_FLASH_BLOCK_SIZE);

    FLASH_EraseInitTypeDef erase_cfg = {0};

    if (address < FLASH_START || (address + aligned_length) > FLASH_END) {
        FM_DEBUG_LedError(1);
        return 0;
    }

    if ((address % FM_FLASH_BLOCK_SIZE) != 0u || aligned_length == 0u) {
        FM_DEBUG_LedError(1);
        return 0;
    }

    erase_cfg.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_cfg.Page = (address - FLASH_START) / FLASH_PAGE_SIZE;
    erase_cfg.NbPages = (aligned_length % FLASH_PAGE_SIZE) ?
                        ((aligned_length / FLASH_PAGE_SIZE) + 1u) :
                        (aligned_length / FLASH_PAGE_SIZE);
    erase_cfg.Banks = FLASH_BANK_2;

    HAL_FLASH_Unlock();
    HAL_FLASHEx_Erase(&erase_cfg, &error_status);

    for (uint32_t offset = 0; offset < aligned_length; ++offset) {
        quad_word[offset % FM_FLASH_BLOCK_SIZE] = data[offset];

        if (((offset + 1u) % FM_FLASH_BLOCK_SIZE) == 0u) {
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD,
                              address + offset + 1u - FM_FLASH_BLOCK_SIZE,
                              (uint32_t)quad_word);
        }
    }

    HAL_FLASH_Lock();
    return aligned_length;
}

/**
 * Reads bytes from Flash into RAM.
 * @param address Base address in Flash.
 * @param data Destination buffer in RAM.
 * @param data_length Number of bytes to copy.
 * @return Number of bytes copied.
 */
uint32_t FM_FLASH_Read(uint32_t address, uint8_t *data, uint16_t data_length)
{
    for (uint16_t i = 0; i < data_length; ++i) {
        data[i] = *(const uint8_t *)(address + i);
    }
    return data_length;
}

