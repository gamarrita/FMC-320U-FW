/**
 * @file    fm_port_pcf8553_ctrl.h
 * @brief   Platform control-line helpers for the PCF8553 LCD path.
 *
 * @details
 *  - This module owns the GPIO control signals associated with the PCF8553
 *    hardware path on the current board.
 *  - It intentionally stays separate from fm_port_spi1: the SPI1 module owns
 *    the bus instance, while this module owns auxiliary device control lines.
 *  - It also stays separate from fm_port_gpio so board-level debug GPIO does
 *    not turn into a dump of unrelated device-specific signals.
 */

#ifndef FM_PORT_PCF8553_CTRL_H
#define FM_PORT_PCF8553_CTRL_H

/**
 * @brief Initialize the PCF8553 auxiliary control GPIO lines.
 *
 * @details
 *  - This configures the control lines required by the current board wiring.
 *  - The initial implementation covers the CE and RESET lines defined by CubeMX.
 *  - Protocol-level LCD behavior remains outside this module.
 */
void FM_PORT_Pcf8553Ctrl_Init(void);

/**
 * @brief Assert the PCF8553 CE line.
 *
 * @details
 *  - This exposes board-local CE semantics rather than raw GPIO levels.
 *  - The active polarity remains localized to this module.
 */
void FM_PORT_Pcf8553Ctrl_Enable(void);

/**
 * @brief Deassert the PCF8553 CE line.
 *
 * @details
 *  - This exposes board-local CE semantics rather than raw GPIO levels.
 *  - The inactive polarity remains localized to this module.
 */
void FM_PORT_Pcf8553Ctrl_Disable(void);

/**
 * @brief Pulse the PCF8553 reset line using board-local reset polarity.
 *
 * @details
 *  - This applies the reset sequence expected by the current board/device path.
 *  - Reset line levels stay localized to this module.
 */
void FM_PORT_Pcf8553Ctrl_Reset(void);

#endif /* FM_PORT_PCF8553_CTRL_H */
