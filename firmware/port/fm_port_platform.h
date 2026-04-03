/*
 * @file    fm_port_platform.h
 * @brief   Minimal platform adapter over Cube/HAL for the hello world flow.
 */

#ifndef FM_PORT_PLATFORM_H_
#define FM_PORT_PLATFORM_H_

/* Includes */
#include <stdbool.h>
#include <stdint.h>

/* Public API */
void FM_PORT_PLATFORM_Init(void);
uint32_t FM_PORT_PLATFORM_GetTickMs(void);
void FM_PORT_PLATFORM_SetStatusLed(bool led_on);

#endif /* FM_PORT_PLATFORM_H_ */

/*** end of file ***/
