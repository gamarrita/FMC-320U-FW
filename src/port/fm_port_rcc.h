/**
 * @file    fm_port_rcc.h
 * @brief   Platform RCC helpers for system clock configuration.
 *
 * This module owns the authored system clock configuration used by the current
 * firmware profile. CubeMX remains the hardware-configuration source of truth;
 * this wrapper applies the repository-facing clock setup.
 */

#ifndef FM_PORT_RCC_H
#define FM_PORT_RCC_H

/**
 * @brief   Configure the system clock tree for the active platform.
 *
 * Sets voltage scaling, enables backup-domain access/LSE drive, configures MSI
 * and LSE, and selects the active bus clock dividers. HAL failures stop in
 * `Error_Handler()`.
 *
 * Call during early foreground startup before peripherals that depend on the
 * configured clock tree.
 */
void FM_PORT_RCC_Init(void);

#endif /* FM_PORT_RCC_H */
