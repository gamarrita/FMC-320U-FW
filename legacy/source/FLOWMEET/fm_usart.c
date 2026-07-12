/*
 * Descripcion
 *
 * Autor:
 * Fecha: 
 */

// Includes.
#include "fm_debug.h"
#include "fm_usart.h"
#include "stdio.h"

// Sección #define

// Sección typedef

// Sección enum

// Const data.

// Debug.

// Variables non-static
char fm_usart_rx3_buf[FM_USART_RX3_BUF_SIZE] =
{ 0 };

char fm_usart_tx3_buf[FM_USART_TX3_BUF_SIZE] =
{ 0 };

// Variables statics.
TX_SEMAPHORE uart3_rx_sem;

// Variables extern, las que no estan en .h.

// Prototipos funciones privadas.

// Cuerpo funciones privadas.

// Public function bodies.

void FM_USART_RtosInit(VOID *memory_ptr)
{
    fmx_status_t fmx_status = FMX_STATUS_NULL;

    // >>>> ThreadX. Semáforo bloque luego de enviar de datos, desbloquea al recibir.
    fmx_status = tx_semaphore_create(&uart3_rx_sem, "UART3_RX_SEMAPHORE", 0);
    if (fmx_status != TX_SUCCESS)
    {
        FM_DEBUG_LedError(1);
        while (1);
    }
    // <<<< Fin.
}

/*
 * @brief   Enciende al UART 3.
 * @param   Ninguno.
 * @retval  Ninguno.
 */
fmx_status_t FM_USART_Uart3PowerOn()
{
    fmx_status_t fmx_status = FMX_STATUS_NULL;
    HAL_StatusTypeDef hal_status;

    // Limpio Enciendo el UART 3.
    HAL_UART_MspInit(&huart3);

    // Con el modulo EMC3080 estable puedo empezar a recibir datos por UART 3.
    hal_status = HAL_UARTEx_ReceiveToIdle_DMA(&huart3, (uint8_t*) fm_usart_rx3_buf,
            FM_USART_RX3_BUF_SIZE);

    if (hal_status != HAL_OK)
    {
        fmx_status = FMX_STATUS_ERROR;
    }
    else
    {
        fmx_status = FMX_STATUS_OK;
    }

    return fmx_status;
}

fmx_status_t FM_USART_Uart3TransmitDma(const char *str, UINT wait_ms)
{
    fmx_status_t fmx_status = FMX_STATUS_NULL;
    HAL_StatusTypeDef hal_status;
    UINT tx_status;

    int written;

    written = sniprintf(fm_usart_tx3_buf, FM_USART_TX3_BUF_SIZE, "%s", str);

    memset(fm_usart_rx3_buf, 0, sizeof(FM_USART_RX3_BUF_SIZE));

    if (written > 0)
    {
        hal_status = HAL_UART_Transmit_DMA(&huart3, (const uint8_t*) fm_usart_tx3_buf, written);
    }
    else
    {
        FM_DEBUG_LedError(1);
        return FMX_STATUS_ERROR;
    }

    if (hal_status == HAL_OK)
    {
        fmx_status = FMX_STATUS_OK;
    }
    else
    {
        FM_DEBUG_LedError(1);
        return FMX_STATUS_ERROR;
    }

    if (wait_ms)
    {
        tx_status = tx_semaphore_get(&uart3_rx_sem, wait_ms / 10);
        if (tx_status != TX_SUCCESS)
        {
            FM_DEBUG_LedError(1);
            return FMX_STATUS_ERROR;
        }
    }

    return fmx_status;
}

// Interrupts

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART3)
    {
        tx_semaphore_put(&uart3_rx_sem);
        HAL_UARTEx_ReceiveToIdle_DMA(huart, (uint8_t*) fm_usart_rx3_buf, FM_USART_RX3_BUF_SIZE);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {

    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        FM_DEBUG_LedError(1);
        HAL_UART_AbortReceive(huart);
        HAL_UARTEx_ReceiveToIdle_DMA(huart, (uint8_t*) fm_usart_rx3_buf, FM_USART_RX3_BUF_SIZE);
    }
}

/*** end of file ***/

