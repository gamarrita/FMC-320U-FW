/**
 * @file fm_backup.h
 * @brief Interface to configure the FMC backup SRAM domain.
 */

#ifndef FM_BACKUP_H_
#define FM_BACKUP_H_

// --- API ---

/**
 * Enables read/write access to backup SRAM while keeping its contents during standby.
 */
void FM_BACKUP_Init(void);

#endif // FM_BACKUP_H_

