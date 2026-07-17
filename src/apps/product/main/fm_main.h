/**
 * @file    fm_main.h
 * @brief   Punto de entrada de la aplicación Flowmeet (capa APP).
 * @date    2025-09-14
 * @author  Flowmeet
 *
 * @details
 *   Este módulo es todavía un placeholder/smoke app del producto, no el runtime
 *   final. Inicializa servicios de board/debug/RTC y luego ejecuta un loop
 *   simple de LED/UART/sleep para validar la línea básica de aplicación.
 */

#ifndef FM_MAIN_H_
#define FM_MAIN_H_

/* =========================== Includes ==================================== */
#include "main.h"
#include <stdint.h>

/* =========================== Public API =================================== */
/**
 * @brief  Inicializa la aplicación placeholder de producto.
 *
 * Configura board, RTC y debug. Las fallas de inicialización de capas bajas
 * pueden terminar en `Error_Handler()`.
 *
 * @warning Foreground startup only.
 */
void FM_MAIN_Init(void);

/**
 * @brief  Ejecuta el loop placeholder de producto.
 *
 * Llama a `FM_MAIN_Init()`, alterna el LED de señal, emite un mensaje UART de
 * smoke test, duerme 500 ms y hace flush de debug en cada iteración.
 *
 * @warning Foreground app entry. Does not return during normal operation.
 */
void FM_MAIN_Main(void);

#endif /* FM_MAIN_H_ */

/*** end of file ***/
