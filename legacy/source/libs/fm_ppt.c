/*
 * @brief Portable Printer Thermal.
 *
 * Version: 1
 * Autor: Daniel H Sagarra.
 * Fecha: 2/3/2025
 * - Version inicial.
 *
 *
 */

// Includes.
#include "fm_fmc.h"
#include "fm_mxc.h"
#include "string.h"
#include "stdio.h"
#include "main.h"
#include "fm_rtc.h"
#include "fm_usart.h"

// Defines.
#define RIGHT_LEN 18 // Cantidad de columnas de la impresora
#define LEFT_LEN 12 // Cantidad de columnas de la impresora
#define PRINTER_LEN 32      // Cantidad de columnas de la impresora
#define MAX_FIELD_LEN (PRINTER_LEN + 2)  // Columnas de la impresora + \n\0

// Typedef.
typedef struct
{
    char number[RIGHT_LEN];
    char ttl[RIGHT_LEN];
    char date[RIGHT_LEN];  // formato esperado: "DD/MM/AAAA"
    char time[RIGHT_LEN];  // formato esperado: "HH:MM" (o "HH:MM:SS")
    char acm[RIGHT_LEN];
} ticket_data_t;

// Const data.

// Debug.

// Project variables, non-static, at least used in other file.

// External variables.

// Global variables, statics.
ticket_data_t ticket;
char page_buffer[FM_USART_TX3_BUF_SIZE];

// Private function prototypes.

// Private function bodies.

// Public function bodies.

void FM_PPT_FormatTicket()
{
    uint8_t sel;

    sel = FM_FMC_TotalizerFpSelGet();

    snprintf(ticket.number, sizeof(ticket.number), "%u", FM_FMC_TicketNumberGet());
    FM_FMC_Ufp3ToString(ticket.ttl, sizeof(ticket.ttl), FM_FMC_TtlGet(), sel);
    FM_RTC_GetPpt(ticket.time, ticket.date);
    FM_FMC_Ufp3ToString(ticket.acm, sizeof(ticket.acm), FM_FMC_AcmGet(), sel);
}

void FM_PPT_PrintTicket()
{
    //Línea separadora superior
    snprintf(page_buffer, MAX_FIELD_LEN, "  ============================\n");

    // Número de control (etiqueta izquierda, valor derecha)
    snprintf(page_buffer + strlen(page_buffer), MAX_FIELD_LEN, "%-10s%s\n", "Ticket Nro: ",
            ticket.number);

    // TTL
    snprintf(page_buffer + strlen(page_buffer), MAX_FIELD_LEN, "%-10s%s\n", "TTL:", ticket.ttl);

    // Fecha
    snprintf(page_buffer + strlen(page_buffer), MAX_FIELD_LEN, "%-10s%s\n", "Fecha:", ticket.date);

    // Hora
    snprintf(page_buffer + strlen(page_buffer), MAX_FIELD_LEN, "%-10s%s\n", "Hora:", ticket.time);

    // ACM
    snprintf(page_buffer + strlen(page_buffer), MAX_FIELD_LEN, "%-10s%s\n", "ACM:", ticket.acm);

    snprintf(page_buffer + strlen(page_buffer), MAX_FIELD_LEN, "\n\n");

    // Operario (dejar en blanco para llenar a mano)
    snprintf(page_buffer + strlen(page_buffer), MAX_FIELD_LEN, "Operario:\n\n\n\n");

    // Recibió (dejar en blanco para llenar a mano)
    snprintf(page_buffer + strlen(page_buffer), MAX_FIELD_LEN, "Recibio:\n\n\n\n");

    // (Opcional) Línea separadora inferior
    snprintf(page_buffer + strlen(page_buffer), MAX_FIELD_LEN, "  ============================\n");

    snprintf(page_buffer + strlen(page_buffer), MAX_FIELD_LEN, "\n\n\n");

    FM_USART_Uart3TransmitDma(page_buffer, 0);
    HAL_Delay(1000);
}

// Interrupts

/*** end of file ***/

