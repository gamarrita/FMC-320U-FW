/**
 * @file    fm_port_spi1.h
 * @brief   Platform SPI1 helpers for device transport.
 *
 * @details
 *  - This module owns the MCU SPI1 instance used by board devices.
 *  - CubeMX-generated SPI1 code is treated as a configuration reference.
 *  - This module applies the technical SPI1 configuration explicitly so port
 *    remains the active owner of the peripheral instance.
 *  - Device-level meaning stays outside this module.
 */

#ifndef FM_PORT_SPI1_H
#define FM_PORT_SPI1_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Configure SPI1 for board device transport.
 *
 * @details
 *  - This is a technical backend API for the MCU SPI1 instance.
 *  - The configuration matches the current CubeMX reference for SPI1.
 *  - Board-level meaning and device transaction policy stay outside this module.
 */
void FM_PORT_Spi1_Init(void);

/**
 * @brief Write a raw byte sequence over SPI1.
 *
 * @param p_data      Buffer to transmit.
 * @param len         Number of bytes to send.
 * @param timeout_ms  HAL timeout in milliseconds.
 *
 * @return true on successful transmit, false on invalid input or HAL failure.
 */
bool FM_PORT_Spi1_Write(const uint8_t *p_data, uint16_t len, uint32_t timeout_ms);

#endif /* FM_PORT_SPI1_H */
