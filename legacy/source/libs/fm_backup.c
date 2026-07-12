/**
 * @file fm_backup.c
 * @brief Brings up the backup SRAM domain and documents linker requirements.
 *
 * Backup SRAM remains powered from VBAT and needs the proper clocks, regulators,
 * and access rights enabled. The linker script must tag `.RAM_BACKUP_Section`
 * as NOLOAD so that programming tools do not overwrite its contents.
 */

#include "fm_backup.h"
#include "main.h"

// --- API ---

/**
 * Prepares backup SRAM for read/write access and keeps it retained in standby.
 *
 * The routine enables the PWR clock, switches the supply to LDO (required by the
 * backup domain), allows access to backup registers, and turns on SRAM retention
 * regardless of the main supply state.
 */
void FM_BACKUP_Init(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_BKPSRAM_CLK_ENABLE();

    HAL_PWREx_EnableBkupRAMRetention();
}

