/*
 * Autor: Daniel H Sagarra
 * 
 * Notas:
 * 
 */

#ifndef FM_PCF8553_H_
#define FM_PCF8553_H_

// Includes.

#include "main.h"

// Macros for microcontroller taken pins (dhs).

/*
 * Pines dedicados al PCF8553, no me confío de siempre usar el cubemx y de su
 * funcion de label name para los pines. De ser posible siempre declaro los
 * recursos usados del hardware lo mas arriba posible en los módulos, debe ser
 * coherente con el code style del proyecto.
 *
 */
#define PCF8553_CE_PORT		GPIOA
#define PCF8553_CE_PIN 		GPIO_PIN_4
#define PCF8553_RESET_PORT	GPIOA
#define PCF8553_RESET_PIN	GPIO_PIN_6

// Typedef.

/*
 * Enumeración de las velocidades disponibles para hacer parpadear la
 * pantalla en su totalidad.
 */
typedef enum
{
    OFF_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED
} blink_t;

// Defines.

/*
 * Tamaño de la memoria interna del pcf8553 para controlar el encendido/apagado
 * de los segmentos, con 20 bytes se controlan 20 * 8 =  160 segmentos.
 *
 */
#define PCF8553_RAM_SIZE   		20
#define PCF8553_SEGMENTS_ON		0xFF   // Macro para escribir 1 en todos los segmentos
#define PCF8553_SEGMENTS_OFF	0x00  // Macro para escribir 0 en todos los segmentos

// Extern

extern uint8_t pcf8553_ram_map[PCF8553_RAM_SIZE];

// Public function prototypes.

void
FM_PCF8553_Blink(blink_t mode);
void
FM_PCF8553_ClearBuffer();
void
FM_PCF8553_Init();
void
FM_PCF8553_Refresh();
void
FM_PCF8553_Reset();
void
FM_PCF8553_WriteAll(uint8_t data);
void
FM_PCF8553_WriteByte(uint8_t add, uint8_t data);

#endif /* FM_FM_PCF8553_H_ */

/*** end of file ***/
